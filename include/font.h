#pragma once

#include <string>
#include <unordered_map>
#include <map>
#include "texture.h"
#include "utils/utils.h"
                             

namespace SpaceEngine
{
    struct Character
    {
        Texture* pTex = nullptr;
        Vector2i size = {0, 0};
        Vector2i bearing = {0, 0};
        unsigned int advance = 0;
    };

    class FontLoader
    {
        public:
            static void LoadFont(const std::string& nameFont);
        private:
            //name font-> char-> gl
            static std::unordered_map<std::string, std::map<char, Character>> m_fonts;
    };
}
