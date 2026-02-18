#pragma once

#include "utils/utils.h"

namespace SpaceEngine
{
    enum class ELightType 
    {
        POINT_LIGHT_NODEC,
        POINT_LIGHT,
        DIRECTIONAL_LIGHT
    };

    struct Light
    {
        Vector3 pos = {0.f, 0.f, 0.f};
        Vector3 color = {255.f, 255.f, 255.f};
        Vector3 dir = {0.f, 0.f, 0.f};
        int type;

        Light(Vector3 pos, Vector3 color, bool dec = false) : 
        pos(pos), color(color)
        {
            type = (dec) ? static_cast<int>(ELightType::POINT_LIGHT) : static_cast<int>(ELightType::POINT_LIGHT_NODEC);
        }

        Light(Vector3 pos, Vector3 color, Vector3 dir) : 
        Light(pos, color)
        {
            this->dir = dir;
            type = static_cast<int>(ELightType::DIRECTIONAL_LIGHT);
        }
        ~Light() = default;
    };
}