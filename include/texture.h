#pragma once

#include <string>

#include <glad/gl.h>
#include <unordered_map>

namespace SpaceEngine
{
    class Texture
    {
        public:
            void bind(GLenum textureUnit);
            void bind();
            
            void getImageSize(int& imageWidth, int& imageHeight)
            {
                imageWidth = this->imageWidth;
                imageHeight = this->imageHeight;
            }

            GLuint getTexture() const { return textureObj;}
            GLuint getBindlessHandle() const { return bindlessHandle;}
            void setBindlessHandle(GLuint texUnit) { bindlessHandle = texUnit;}

            private:
                Texture(GLenum textureTarget, const std::string& path);
                Texture(GLenum textureTarget); 
                void bindInternal(GLenum textureUnit);
                
                std::string fileName;
                std::string path;
                GLuint bindlessHandle = -1;
                int imageWidth = 0;
                int imageHeight = 0;
                int imageBPP = 0;
                GLenum textureTarget = 0;
                GLenum textureObj = 0;

        friend class TextureManager;
    }; 

    class TextureManager
    {
        public:
            TextureManager() = default;
            ~TextureManager() = default;
            void Initialize() {};
            static Texture* load(const std::string& path, bool isSRGB = false);
            static Texture* load(const std::string& texName, uint32_t bufferSize, void* pBufferData, bool isSRGB = false);
            static Texture* loadRaw(const std::string&, int width, int height, int BPP, const unsigned char* pImageData, bool isSRGB);
            static Texture* findTexture(const std::string& texName);
            void Shutdown();
        private:
            static bool load(Texture* pTex, bool isSRGB = false);
            static void loadInternal(Texture* pTex, const void* pImageData, bool IsSRGB);
            static void insert(std::string& nameKey, Texture* pTex);

        private:
            static std::unordered_map<std::string, Texture*> texMap;
    };
}