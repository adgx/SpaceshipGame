#pragma once
#include "shader.h"
#include "texture.h"

namespace SpaceEngine
{
    class Skybox
    {
        public:
            Skybox();
            ~Skybox() = default;

            void init();
            void draw();
            void bindTex();
            void bindVAO();
            ShaderProgram* pShader = nullptr;
        private:
            unsigned int VAO = 0;
            unsigned int VBO = 0;
            Texture* pCubeMapTex = nullptr;
    };
}