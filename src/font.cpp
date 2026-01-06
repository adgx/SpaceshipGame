#include "font.h"
#include "log.h"

namespace SpaceEngine
{

    void FontLoader::LoadFont(const std::string &nameFont)
    {
        std::map<char, Character> mapChars;
        mapChars = TextureManager::loadFontChars(nameFont);

        if (!mapChars.empty())
            m_fonts[Utils::getFileNameNoExt(nameFont)] = mapChars;
    }
}