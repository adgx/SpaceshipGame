#include "app.h"
#include "log.h"
#include "camera.h"
#include "player.h"
#include "playerShip.h"
#include <vector>
#include "audioManager.h"

namespace SpaceEngine{
    //static functions
    static void joystick_callback(int jid, int event);
    
    App::App()
    {
        //initialize Managers
        logManager.Initialize();
        windowManager.Initialize();
        physicsManager.Initialization();
        inputManager.Initialize();
        shaderManager.Initialize();
        materialManager.Initialize();
        textureManager.Initialize();
        audioManager.Initialize();

        audioManager.LoadSound("menu_music", "assets/audio/menu.wav");
        audioManager.LoadSound("bg_music", "assets/audio/music_space.wav");
        audioManager.LoadSound("shoot_player", "assets/audio/laser_player.wav");
        audioManager.LoadSound("shoot_enemy", "assets/audio/laser_enemy.wav");
        audioManager.LoadSound("enemy_explosion", "assets/audio/enemyexplosion.wav");
        audioManager.LoadSound("asteroid_explosion", "assets/audio/asteroid_explosion.wav");
        audioManager.LoadSound("game_over", "assets/audio/game_over.wav");

        audioManager.PlayMusic("bg_music", true);
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
        physicsManager.Shutdown();
        windowManager.Shutdown();
        logManager.Shutdown();
        audioManager.Shutdown();
        delete pScene;
        delete renderer;
    }
    
    void App::Start()
    {
        //initialize main scene
        pScene = new Scene(&physicsManager);
        pScene->setAudioManager(&audioManager);
        pScene->Init();
        //crea e inizializza il player
        PlayerShip* pPlayer = new PlayerShip(pScene, "TestCube.obj");
        pPlayer->Init();
        auto glError = glGetError();
        
        //add GameObject to the scene
        pScene->addSceneComponent<GameObject*>(pPlayer);


        //TODO: initialize correctly the camera please 
        PerspectiveCamera* pCamera = new PerspectiveCamera();
        pCamera->transf.translateGlobal(Vector3(0.f, 3.f, 3.f));
        
        pCamera->transf.lookAt(pPlayer->getComponent<Transform>()->getWorldPosition());
        pScene->addSceneComponent<PerspectiveCamera*>(pCamera);

        //Initialize lights
        Light* pLight = new Light(Vector3{-10.f, 10.f, 0.f}, Vector3{1.f, 1.f, 1.f}); //left-top
        pScene->addSceneComponent<Light*>(pLight);
        pLight = new Light(Vector3{10.f, 10.f, 0.f}, Vector3{1.f, 1.f, 1.f}); //right-top
        pScene->addSceneComponent<Light*>(pLight);
        pLight = new Light(Vector3{10.f, -10.f, 0.f}, Vector3{1.f, 1.f, 1.f}); //right-bottom
        pScene->addSceneComponent<Light*>(pLight);
        pLight = new Light(Vector3{-10.f, -10.f, 0.f}, Vector3{1.f, 1.f, 1.f}); //left-bottom
        glError = glGetError();
    }

    void App::Run()
    {
        SPACE_ENGINE_DEBUG("App - GameLoop");
        //token debug stuff
        bool token = false; 
        float lastTime = static_cast<float>(glfwGetTime());
        float currentTime;
        //handle the tunneling caused by a to slow frame dt
        //fixed time step
        float fixed_dt = 1.f/60.f;
        float accumulator = 0.0;
        
        //Gathers
        std::vector<RenderObject> worldRenderables;
        std::vector<UIRenderObject> uiRenderables;
        std::list<Collider> l;

        while(!windowManager.WindowShouldClose())
        {
            currentTime = static_cast<float>(glfwGetTime()); 
            float dt = currentTime - lastTime;
            lastTime = currentTime;
           
            if (dt > 0.05f) dt = 0.05f;
            //collision/physic system
            accumulator += dt;
            //SPACE_ENGINE_INFO("Accumulator: {}, dt: {}", accumulator, dt);
            while(accumulator >= fixed_dt)
            {
                //SPACE_ENGINE_INFO("Physics Step Start");
                physicsManager.Step(fixed_dt);
                //SPACE_ENGINE_INFO("Physics Step End");
                accumulator -= fixed_dt;
            }

            //SPACE_ENGINE_INFO("Scene Update Start");
            //refresh the input 
            inputManager.Update();
            //SPACE_ENGINE_INFO("Scene Update End");

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

            //SPACE_ENGINE_INFO("Scene Update Start");
            //update game objects in the scene
            pScene->Update(dt);
            //SPACE_ENGINE_INFO("Scene Update End");

            //collects the renderizable objects in the scene
            pScene->gatherRenderables(worldRenderables, uiRenderables);
            //before rendering
            glClearColor(1.f, 1.f, 1.f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            //gather scene object to rendering the scene
            BaseCamera* pCam = pScene->getActiveCamera();
            std::vector<Light*>* pLights = pScene->getLights();
            RendererParams rParams{worldRenderables, *(pLights), *(pCam), pScene->getSkybox()};
            
            auto glError = glGetError();
            //SPACE_ENGINE_INFO("Renderer Start");
            renderer->render(rParams);
            //SPACE_ENGINE_INFO("Renderer End");
            glError = glGetError();
            
            windowManager.PollEvents();
            windowManager.SwapBuffers();
        }
    }
};
