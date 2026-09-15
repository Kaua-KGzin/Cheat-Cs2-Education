#pragma once
#include "memory.h"
#include <optional>

inline std::optional<std::pair<int,int>> world_to_screen(const float vm[16], const Vec3& pos, int sw, int sh) {
    float sx = vm[0]*pos.x + vm[1]*pos.y + vm[2]*pos.z + vm[3];
    float sy = vm[4]*pos.x + vm[5]*pos.y + vm[6]*pos.z + vm[7];
    float sz = vm[8]*pos.x + vm[9]*pos.y + vm[10]*pos.z + vm[11];
    float sw_val = vm[12]*pos.x + vm[13]*pos.y + vm[14]*pos.z + vm[15];

    if (sw_val < 0.001f) return std::nullopt;

    float inv_w = 1.0f / sw_val;
    float nx = sx * inv_w;
    float ny = sy * inv_w;

    int screen_x = (int)(sw / 2.0f + nx * sw / 2.0f);
    int screen_y = (int)(sh / 2.0f - ny * sh / 2.0f);
    return std::make_pair(screen_x, screen_y);
}
