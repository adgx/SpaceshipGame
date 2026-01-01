#include "app.h"
#include "log.h"
#include "camera.h"
#include "player.h"
#include "playerShip.h"
#include <vector>

namespace SpaceEngine
{
    EAppState state = EAppState::START;
    InputHandler* App::inputHandler = nullptr;
    
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
        sceneManager.Initialize();
        
        //Objects
        renderer = new Renderer();
        uiRenderer = new UIRenderer();
        SPACE_ENGINE_INFO("Initilization app done");
        //scene 
        SPACE_ENGINE_DEBUG("Start the initialization the scene");
        //input handler init
        inputHandler = new InputHandler();
        SPACE_ENGINE_DEBUG("Input handler initialization done");

        Start();
        SPACE_ENGINE_INFO("Initialization the scene done");
    }
    
    App::~App()
    {
        //Shutdown Managers
        sceneManager.Shutdown();
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
        state = EAppState::START;
        //initialize main scene
        pScene = new SpaceScene(&physicsManager);
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
        pScene->addSceneComponent<PerspectiveCamera*>(pCamera);
        glError = glGetError();

        SceneManager::LoadScene(pScene);
    }

    void App::InputHandle()
    {
        bool token = false; 
        
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

    }

    InputHandler& App::GetInputHandler()
    {
        return *inputHandler;
    }


    void App::Run()
    {
        SPACE_ENGINE_DEBUG("App - GameLoop");
        state = EAppState::RUN;
        //token debug stuff
        
        float lastTime = static_cast<float>(glfwGetTime());
        float currentTime;
        //handle the tunneling caused by a to slow frame dt
        //fixed time step
        float fixed_dt = 1.f/60.f;
        float accumulator = 0.0;
        
        //Gathers
        std::vector<RenderObject> worldRenderables;
        std::vector<UIRenderObject> uiRenderables;

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

            //refresh the input data
            inputManager.Update();
            InputHandle();
            inputHandler->handleInput();

            //update game objects in the scene
            sceneManager.Update(dt);

            //collects the renderizable objects in the scene
            sceneManager.GatherRenderables(worldRenderables, uiRenderables);
            //gather scene object to rendering the scene
            RendererParams rParams{worldRenderables, 
                *(sceneManager.GetLights()), 
                sceneManager.GetActiveCamera(), 
                sceneManager.GetSkybox()};
            
            GL_CHECK_ERRORS(); 
            renderer->render(rParams);
            GL_CHECK_ERRORS();
            uiRenderer->render(uiRenderables);
            GL_CHECK_ERRORS();

            sceneManager.LateUpdate();
            
            windowManager.PollEvents();
            windowManager.SwapBuffers();
        }
    }
};
