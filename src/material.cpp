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

    void BaseMaterial::addTexture(std::string name, Texture* pTex)
    {

        texs[name] = pTex;
        pTex->setBindlessHandle(texs.size());
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

}