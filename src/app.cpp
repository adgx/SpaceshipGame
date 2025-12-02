#include "app.h"
#include "log.h"
#include "camera.h"
#include "player.h"
#include <vector>

namespace SpaceEngine{
    //static functions
    static void joystick_callback(int jid, int event);
    
    App::App()
    {
        //initialize Managers
        logManager.Initialize();
        windowManager.Initialize();
        inputManager.Initialize();
        shaderManager.Initialize();
        materialManager.Initialize();
        textureManager.Initialize();
        //Objects
        renderer = new Renderer();
        SPACE_ENGINE_INFO("Initilization app done");
        //scene 
        SPACE_ENGINE_DEBUG("Start the initialization the scene");
        Start();
        SPACE_ENGINE_INFO("Initialization the scene done");
    }
    
    App::~App()
    {
        //Shutdown Managers
        textureManager.Shutdown();
        materialManager.Shutdown();
        shaderManager.Shutdown();
        inputManager.Shutdown();
        windowManager.Shutdown();
        logManager.Shutdown();
        delete scene;
        delete renderer;
    }
    
    void App::Start()
    {
        //initialize main scene
        scene = new Scene();
        //TODO: initialize correctly the camera please 
        PerspectiveCamera* pCamera = new PerspectiveCamera();
        pCamera->transf.translateGlobal(Vector3(0.f, 0.f, -3.f));
        scene->addSceneComponent<PerspectiveCamera*>(pCamera);
        Player* pPlayer = new Player("TestCube.obj");
        scene->addSceneComponent<Player*>(pPlayer);

    }
    void App::Run()
    {
        SPACE_ENGINE_DEBUG("App - GameLoop");
        //token debug stuff
        bool token = false; 
        float currentTime = static_cast<float>(glfwGetTime());
        //Gathers
        std::vector<RenderObject> worldRenderables;
        std::vector<UIRenderObject> uiRenderables;
        
        while(!windowManager.WindowShouldClose())
        {
            
            float dt = static_cast<float>(glfwGetTime()) - currentTime;
            //refresh the input 
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
            if(!token && Keyboard::keyDown(SPACE_ENGINE_KEY_BUTTON_ESCAPE) && WindowManager::fullScreenState)
            {
                token = true;
                windowManager.Windowed();
            }
            else if(!token && Keyboard::keyDown(SPACE_ENGINE_KEY_BUTTON_ESCAPE) && !WindowManager::fullScreenState)
            {
                windowManager.SetWindowShouldClose();
            }
            if(Keyboard::keyUp(SPACE_ENGINE_KEY_BUTTON_ESCAPE))
            {
                token = false;
            }

            //update game objects in the scene
            scene->Update(dt);
            //collects the renderizable objects in the scene
            scene->gatherRenderables(worldRenderables, uiRenderables);
            renderer->render(worldRenderables, *(scene->getActiveCamera()));

            glClearColor(0.f, 0.f, 0.f, 1.f);
            //before rendering
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            windowManager.PollEvents();
            windowManager.SwapBuffers();
        }
    }
};
