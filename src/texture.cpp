#include "texture.h"
#include "utils/stb_image.h"
#include "utils/utils.h"
#include "log.h"

namespace SpaceEngine
{
    Texture::Texture(GLenum textureTarget, const std::string& FileName)
    {
        this->textureTarget = textureTarget;
        this->fileName      = fileName;
    }

    Texture::Texture(GLenum textureTarget)
    {
        this->textureTarget = textureTarget;
    }

    void Texture::bind(GLenum textureUnit)
    {
        bindInternal(textureUnit);
    }

    void Texture::bind()
    {
        if(bindlessHandle!=-1)
            bindInternal(bindlessHandle);
        else
        {
            SPACE_ENGINE_ERROR("Error to binding the texture: {}", fileName);
        }
    }

    void Texture::bindInternal(GLenum textureUnit)
    {
        glActiveTexture(textureUnit);
        glBindTexture(textureTarget, textureObj);
    }
    //Texture manager
    std::unordered_map<std::string, Texture*> TextureManager::texMap;
    
    void TextureManager::insert(std::string& nameKey, Texture* pTex)
    {
        auto it = texMap.find(nameKey);
        if(it == texMap.end())
        {
            texMap[nameKey] = pTex;
        }
    }

    Texture* TextureManager::load(const std::string& path, bool isSRGB)
    {
        Texture* pTex = new Texture(GL_TEXTURE_2D);
        SPACE_ENGINE_INFO("Loading texture");
        
        if(!Utils::fileExists(path))
        {
            SPACE_ENGINE_ERROR("File Texture not found, dir{}", path);
            exit(-1);
        }
        pTex->path = path;
        pTex->fileName = Utils::getFileNameFormPath(path);

        if (!load(pTex, isSRGB)) {
            return nullptr;
        }

        insert(pTex->fileName, pTex);
        
        return pTex;
    }

    bool TextureManager::load(Texture* pTex, bool isSRGB)
    {
        unsigned char* pImageData = NULL;
        stbi_set_flip_vertically_on_load(1);
        pImageData = stbi_load(pTex->path.c_str(), &(pTex->imageWidth), &(pTex->imageHeight), &(pTex->imageBPP), 0);

        if (!pImageData) {
            SPACE_ENGINE_ERROR("Can't load texture from '{}' - {}", pTex->path, stbi_failure_reason());
            return false;
        }

        SPACE_ENGINE_INFO("Loaded texture '{}' width {}, height {}, bpp {}", pTex->path, pTex->imageWidth, pTex->imageHeight, pTex->imageBPP);
        loadInternal(pTex, pImageData, isSRGB);
        stbi_image_free(pImageData);
        
        return true;
    }

    Texture* TextureManager::load(const std::string& texName, uint32_t bufferSize, void* pBufferData, bool isSRGB)
    {
        Texture* pTex = new Texture(GL_TEXTURE_2D);
        pTex->fileName = texName;
        void* pImageData = stbi_load_from_memory((const stbi_uc*)pBufferData, bufferSize, &(pTex->imageWidth), &(pTex->imageHeight), &(pTex->imageBPP), 0);
        loadInternal(pTex, pImageData, isSRGB);
        stbi_image_free(pImageData);

        insert(pTex->fileName, pTex);

        return pTex;
    }



    Texture* TextureManager::loadRaw(const std::string& texName, int width, int height, int BPP, const unsigned char* pImageData, bool isSRGB)
    {
        Texture* pTex = new Texture(GL_TEXTURE_2D);
        pTex->fileName = texName;

        pTex->imageWidth = width;
        pTex->imageHeight = height;
        pTex->imageBPP = BPP;

        loadInternal(pTex, pImageData, isSRGB);
        insert(pTex->fileName, pTex);
        return pTex;
    }

    void TextureManager::loadInternal(Texture* pTex, const void* pImageData, bool isSRGB)
    {
        glGenTextures(1, &(pTex->textureObj));
        glBindTexture(pTex->textureTarget, pTex->textureObj);

        GLenum internalFormat = GL_NONE;

        if (pTex->textureTarget == GL_TEXTURE_2D) 
        {
            switch (pTex->imageBPP) 
            {
                case 1: 
                    {
                        glTexImage2D(pTex->textureTarget, 0, GL_RED, pTex->imageWidth, pTex->imageHeight, 0, GL_RED, GL_UNSIGNED_BYTE, pImageData);
                        GLint SwizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_RED };
                        glTexParameteriv(pTex->textureTarget, GL_TEXTURE_SWIZZLE_RGBA, SwizzleMask);
                    }
                    break;

                case 2:
                    glTexImage2D(pTex->textureTarget, 0, GL_RG, pTex->imageWidth, pTex->imageHeight, 0, GL_RG, GL_UNSIGNED_BYTE, pImageData);
                    break;

                case 3:
                    internalFormat = isSRGB ? GL_SRGB8 : GL_RGB8;
                    glTexImage2D(pTex->textureTarget, 0, internalFormat, pTex->imageWidth, pTex->imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pImageData);
                    break;

                case 4:
                    internalFormat = isSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
                    glTexImage2D(pTex->textureTarget, 0, internalFormat, pTex->imageWidth, pTex->imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImageData);
                    break;

                default: SPACE_ENGINE_FATAL("Support for BPP {} is not implemented\n", pTex->imageBPP);
            }
        } 
        else 
        {
            SPACE_ENGINE_FATAL("Support for texture target {} is not implemented\n", pTex->textureTarget);
            exit(1);
        }

        glTexParameteri(pTex->textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(pTex->textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(pTex->textureTarget, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(pTex->textureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(pTex->textureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glGenerateMipmap(pTex->textureTarget);

        glBindTexture(pTex->textureTarget, 0);
    }

    Texture* TextureManager::findTexture(const std::string& texName)
    {
        auto it = texMap.find(texName);
        if(it != texMap.end())
        {
            return it->second;
        }
        return nullptr;
    }

    void TextureManager::Shutdown()
    {
        for (auto& [name, pTex] : texMap)
            delete pTex;
        texMap.clear();
    }


}