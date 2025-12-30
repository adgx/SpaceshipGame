#pragma once
#include "renderer.h"
#include "scene.h"
#include "managers/logManager.h"
#include "managers/inputManager.h"
#include "managers/windowManager.h"
#include "managers/sceneManager.h"
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
            void InputHandle();
            //managers
            LogManager logManager;
            InputManager inputManager;
            MaterialManager materialManager;
            TextureManager textureManager;
            ShaderManager shaderManager;
            PhysicsManager physicsManager;
            SceneManager sceneManager;
            //Objects
            Scene* pScene;
            Renderer* renderer;
            UIRenderer* uiRenderer;
            WindowManager windowManager;
            InputHandler* inputHandler;
    };
};