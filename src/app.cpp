#include "app.h"
#include "log.h"

namespace SpaceEngine{
    //static functions
    static void joystick_callback(int jid, int event);
    
    App::App()
    {
        //initialize Managers
        logManager.Initialize();
        windowManager.Initialize();
        inputManager.Initialize();
        //Objects
        scene = new Scene();
        renderer = new Renderer();
        SPACE_ENGINE_INFO("Initilization app done");
    }
    
    App::~App()
    {
        //Shutdown Managers
        inputManager.Shutdown();
        windowManager.Shutdown();
        logManager.Shutdown();
        delete scene;
        delete renderer;
    }

    void App::Run()
    {
        SPACE_ENGINE_DEBUG("App - GameLoop");
        //token debug stuff
        bool token = false; 
        while(!windowManager.WindowShouldClose())
        {
            
            inputManager.Update();
            //mouse debug
            #if 0
            if(Mouse::buttonDown(SPACE_ENGINE_MOUSE_BUTTON_LEFT))
                SPACE_ENGINE_DEBUG("Left mouse button pressed");
            if(Mouse::buttonDown(SPACE_ENGINE_MOUSE_BUTTON_RIGHT))
                SPACE_ENGINE_DEBUG("Right mouse button pressed");
            #endif
            #if 0
            if(Joystick::buttonDown(SPACE_ENGINE_JK_BUTTON_A))
                SPACE_ENGINE_DEBUG("Joystick: Pressed button A");
            if(Joystick::buttonDown(SPACE_ENGINE_JK_BUTTON_B))
                SPACE_ENGINE_DEBUG("Joystick: Pressed button B");
            SPACE_ENGINE_DEBUG("Joystick: Left axis x:{} Left axis y:{}", Joystick::axis(SPACE_ENGINE_JK_AXIS_LEFT_X), 
            Joystick::axis(SPACE_ENGINE_JK_AXIS_LEFT_Y));
            #endif
            //fast debug for windowed and fullwindow feature
            if(!token && Keyboard::keyDown(SPACE_ENGINE_KEY_BUTTON_ESCAPE) && Managers::Window::fullScreenState)
            {
                token = true;
                windowManager.Windowed();
            }
            else if(!token && Keyboard::keyDown(SPACE_ENGINE_KEY_BUTTON_ESCAPE) && !Managers::Window::fullScreenState)
            {
                windowManager.SetWindowShouldClose();
            }
            if(Keyboard::keyUp(SPACE_ENGINE_KEY_BUTTON_ESCAPE))
            {
                token = false;
            }
            glClearColor(0.f, 0.f, 0.f, 1.f);
            //before rendering
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            windowManager.PollEvents();
            windowManager.SwapBuffers();
        }
    }
};
