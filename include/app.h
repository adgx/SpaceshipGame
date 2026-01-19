#pragma once
#include "renderer.h"
#include "managers/logManager.h"
#include "managers/inputManager.h"
#include "managers/windowManager.h"
#include "managers/sceneManager.h"
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
            static InputHandler& GetInputHandler();
            static EAppState state;
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
            AudioManager audioManager;
            SceneManager sceneManager;
            //Objects
            Scene* pScene;
            Renderer* renderer;
            UIRenderer* uiRenderer;
            TextRenderer* textRenderer;
            WindowManager windowManager;
            static InputHandler* inputHandler;
    };
};