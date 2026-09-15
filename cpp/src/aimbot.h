#pragma once
#include <windows.h>
#include <cmath>
#include "memory.h"
#include "w2s.h"

class Aimbot {
public:
    float sensitivity = 2.0f;
    int fov_limit = 120;
    float smooth = 0.15f;
    int deadzone = 5;
    int stick_radius = 160;
    bool locked = false;
    Vec3 locked_pos{};

    struct Target {
        Vec3 head_pos;
        float screen_dist;
    };

    void aim(const float vm[16], const std::vector<Target>& targets, int sw, int sh) {
        int cx = sw / 2;
        int cy = sh / 2;

        if (targets.empty()) {
            locked = false;
            return;
        }

        Target best{};
        best.screen_dist = static_cast<float>(stick_radius) + 1.0f;

        if (locked) {
            float min_dist = static_cast<float>(stick_radius) + 1.0f;
            for (const auto& t : targets) {
                auto screen = world_to_screen(vm, t.head_pos, sw, sh);
                if (!screen) continue;
                auto [sx, sy] = *screen;
                float dx = static_cast<float>(sx) - static_cast<float>(cx);
                float dy = static_cast<float>(sy) - static_cast<float>(cy);
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < min_dist) {
                    min_dist = dist;
                    best = t;
                }
            }
            if (min_dist > static_cast<float>(stick_radius)) {
                locked = false;
                locked_pos = Vec3{};
                return;
            }
            best.screen_dist = min_dist;
        } else {
            for (const auto& t : targets) {
                auto screen = world_to_screen(vm, t.head_pos, sw, sh);
                if (!screen) continue;
                auto [sx, sy] = *screen;
                float dx = static_cast<float>(sx) - static_cast<float>(cx);
                float dy = static_cast<float>(sy) - static_cast<float>(cy);
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < best.screen_dist && dist <= static_cast<float>(fov_limit)) {
                    best = t;
                    best.screen_dist = dist;
                }
            }
        }

        if (!is_valid_target(best)) return;

        auto screen = world_to_screen(vm, best.head_pos, sw, sh);
        if (!screen) return;
        auto [sx, sy] = *screen;

        float dx = static_cast<float>(sx) - static_cast<float>(cx);
        float dy = static_cast<float>(sy) - static_cast<float>(cy);
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < static_cast<float>(deadzone)) {
            locked = true;
            locked_pos = best.head_pos;
            return;
        }

        locked = true;
        locked_pos = best.head_pos;

        float move_x = dx * sensitivity * smooth;
        float move_y = dy * sensitivity * smooth;

        if (std::abs(move_x) < 0.5f) move_x = 0;
        if (std::abs(move_y) < 0.5f) move_y = 0;

        INPUT input{};
        input.type = INPUT_MOUSE;
        input.mi.dx = (LONG)move_x;
        input.mi.dy = (LONG)move_y;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        SendInput(1, &input, sizeof(INPUT));
    }

private:
    bool is_valid_target(const Target& t) const {
        return locked || t.screen_dist <= static_cast<float>(stick_radius) + 1.0f;
    }
};