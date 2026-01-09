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
            static std::map<char, Character>* getFont(const std::string& nameFont);
        private:
            //name font-> char-> gl
            static std::unordered_map<std::string, std::map<char, Character>> m_fonts;
    };

    struct Transform2D
    {
        Vector2 anchor = {0.f, 0.f}; // normalized 0..1 
        Vector2 scale = {1.f, 1.f};   // normalized 0..1 
        Vector2 pos = {0.f, 0.f};
        //Vector2 size = {0.f, 0.f};   // not normalizated
        bool dirty = true;
        
        public:
        Transform2D(Vector2 anchor, Vector2 scale, Vector2 pos):
        anchor(anchor), scale(scale), pos(pos){}

        void setDirty(bool flag)
        {
            dirty = flag;
        }
    };

    class TextMaterial;
    class TextMeshRenderer;

    class Text
    {
        public:
            Text() = delete;
            Text(TextMaterial* pTextMaterial);
            //posAncor is in the range[0,1], pos is in px
            Text(Vector2 posAncor, Vector2 pos, TextMaterial* pTextMaterial);
            Text(Vector2 posAncor, Vector2 pos, Vector2 scale, TextMaterial* pTextMaterial);
            
            ~Text();
        
            inline std::string getString(){return m_string;}
            
            inline void setString(const std::string& str){m_string = str;}
            inline void appendString(const std::string& str){m_string.append(str);}
        public:
            TextMeshRenderer* pTextMeshRend = nullptr;
            Transform2D* pTransf = nullptr;

            private:
                std::string m_string = "";
    };

}
