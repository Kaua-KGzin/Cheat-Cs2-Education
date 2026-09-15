#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <cmath>
#include <vector>
#include <windows.h>

#include "memory.h"
#include "offsets.h"
#include "overlay.h"
#include "trigger.h"
#include "aimbot.h"
#include "w2s.h"
#include "gui.h"

struct Player {
    Vec3 pos;
    uint8_t team;
    int hp;
    bool is_enemy;
    bool crouching;
    float distance;
    int entity_idx;
};

float calc_distance(Vec3 a, Vec3 b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

float get_bone_z_offset(int bone_id, bool crouching) {
    switch (bone_id) {
        case 0: return crouching ? 48.0f : 64.0f;
        case 1: return crouching ? 32.0f : 46.0f;
        case 2: return crouching ? 22.0f : 34.0f;
        case 3: return crouching ? 16.0f : 28.0f;
    }
    return crouching ? 48.0f : 64.0f;
}

float get_bone_esp_offset(int bone_id, bool crouching) {
    switch (bone_id) {
        case 0: return crouching ? 52.0f : 72.0f;
        case 1: return crouching ? 36.0f : 50.0f;
        case 2: return crouching ? 26.0f : 38.0f;
        case 3: return crouching ? 20.0f : 32.0f;
    }
    return crouching ? 52.0f : 72.0f;
}

class EntityScanner {
public:
    EntityScanner(Memory& mem, Offsets& off) : mem_(mem), off_(off) {}

    bool scan_local_player(uintptr_t& pawn, uint8_t& team, Vec3& pos) {
        pawn = mem_.read_ptr(mem_.client_base + off_.dwLocalPlayerPawn);
        if (!pawn || pawn <= 0x10000) return false;

        team = 0;
        pos = Vec3{};
        if (pawn && pawn > 0x10000) {
            team = mem_.read<uint8_t>(pawn + off_.m_iTeamNum);
            pos = mem_.read_vec3(pawn + off_.m_vOldOrigin);
        }
        return true;
    }

    bool scan_entities(
        uintptr_t entity_list,
        uintptr_t local_pawn,
        uint8_t local_team,
        std::vector<Player>& players,
        int max_dist
    ) {
        players.clear();

        if (!entity_list || local_pawn <= 0x10000 || local_team == 0) return false;

        uintptr_t chunk0 = mem_.read_ptr(entity_list + 0x10);
        if (!chunk0) return false;

        constexpr int SCAN_COUNT = 64;
        constexpr size_t CHUNK_SIZE = SCAN_COUNT * 0x70;
        auto chunk_data = mem_.read_bytes(chunk0, CHUNK_SIZE);

        if (chunk_data.size() < CHUNK_SIZE) return false;

        for (int i = 0; i < SCAN_COUNT; i++) {
            uintptr_t entity_ptr = *reinterpret_cast<uintptr_t*>(chunk_data.data() + i * 0x70);
            if (!entity_ptr || entity_ptr <= 0x10000) continue;

            uint32_t pawn_handle = mem_.read<uint32_t>(entity_ptr + off_.m_hPlayerPawn);
            if (!pawn_handle || pawn_handle == 0xFFFFFFFF) continue;

            int pawn_idx = pawn_handle & 0x3FFF;
            if (pawn_idx == 0) continue;

            uintptr_t pawn_addr = mem_.get_entity(entity_list, pawn_idx);
            if (!pawn_addr || pawn_addr < 0x10000) continue;

            uint8_t team = mem_.read<uint8_t>(pawn_addr + off_.m_iTeamNum);
            if (team == 0 || (team != local_team && team != 2 && team != 3)) continue;

            int hp = mem_.read<int>(pawn_addr + off_.m_iHealth);
            if (hp <= 0 || hp > 200) continue;

            Vec3 pos = mem_.read_vec3(pawn_addr + off_.m_vOldOrigin);
            if (std::abs(pos.x) > 50000 || std::abs(pos.y) > 50000 || std::abs(pos.z) > 50000) continue;

            uint32_t flags = mem_.read<uint32_t>(pawn_addr + off_.m_fFlags);
            bool crouching = (flags & 0x2) != 0;

            float dist = calc_distance(local_pos_, pos);
            if (max_dist > 0 && dist > static_cast<float>(max_dist)) continue;

            players.push_back({pos, team, hp, team != local_team, crouching, dist, i});
        }
        return true;
    }

    void set_local_pos(Vec3 pos) { local_pos_ = pos; }
    Vec3 get_local_pos() const { return local_pos_; }

private:
    Memory& mem_;
    Offsets& off_;
    Vec3 local_pos_;
};

int main() {
    Offsets off;
    std::string dumper_dir = "cs2-dumper-main\\output\\";
    bool offsets_loaded = false;

    offsets_loaded = load_offsets_from_file(dumper_dir + "offsets.json", off);
    if (!offsets_loaded) {
        offsets_loaded = load_offsets_from_file(dumper_dir + "client_dll.json", off);
    }
    if (!offsets_loaded) return 1;

    Memory mem;
    if (!mem.attach("cs2.exe")) return 1;
    if (!mem.find_module("client.dll")) return 1;

    EntityScanner scanner(mem, off);

    Overlay overlay;
    if (!overlay.create()) return 1;

    if (!gui::create()) return 1;

    Trigger trigger;
    Aimbot aimbot;
    bool running = true;

    auto debounce = [](int vk) -> bool {
        static bool states[256] = {};
        bool pressed = GetAsyncKeyState(vk) & 0x8000;
        bool was = states[vk & 0xFF];
        states[vk & 0xFF] = pressed;
        return pressed && !was;
    };

    int fps_counter = 0;
    int fps_display = 0;
    auto fps_timer = std::chrono::steady_clock::now();

    uintptr_t entity_list = 0;
    int entity_list_failures = 0;
    int max_dist = 0;
    int bone_id = 0;

    while (running) {
        auto t0 = std::chrono::steady_clock::now();

        gui::pump();
        if (!gui::running) break;

        if (debounce(VK_F1)) gui::settings.trigger_on = !gui::settings.trigger_on;
        if (debounce(VK_F2)) gui::settings.esp_on = !gui::settings.esp_on;
        if (debounce(VK_F3)) gui::settings.aimbot_on = !gui::settings.aimbot_on;
        if (debounce(VK_F4)) gui::settings.anti_recoil_on = !gui::settings.anti_recoil_on;
        if (debounce(VK_F5)) gui::settings.headshot_only = !gui::settings.headshot_only;
        if (debounce(VK_F6)) gui::settings.radar_hack = !gui::settings.radar_hack;
        if (debounce(VK_F7)) gui::save_config("config.cfg");
        if (debounce(VK_F8)) gui::load_config("config.cfg");
        if (debounce(VK_F9)) { gui::running = false; break; }

        bool active_trigger = gui::settings.trigger_on;
        bool active_esp = gui::settings.esp_on;
        bool active_aimbot = gui::settings.aimbot_on;
        bool active_anti_recoil = gui::settings.anti_recoil_on;
        bool active_headshot = gui::settings.headshot_only;
        bool active_radar = gui::settings.radar_hack;
        max_dist = gui::settings.max_distance;
        bone_id = gui::settings.bone_selection;
        int need_entities = active_esp || active_trigger || active_aimbot;

        aimbot.sensitivity = gui::settings.aim_sensitivity;
        aimbot.smooth = gui::settings.aim_smooth;
        aimbot.fov_limit = gui::settings.aim_fov;
        aimbot.deadzone = gui::settings.aim_deadzone;
        aimbot.stick_radius = gui::settings.aim_stick;

        uintptr_t local_pawn = 0;
        uint8_t local_team = 0;
        Vec3 local_pos{};
        scanner.set_local_pos(local_pos);

        if (need_entities || active_anti_recoil) {
            if (scanner.scan_local_player(local_pawn, local_team, local_pos)) {
                scanner.set_local_pos(local_pos);
            }
        }

        if (!entity_list || entity_list_failures > 10) {
            entity_list = mem.read_ptr(mem.client_base + off.dwEntityList);
            entity_list_failures = 0;
        }

        float vm[16]{};
        bool vm_ok = false;
        if (active_esp && off.dwViewMatrix) {
            vm_ok = mem.read_view_matrix(mem.client_base + off.dwViewMatrix, vm);
        }

        std::vector<Player> players;
        scanner.set_local_pos(local_pos);

        if (need_entities && entity_list && local_pawn && local_team) {
            scanner.scan_entities(entity_list, local_pawn, local_team, players, max_dist);
        }

        gui::settings.players = static_cast<int>(players.size());

        bool hit = false;
        if (active_trigger && local_pawn && local_team && entity_list) {
            int crosshair_idx = mem.read<int>(local_pawn + off.m_iIDEntIndex);
            if (crosshair_idx >= 1 && crosshair_idx <= 0x1000) {
                uintptr_t target = mem.get_entity(entity_list, crosshair_idx);
                if (target && target > 0x10000) {
                    uint8_t target_team = mem.read<uint8_t>(target + off.m_iTeamNum);
                    int target_hp = mem.read<int>(target + off.m_iHealth);
                    if (target_team && target_team != local_team && target_hp > 0 && target_hp <= 200) {
                        if (active_headshot) {
                            uint32_t th = mem.read<uint32_t>(target + off.m_hPlayerPawn);
                            int tpi = th & 0x3FFF;
                            uintptr_t tp = mem.get_entity(entity_list, tpi);
                            if (tp && tp > 0x10000) {
                                uint32_t tf = mem.read<uint32_t>(tp + off.m_fFlags);
                                bool tc = (tf & 0x2) != 0;
                                Vec3 head = mem.read_vec3(tp + off.m_vOldOrigin);
                                head.z += get_bone_z_offset(0, tc);
                                float hvm[16]{};
                                if (off.dwViewMatrix) mem.read_view_matrix(mem.client_base + off.dwViewMatrix, hvm);
                                auto hs = world_to_screen(hvm, head, overlay.screen_w, overlay.screen_h);
                                if (hs) {
                                    auto [hx, hy] = *hs;
                                    float dx = static_cast<float>(hx) - static_cast<float>(overlay.screen_w) / 2.0f;
                                    float dy = static_cast<float>(hy) - static_cast<float>(overlay.screen_h) / 2.0f;
                                    if (std::sqrt(dx * dx + dy * dy) < 40.0f) hit = true;
                                }
                            }
                        } else {
                            hit = true;
                        }
                    }
                }
            }
        }

        if (hit) {
            trigger.click();
            gui::settings.shots++;
            std::this_thread::sleep_for(std::chrono::milliseconds(gui::settings.trigger_delay));
        }

        if (active_anti_recoil) {
            bool shooting = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
            if (shooting) {
                float recoil = gui::settings.anti_recoil_strength * 0.06f;
                INPUT input{};
                input.type = INPUT_MOUSE;
                input.mi.dy = static_cast<LONG>(recoil);
                input.mi.dwFlags = MOUSEEVENTF_MOVE;
                SendInput(1, &input, sizeof(INPUT));
            }
        }

        if (active_aimbot && local_team) {
            int sw = overlay.screen_w;
            int sh = overlay.screen_h;
            float aim_vm[16]{};
            if (off.dwViewMatrix) {
                mem.read_view_matrix(mem.client_base + off.dwViewMatrix, aim_vm);
            }
            std::vector<Aimbot::Target> targets;
            for (auto& p : players) {
                if (p.is_enemy && p.hp > 0) {
                    Vec3 target_pos = p.pos;
                    target_pos.z += get_bone_z_offset(bone_id, p.crouching);
                    targets.push_back({target_pos, 0});
                }
            }
            aimbot.aim(aim_vm, targets, sw, sh);
        }

        if (active_esp) {
            overlay.begin();

            if (vm_ok) {
                int sw = overlay.screen_w;
                int sh = overlay.screen_h;

                for (auto& p : players) {
                    auto foot = world_to_screen(vm, p.pos, sw, sh);
                    Vec3 head_pos = p.pos;
                    head_pos.z += get_bone_esp_offset(bone_id, p.crouching);
                    auto head = world_to_screen(vm, head_pos, sw, sh);

                    if (!foot || !head) continue;
                    auto [fx, fy] = *foot;
                    auto [hx, hy] = *head;

                    if (fx < -200 || fx > sw + 200 || fy < -200 || fy > sh + 200) continue;

                    COLORREF color = p.is_enemy ? RGB(0, 180, 255) : (p.team == 2 ? RGB(255, 80, 80) : RGB(100, 180, 255));
                    int box_h = std::abs(fy - hy);
                    if (box_h < 4) box_h = 20;
                    int box_w = box_h / 2;
                    int cx = fx;
                    int cy = hy;

                    overlay.draw_corner_box(cx - box_w / 2, cy, box_w, box_h, color, 2);
                    overlay.draw_health_bar(cx - box_w / 2, cy, box_h, p.hp);

                    char dist_buf[16];
                    sprintf(dist_buf, "%.0fm", p.distance / 100.0f);
                    std::string dist_text(dist_buf);

                    std::string hp_text = std::to_string(p.hp);
                    overlay.draw_text(cx - box_w / 2 - 28, cy + box_h / 2 - 6, hp_text, color, 11);
                    overlay.draw_text(cx + box_w / 2 + 4, cy + box_h / 2 - 6, dist_text, RGB(200, 200, 200), 10);
                    overlay.draw_snapline(cx, sh, cx, cy + box_h, RGB(255, 255, 255));
                }

                overlay.draw_crosshair(sw / 2, sh / 2, 14, RGB(255, 255, 0));
            }

            overlay.end();
        }

        fps_counter++;
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration<double>(now - fps_timer).count() >= 1.0) {
            fps_display = fps_counter;
            gui::settings.fps = fps_counter;
            fps_counter = 0;
            fps_timer = now;
        }

        auto elapsed = std::chrono::steady_clock::now() - t0;
        auto target = std::chrono::microseconds(8333);
        if (elapsed < target) {
            std::this_thread::sleep_for(target - elapsed);
        }
    }

    return 0;
}
