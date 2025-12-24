#pragma once

#include "utils/utils.h"

namespace SpaceEngine
{
    struct Light
    {
        Vector3 pos = {0.f, 0.f, 0.f};
        Vector3 color = {1.f, 1.f, 1.f};

        Light(Vector3 pos, Vector3 color)
        {
            this->pos = pos;
            this->color = color;
        }

        ~Light() = default;
    };
}