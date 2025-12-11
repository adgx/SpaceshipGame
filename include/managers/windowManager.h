#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "utils/utils.h"

#define SPACE_ENGINE_MIN_RES_W 720
#define SPACE_ENGINE_MIN_RES_H 480
#define SPACE_ENGINE_MAX_RES_W 1920
#define SPACE_ENGINE_MAX_RES_H 1080


namespace SpaceEngine
{
    class WindowManager
    {
        public:
            WindowManager() = default;
            ~WindowManager() = default;

            void Initialize();
            int WindowShouldClose();
            void SetWindowShouldClose();
            void Windowed();
            void PollEvents();
            void SwapBuffers();
            void Shutdown();

            static int height, width;
            static int xpos, ypos;
            static GLFWwindow* window;
            static GLFWmonitor* monitor;
            static bool fullScreenState;
            constexpr static const float aspectRatio = 16.f/9.f;
            static Matrix4 sceenProjMatrix; 
        private:
            bool setUpGLFW();
    };
}

