#pragma once
#include <cstdint>
#include <string>
#include <fstream>
#include <sstream>

struct Offsets {
    // Global offsets (from offsets.json)
    uint64_t dwEntityList = 0;
    uint64_t dwLocalPlayerPawn = 0;
    uint64_t dwLocalPlayerController = 0;
    uint64_t dwViewMatrix = 0;
    uint64_t dwGameEntitySystem_highestEntityIndex = 0;

    // Class offsets (from client_dll.json) - C_BaseEntity
    uint32_t m_pGameSceneNode = 0x330;
    uint32_t m_iHealth = 0x34C;
    uint32_t m_iTeamNum = 0x3E7;
    uint32_t m_fFlags = 0x3F4;

    // CGameSceneNode
    uint32_t m_vecAbsOrigin = 0xC8;

    // C_BasePlayerPawn
    uint32_t m_vOldOrigin = 0x13B8;

    // CCSPlayerController
    uint32_t m_hPlayerPawn = 0x914;

    // C_CSPlayerPawnBase
    uint32_t m_iIDEntIndex = 0x342C;

    // Bone indices for aimbot/ESP
    enum class Bone {
        Head = 0,
        Chest = 1,
        Stomach = 2,
        Pelvis = 3
    };
};

inline bool load_offsets_from_file(const std::string& path, Offsets& off) {
    std::ifstream f(path);
    if (!f.is_open()) return false;

    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();

    auto find_val = [&](const std::string& key) -> int64_t {
        std::string search = "\"" + key + "\"";
        size_t pos = content.find(search);
        if (pos == std::string::npos) return -1;
        pos = content.find(':', pos + search.size());
        if (pos == std::string::npos) return -1;
        pos++;
        while (pos < content.size() && content[pos] == ' ') pos++;
        size_t end = pos;
        while (end < content.size() && (content[end] >= '0' && content[end] <= '9')) end++;
        if (end == pos) return -1;
        try { return std::stoll(content.substr(pos, end - pos)); } catch (...) { return -1; }
    };

    auto find_val_from = [&](const std::string& key, size_t start_pos) -> int64_t {
        std::string search = "\"" + key + "\"";
        size_t pos = content.find(search, start_pos);
        if (pos == std::string::npos) return -1;
        pos = content.find(':', pos + search.size());
        if (pos == std::string::npos) return -1;
        pos++;
        while (pos < content.size() && content[pos] == ' ') pos++;
        size_t end = pos;
        while (end < content.size() && (content[end] >= '0' && content[end] <= '9')) end++;
        if (end == pos) return -1;
        try { return std::stoll(content.substr(pos, end - pos)); } catch (...) { return -1; }
    };

    auto find_in_class = [&](const std::string& class_name, const std::string& field) -> int64_t {
        std::string cls_search = "\"" + class_name + "\"";
        size_t cls_pos = content.find(cls_search);
        if (cls_pos == std::string::npos) return -1;
        std::string fields_search = "\"fields\"";
        size_t fields_pos = content.find(fields_search, cls_pos);
        if (fields_pos == std::string::npos) return -1;
        return find_val_from(field, fields_pos);
    };

    int64_t v;

    // Try global offsets
    v = find_val("dwEntityList");          if (v >= 0) off.dwEntityList = static_cast<uint64_t>(v);
    v = find_val("dwLocalPlayerPawn");     if (v >= 0) off.dwLocalPlayerPawn = static_cast<uint64_t>(v);
    v = find_val("dwLocalPlayerController"); if (v >= 0) off.dwLocalPlayerController = static_cast<uint64_t>(v);
    v = find_val("dwViewMatrix");          if (v >= 0) off.dwViewMatrix = static_cast<uint64_t>(v);
    v = find_val("dwGameEntitySystem_highestEntityIndex"); if (v >= 0) off.dwGameEntitySystem_highestEntityIndex = static_cast<uint64_t>(v);

    // Class offsets - try JSON first, fallback to hardcoded
    {
        auto v = find_in_class("C_BaseEntity", "m_pGameSceneNode");
        if (v >= 0) off.m_pGameSceneNode = static_cast<uint32_t>(v); else off.m_pGameSceneNode = 0x330;
    }
    {
        auto v = find_in_class("C_BaseEntity", "m_iHealth");
        if (v >= 0) off.m_iHealth = static_cast<uint32_t>(v); else off.m_iHealth = 0x34C;
    }
    {
        auto v = find_in_class("C_BaseEntity", "m_iTeamNum");
        if (v >= 0) off.m_iTeamNum = static_cast<uint32_t>(v); else off.m_iTeamNum = 0x3E7;
    }
    {
        auto v = find_in_class("C_BaseEntity", "m_fFlags");
        if (v >= 0) off.m_fFlags = static_cast<uint32_t>(v); else off.m_fFlags = 0x3F4;
    }
    {
        auto v = find_in_class("CGameSceneNode", "m_vecAbsOrigin");
        if (v >= 0) off.m_vecAbsOrigin = static_cast<uint32_t>(v); else off.m_vecAbsOrigin = 0xC8;
    }
    {
        auto v = find_in_class("C_BasePlayerPawn", "m_vOldOrigin");
        if (v >= 0) off.m_vOldOrigin = static_cast<uint32_t>(v); else off.m_vOldOrigin = 0x13B8;
    }
    {
        auto v = find_in_class("CCSPlayerController", "m_hPlayerPawn");
        if (v >= 0) off.m_hPlayerPawn = static_cast<uint32_t>(v); else off.m_hPlayerPawn = 0x914;
    }
    {
        auto v = find_in_class("C_CSPlayerPawnBase", "m_iIDEntIndex");
        if (v >= 0) off.m_iIDEntIndex = static_cast<uint32_t>(v); else off.m_iIDEntIndex = 0x342C;
    }

    return off.dwEntityList != 0 && off.dwLocalPlayerPawn != 0;
}