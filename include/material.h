#pragma once
#include "utils/utils.h"
#include "shader.h"
#include "texture.h"
#include <variant>

namespace SpaceEngine
{
  
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
    
            BaseMaterial() = default;
            BaseMaterial(std::string);
            BaseMaterial(std::unordered_map<std::string, PropertyValue> initProps)
                : props(std::move(initProps))
            {}
            void addTexture(std::string name, Texture* pTex);
            void bindingPropsToShader();
            std::string name;
            std::unordered_map<std::string, PropertyValue> props;
            ShaderProgram* pShader = nullptr;
            protected:
                std::unordered_map<std::string, Texture*> texs;
    };

    class PBRMaterial : public BaseMaterial
    {
        public:
            PBRMaterial()
            {
                texs = 
                {
                    {"albedo_tex", nullptr},
                    {"metalness_tex", nullptr},
                    {"roughness_tex", nullptr},
                    {"specular_tex", nullptr},
                    {"diffuse_tex", nullptr},
                    {"normal_map_tex", nullptr},
                    {"ambient_occlusion_tex", nullptr}
                };

                props = 
                {
                    
                    {"ambient_color_val", Vector4{0.f, 0.f, 0.f, 1.f}},
                    {"diffuse_color_val", Vector3{0.f, 0.f, 0.f}},
                    {"specular_color_val", Vector3{0.f, 0.f, 0.f}},
                    {"base_color_val", Vector4{0.f, 0.f, 0.f, 1.f}},
                    {"metalness_val", float{0.f}},
                    {"roughness_val", float{0.f}},
                    {"emissive_color_val", Vector3{0.f, 0.f, 0.f}}
                };
            }
    };
}