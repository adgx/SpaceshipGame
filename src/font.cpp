#include "font.h"
#include "log.h"
#include "material.h"
#include "mesh.h"

namespace SpaceEngine
{
    std::unordered_map<std::string, std::map<char, Character>> FontLoader::m_fonts;

    void FontLoader::LoadFont(const std::string &nameFont)
    {
        std::map<char, Character> mapChars;
        mapChars = TextureManager::loadFontChars(nameFont);

        if (!mapChars.empty()){
            m_fonts[Utils::getFileNameNoExt(nameFont)] = mapChars;
            SPACE_ENGINE_INFO("Font: {} loaded", nameFont);
        }
    }

    std::map<char, Character>* FontLoader::getFont(const std::string& nameFont)
    {
        if(auto it = m_fonts.find(nameFont); it != m_fonts.end())
        {
            return &m_fonts[nameFont];
        }
        
        return nullptr;

    }

    Text::Text(TextMaterial* pTextMaterial)
    {
        pTextMeshRend = new TextMeshRenderer();
        pTextMeshRend->bindMaterial(pTextMaterial);

        if(!pTextMaterial->pShader)
        {
            pTextMaterial->pShader = ShaderManager::findShaderProgram("glyphs"); 
        }
    }

    Text::Text(Vector2 posAncor, Vector2 pos, TextMaterial* pTextMaterial):
    Text(pTextMaterial)
    {
        pTransf = new Transform2D(posAncor, {1.f, 1.f}, pos);
    }

    Text::Text(Vector2 posAncor, Vector2 pos, Vector2 scale, TextMaterial* pTextMaterial):
    Text(posAncor, pos, pTextMaterial)
    {
        pTransf->scale = scale;
    }

    Text::~Text()
    {
        delete pTextMeshRend;
        delete pTransf;
    }

}