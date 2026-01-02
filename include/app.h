#pragma once
#include "renderer.h"
#include "scene.h"
#include "managers/logManager.h"
#include "managers/inputManager.h"
#include "managers/windowManager.h"
#include "managers/audioManager.h"
#include "collisionDetection.h"
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
            void Run();
        private:
            void Start();
            //managers
            LogManager logManager;
            InputManager inputManager;
            MaterialManager materialManager;
            TextureManager textureManager;
            ShaderManager shaderManager;
            PhysicsManager physicsManager;
            AudioManager audioManager;
            
            //Objects
            Scene* pScene;
            Renderer* renderer;
            UIRenderer* uiRenderer;
            WindowManager windowManager;
            
    };
};