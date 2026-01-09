#pragma once
#include "utils/utils.h"
#include "shader.h"
#include "texture.h"
#include "font.h"
#include "log.h"

#include <variant>
#include <map>
#include <array>

namespace SpaceEngine
{
    enum MATERIAL_TYPE
    {
        BASE_MAT,
        PBR_MAT
    };

    enum TEXTURE_TYPE 
    {
        TEXTURE_TYPE_FIRST = 0,
        ALBEDO_TEX = TEXTURE_TYPE_FIRST,      
        DIFFUSE_TEX,
        SPECULAR_TEX,
        NORMAL_TEX,
        METALNESS_TEX,
        EMISSIVE_TEX,
        NORMAL_CAMERA_TEX,
        EMISSION_COLOR_TEX,
        ROUGHNESS_TEX,
        AMBIENT_OCCLUSION_TEX,
        CLEARCOAT_TEX,
        CLEARCOAT_ROUGHNESS_TEX,
        CLEARCOAT_NORMAL_TEX,
        TEXTURE_TYPE_LAST = CLEARCOAT_NORMAL_TEX 
    };

    enum VALUE_TYPE 
    {
        VALUE_TYPE_FIRST = TEXTURE_TYPE_LAST+1,
        DIFFUSE_COLOR_VAL = VALUE_TYPE_FIRST,      // Base color / diffuse / albedo
        BASE_COLOR_VAL,
        AMBIENT_COLOR_VAL,
        SPECULAR_COLOR_VAL,
        EMISSIVE_COLOR_VAL,
        METALNESS_VAL,
        ROUGHNESS_VAL,
        VALUE_TYPE_LAST = ROUGHNESS_VAL
    };

    class BaseMaterial 
    {

        public:
            using PropertyValue = std::variant<
                float,
                int,
                bool,
                glm::vec2,
                glm::vec3,
                Vector4,
                glm::mat3,
                glm::mat4>;
    
            
            virtual ~BaseMaterial() {} 
            void bindingPropsToShader();
            void bindingPropsToShader(ShaderProgram* pShaderProg);
            ShaderProgram* getShader();
            int addTexture(const std::string& nameTex, Texture* pTex);
            int addProperty(const std::string& nameProp, PropertyValue val);
            int removeProperty(const std::string& nameProp);
            int removeTexture(const std::string& nameTex);
            Texture* getTexture(std::string nameTex);
            
            std::string name;
            std::unordered_map<std::string, PropertyValue> props;
            //name subroutine/ bool is active
            //when switch for a subroutine to another remember to turn of the subroutine
            struct subroutineInfo
            {
                bool active = false;
                std::string type;
            };

            //subroutines["nameSubroutine, nameSubroutineUniform"]
            std::unordered_map<std::string, subroutineInfo> subroutines;
            ShaderProgram* pShader = nullptr;
            protected:
                BaseMaterial() = default;
                BaseMaterial(std::string name);
                BaseMaterial(std::unordered_map<std::string, PropertyValue> initProps)
                    : props(std::move(initProps))
                {}
                std::unordered_map<std::string, Texture*> texs;
                unsigned int settedTexs = 0;
                virtual void bindSubroutines(); 
            friend class MaterialManager;
    };

    class TextMaterial : public BaseMaterial
    {
        friend class MaterialManager;
        public:
        std::array<std::array<float, 4>, 6> bindCharacter(char c, float& offsetX, float resScale,  Vector2 pos, Transform2D& transf)
        {
            if(m_font.find(c) == m_font.end())
            {
                SPACE_ENGINE_FATAL("TextMaterial: Charater not found");
                exit(-1);
            }

            Character ch = m_font[c];
            //bind texture
            ch.pTex->bind(); 
            GL_CHECK_ERRORS();
            float xpos = offsetX + ch.bearing.x * transf.scale.x * resScale;
            float ypos = pos.y - (ch.size.y - ch.bearing.y) * transf.scale.y * resScale;

            float w = ch.size.x * transf.scale.x * resScale;
            float h = ch.size.y * transf.scale.y * resScale;
        
            std::array<std::array<float, 4>, 6> vertices
            {{
                { xpos,     ypos + h,   0.0f, 1.0f },
                { xpos,     ypos,       0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 0.0f },
                
                { xpos,     ypos + h,   0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 0.0f },
                { xpos + w, ypos + h,   1.0f, 1.0f }
            }};

            offsetX += (ch.advance >> 6) * transf.scale.x * resScale;
            
            return vertices;
        }

        private:
         TextMaterial(const std::string& nameFont)
         {
            props = 
            {
                {"color_val", Vector3{1.f, 1.f, 1.f}}
            };

            auto pFont = FontLoader::getFont(nameFont); 
            if(!pFont)
            {
                SPACE_ENGINE_FATAL("Font: {} not loaded");
                exit(-1);
            }

            m_font = *pFont;
         }

         std::map<char, Character> m_font;
    };

    class PBRMaterial : public BaseMaterial
    {
        private:
            PBRMaterial()
            {
                texs = 
                {
                    {"albedo_tex", nullptr},
                    {"metalness_tex", nullptr},
                    {"roughness_tex", nullptr},
                    {"normal_map_tex", nullptr},
                    {"ambient_occlusion_tex", nullptr}
                };

                props = 
                {
                    
                    {"albedo_color_val", Vector4{0.f, 0.f, 0.f, 1.f}},
                    {"metalness_val", float{0.f}},
                    {"roughness_val", float{0.f}},
                    {"ambient_occlusion_val", float{1.f}}
                };
            }
            
            void bindSubroutines() override;
        friend class MaterialManager;
    };

    class UIMaterial : public BaseMaterial
    {
        public: 
            virtual ~UIMaterial() {} 
        protected:
            UIMaterial()
            {
                texs = 
                {
                    {"ui_tex", nullptr},
                };

                props = 
                {
                    {"color_val", Vector4{1.f, 1.f, 1.f, 1.f}},
                };
            }
        friend class MaterialManager;
    };

    class UIButtonMaterial : public UIMaterial
    {
        public:
            void setSubroutineBase(bool flag);
            void setSubroutineHover(bool flag);
        private:
            UIButtonMaterial()
            {
                subroutines = 
                {
                    {"uiTextureBase", {true, "uiTextureMode"}},
                    {"uiTextureHover", {false, "uiTextureMode"}}
                };
            }
        friend class MaterialManager;
    };

    class MaterialManager
    {
        public:
            MaterialManager() = default;
            ~MaterialManager() = default;
            void Initialize();
            static BaseMaterial* findMaterial(const std::string nameMaterial);
            void Shutdown();

            template <typename T>
            static T* createMaterial(const std::string name, const std::string nameFont = "")
            {
                static_assert(std::is_base_of_v<BaseMaterial, T>);

                if(materialsMap.find(name) == materialsMap.end())
                {
                    T* pMat = nullptr;

                    if constexpr (std::is_same_v<T, TextMaterial>)
                    {
                        pMat = new TextMaterial(nameFont);    
                    }
                    else pMat = new T();
                    
                    pMat->name = name;
                    materialsMap[name] = pMat;
                    return pMat;   
                }

                return dynamic_cast<T*>(materialsMap[name]);
            }

        private:
            static std::unordered_map<std::string, BaseMaterial*> materialsMap;
    };
}