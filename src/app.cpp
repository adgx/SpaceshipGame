#include "app.h"
#include "log.h"
#include "camera.h"
#include "player.h"
#include "enemyShip.h"
#include "asteroid.h"
#include "playerShip.h"
#include <vector>

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
        delete pScene;
        delete renderer;
    }
    
    void App::Start()
    {
        //initialize main scene
        pScene = new Scene(&physicsManager);
        //crea e inizializza il player
        PlayerShip* pPlayer = new PlayerShip(pScene, "TestCube.obj");
        pPlayer->Init();
        //GameObject* pCube = new GameObject();
        //make another cube
        /*pCube->addComponent(pPlayer->getComponent<Mesh>()); // this avoid to reallocate mesh cube data
        //pCube->addComponent(new Transform());
        //pCube->getComponent<Transform>()->setWorldPosition(Vector3{0.f, 0.f, -3.f});
        pCube->addComponent(new Collider(pCube));*/
        //add GameObject to the scene
        pScene->addSceneComponent<GameObject*>(pPlayer);

        //TODO: initialize correctly the camera please 
        PerspectiveCamera* pCamera = new PerspectiveCamera();
        pCamera->transf.translateGlobal(Vector3(0.f, 3.f, 3.f));
        
        pCamera->transf.lookAt(pPlayer->getComponent<Transform>()->getWorldPosition());
        pScene->addSceneComponent<PerspectiveCamera*>(pCamera);
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

        //var per timer di spawn
        float asteroidTimer = 0.0f;
        float enemyTimer = 0.0f;

        //Gathers
        std::vector<RenderObject> worldRenderables;
        std::vector<UIRenderObject> uiRenderables;
        std::list<Collider> l;

        while(!windowManager.WindowShouldClose())
        {
            currentTime = static_cast<float>(glfwGetTime()); 
            float dt = currentTime - lastTime;
            lastTime = static_cast<float>(glfwGetTime());
           
            //collision/physic system
            accumulator += dt;
            while(accumulator >= fixed_dt)
            {
                physicsManager.Step(fixed_dt);
                accumulator -= fixed_dt;
            }


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

           

            asteroidTimer += dt;
            enemyTimer += dt;
            float spawnRadius = 5.0f; //distanza dal centro entro quale possono essere generati gli elementi
            //da piazzare da una altra parte
            //spawn asteroidi
            if(asteroidTimer >= 2.0f)
            {
                Asteroid* pAsteroid = new Asteroid(pScene, "TestCube.obj");
                float angle = (float)(rand() % 90);
                float rad = (angle) * 3.14159f / 180.0f;
                float z = -15.f + cos(rad) * spawnRadius;
                angle = (float)(rand() % 360);
                rad = (angle) * 3.14159f / 180.0f;
                float x = cos(rad) * spawnRadius;
                float y = sin(rad) * spawnRadius;

                //pAsteroid->Init(); per implementare generazione asteroidi di diverse dimensioni
                Transform* t = pAsteroid->getComponent<Transform>();
                if(t) t->setWorldPosition(Vector3(x, y, z));
                pScene->addSceneComponent(pAsteroid);
                //scene->requestInstatiate(pAsteroid);
                asteroidTimer = 0.0f;
            }
 
            //spawn navicelle nemiche
            if (enemyTimer >= 3.0f) {
                EnemyShip* pEnemy = new EnemyShip(pScene, "TestCube.obj");
                
                // Posizione randomica
                float angle = (float)(rand() % 360);
                float rad = angle * 3.14159f / 180.0f;
                float x = cos(rad) * spawnRadius;
                float y = 0.f;
                float z = -12.f;

                // Inizializza nemico (posizione e tipo)
                pEnemy->Init(Vector3(x, y, z), EnemyType::NORMAL, nullptr);
                pScene->addSceneComponent(pEnemy);
                //scene->requestInstatiate(enemy);
                enemyTimer = 0.0f;
            }

            //update game objects in the scene
            pScene->Update(dt);

            //collects the renderizable objects in the scene
            pScene->gatherRenderables(worldRenderables, uiRenderables);
            //before rendering
            glClearColor(0.1f, 0.1f, 0.1f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            BaseCamera* cam = pScene->getActiveCamera();
            
            renderer->render(worldRenderables, *(pScene->getActiveCamera()), pScene->getSkybox());
            windowManager.PollEvents();
            windowManager.SwapBuffers();
        }
    }
};
