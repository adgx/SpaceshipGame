#pragma once
#include "renderer.h"
#include "scene.h"
#include "managers/logManager.h"
#include "managers/inputManager.h"
#include "managers/windowManager.h"
#include "mesh.h"
#include "material.h"
#include "shader.h"

#include <glad/gl.h>

namespace SpaceEngine
{
    class App
    {
        public:
            App();
            ~App();
            void Start();
            void Run();
        private:
            //managers
            LogManager logManager;
            InputManager inputManager;
            MaterialManager materialManager;
            TextureManager textureManager;
            ShaderManager shaderManager;
            //Objects
            Scene* scene;
            Renderer* renderer;
            UIRenderer* uiRenderer;
            WindowManager windowManager;
    };
};