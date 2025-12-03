#include "log.h"
#include "material.h" 

namespace SpaceEngine
{
    template<typename T>
    bool compareTypeGL( T matType, GLenum shType) 
    {
        // There are many more types than are covered here, but
        // these are the most common in these examples.
        switch (shType) 
        {
            case GL_FLOAT:
                return std::is_same_v<decltype(matType), float>;
            case GL_FLOAT_VEC2:
                return std::is_same_v<decltype(matType), Vector2>;
            case GL_FLOAT_VEC3:
                return std::is_same_v<decltype(matType), Vector3>;
            case GL_FLOAT_VEC4:
                return std::is_same_v<decltype(matType), Vector4>;
            case GL_DOUBLE:
                return std::is_same_v<decltype(matType), double>;
            case GL_INT:
                return std::is_same_v<decltype(matType), int>;
            case GL_UNSIGNED_INT:
                return std::is_same_v<decltype(matType), unsigned int>;
            case GL_BOOL:
                return std::is_same_v<decltype(matType), bool>;
            case GL_FLOAT_MAT2:
                return std::is_same_v<decltype(matType), Matrix2>;
            case GL_FLOAT_MAT3:
                return std::is_same_v<decltype(matType), Matrix3>;
            case GL_FLOAT_MAT4:
                return std::is_same_v<decltype(matType), Matrix4>;
            default:
                return false;
        }
    }
    //BaseMaterial
    BaseMaterial::BaseMaterial(std::string name)
    {
        this->name = name;
    }

    void BaseMaterial::bindingPropsToShader(ShaderProgram* pShaderProg)
    {
        if(!pShaderProg)
        {
            SPACE_ENGINE_ERROR("Error shader is nullptr");
        }

        pShader = pShaderProg; 
        bindingPropsToShader();
    }

    void BaseMaterial::bindingPropsToShader()
    {
        if(!pShader)
        {
            SPACE_ENGINE_FATAL("No shader binding to the material");
            exit(-1);
        }

        SPACE_ENGINE_DEBUG("Binding properties material to shader");
        std::vector<std::tuple<const std::string, GLenum>> uniformsShader = pShader->getPairUniformNameLocation();
        
        for(const auto& [name, type] : uniformsShader)
        {
            if(props.find(name)!= props.end())
            {
                std::visit([&](auto val)
                {
                    if(compareTypeGL(val, type))
                    {
                        pShader->setUniform(name.c_str(), val);
                    }
                }, props[name]);
            }
            else if(texs.find(name) != texs.end())
            {
                pShader->setUniform(name.c_str(), texs[name]->getBindlessHandle());
            }
        }
    }

    ShaderProgram* BaseMaterial::getShader()
    {
        return pShader;
    }

    int BaseMaterial::addTexture(const std::string& nameTex, Texture* pTex)
    {
        auto pos = texs.find(nameTex);
        if(pos == texs.end() || (pos != texs.end() && texs[nameTex] == nullptr))
        {
            SPACE_ENGINE_INFO("Material: {}, added Texture: {}", name, nameTex);
            pTex->setBindlessHandle(static_cast<unsigned int>(GL_TEXTURE0+settedTexs));
            pTex->bind();
            settedTexs++;
            texs[nameTex] = pTex;

        }
        else
        {
            SPACE_ENGINE_WARN("Material: {}, overwrite Texture: {}", name, nameTex);
            pTex->setBindlessHandle(pos->second->getBindlessHandle());
            texs[nameTex] = pTex;
            return 2;
        }
        return 1;
    }

    int BaseMaterial::addProperty(const std::string& nameProp, PropertyValue val)
    {
        if(props.find(nameProp) == props.end())
        {
            SPACE_ENGINE_INFO("Material: {}, added Property: {}", name, nameProp);
            props[nameProp] = val;
        }
        else 
        {
            SPACE_ENGINE_WARN("Material: {}, overwrite Property: {}", name, nameProp);
            props[nameProp] = val;
            return 2;
        }
        return 1;
    }

    int BaseMaterial::removeProperty(const std::string& nameProp)
    {
        if(props.find(nameProp) != props.end())
        {
            props.erase(nameProp);
            SPACE_ENGINE_INFO("Property: {} is removed", nameProp);
            return 1;
        }
        SPACE_ENGINE_ERROR("Property: {} not found",  nameProp);
        return 0;
    }

    int BaseMaterial::removeTexture(const std::string& nameTex)
    {
        if(texs.find(nameTex) != texs.end())
        {
            props.erase(nameTex);
            settedTexs--;
            SPACE_ENGINE_INFO("Texture: {} is removed", nameTex);
            SPACE_ENGINE_INFO("Texture setted: {}", settedTexs);
            return 1;
        }
        SPACE_ENGINE_ERROR("Texture: {} not found",  nameTex);
        return 0;
    }

    //MaterialManager
    std::unordered_map<std::string, BaseMaterial*> MaterialManager::materialsMap;
    
    void MaterialManager::Initialize()
    {

    }

    BaseMaterial* MaterialManager::findMaterial(const std::string nameMaterial)
    {
        auto pos = materialsMap.find(nameMaterial);

        if(pos == materialsMap.end())
        {
            SPACE_ENGINE_ERROR("Material not found, name: {}", nameMaterial);
            return nullptr;
        }

        return pos->second;
    }

    void MaterialManager::Shutdown()
    {
        for (auto& [name, pMat] : materialsMap)
            delete pMat;
        materialsMap.clear();
    }

}