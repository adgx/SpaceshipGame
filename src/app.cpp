#include "app.h"
#include "log.h"
#include "camera.h"
#include "player.h"
#include "playerShip.h"
#include "titleScreen.h"
#include "settingsScene.h"
#include "gameOverScene.h"
#include "leaderboardScene.h"
#include "font.h"

#include <vector>

#define DEBUG_RENDERERV2 1

namespace SpaceEngine
{
    EAppState App::state = EAppState::START;
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
        rendererV2.Initialize();
        audioManager.Initialize();

        audioManager.LoadSound("menu_music", AUDIO_PATH"menu.wav");
        audioManager.LoadSound("bg_music", AUDIO_PATH"music_space.wav");
        audioManager.LoadSound("shoot_player", AUDIO_PATH"laser_player.wav");
        audioManager.LoadSound("shoot_enemy", AUDIO_PATH"laser_enemy.wav");
        audioManager.LoadSound("player_explosion", AUDIO_PATH"player_explosion.wav");
        audioManager.LoadSound("lose_hp", AUDIO_PATH"lose_hp.wav");
        audioManager.LoadSound("enemy_explosion", AUDIO_PATH"enemyexplosion.wav");
        audioManager.LoadSound("asteroid_explosion", AUDIO_PATH"asteroid_crash.wav");
        audioManager.LoadSound("game_over", AUDIO_PATH"game_over.wav");
        audioManager.LoadSound("bomb", AUDIO_PATH"bomb.wav");
        audioManager.LoadSound("heal", AUDIO_PATH"heal.wav");
        audioManager.LoadSound("rapid_fire", AUDIO_PATH"rapid_fire.wav");

        //Renderer objects 
        renderer = new Renderer();
        uiRenderer = new UIRenderer();
        textRenderer = new TextRenderer();
        screenRenderer = new ScreenRenderer();

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
        audioManager.Shutdown();
        delete renderer;
    }
    
    void App::Start()
    {
        state = EAppState::START;
        //crate material for the buller 
        BaseMaterial* pBulletMat = MaterialManager::createMaterial<BaseMaterial>("BulletMat");
        pBulletMat->addProperty("color_val", Vector4(8.f, 1.f, 1.f, 1.f));
        pBulletMat->subroutines["getColorTex"] = {false, "colorMode"};
        pBulletMat->subroutines["getColorVal"] = {true, "colorMode"};
        pBulletMat->pShader = ShaderManager::findShaderProgram("simpleTex");
        
        FontLoader::LoadFont("Orbitron-Regular.ttf");
        //initialize main scene
        pScene = new SpaceScene(&physicsManager);
        pScene->setAudioManager(&audioManager);
        pScene->Init();
        //crea e inizializza il player
        PlayerShip* pPlayer = new PlayerShip(pScene, "PlayerShipV3.obj"); 
        pPlayer->Init();
        if (SpaceScene* spaceScene = dynamic_cast<SpaceScene*>(pScene)) {
            spaceScene->SetPlayer(pPlayer);
        }
        GL_CHECK_ERRORS();
        //add GameObject to the scene
        pScene->addSceneComponent<GameObject*>(pPlayer);

        //TODO: initialize correctly the camera please 
        PerspectiveCamera* pCamera = new PerspectiveCamera();
        pCamera->transf.translateGlobal(Vector3(0.f, 3.f, 3.f));
        
        pCamera->transf.lookAt(pPlayer->getComponent<Transform>()->getWorldPosition());
        pScene->addSceneComponent<PerspectiveCamera*>(pCamera);

        //Initialize lights
        #if 0
        Light* pLight = new Light(Vector3{-10.f, 10.f, 0.f}, Vector3{25.f, 25.f, 25.f}); //left-top
        pScene->addSceneComponent<Light*>(pLight);
        pLight = new Light(Vector3{10.f, 10.f, 0.f}, Vector3{25.f, 25.f, 25.f}); //right-top
        pScene->addSceneComponent<Light*>(pLight);
        pLight = new Light(Vector3{10.f, -10.f, 0.f}, Vector3{25.f, 25.f, 25.f}); //right-bottom
        pScene->addSceneComponent<Light*>(pLight);
        pLight = new Light(Vector3{-10.f, -10.f, 0.f}, Vector3{25.f, 25.f, 25.f}); //left-bottom
        GL_CHECK_ERRORS();
        #else 
        Vector3 colorLT(0.231, 0.078, 0.474);//dark purple
        Vector3 colorRT(0.184, 0.055, 0.506);//dark purple
        Vector3 colorRB(0.443, 0.11, 0.667);//dark purple
        Vector3 colorLB(0.098, 0.035, 0.255);//dark purple
        float pow1 = 15.f;
        float pow2 = 23.f;

        Light* pLight = new Light(Vector3{-10.f, 10.f, 0.f}, colorLT*pow1, Math::getDirection(90.f, 0.f, 135.f)); //left-top
        pScene->addSceneComponent<Light*>(pLight);
        pLight = new Light(Vector3{10.f, 10.f, 0.f}, colorRT*pow2, Math::getDirection(90.f, 0.f, -135.f)); //right-top
        pScene->addSceneComponent<Light*>(pLight);
        pLight = new Light(Vector3{10.f, -10.f, 0.f}, colorRB*pow1, Math::getDirection(90.f, 0.f, -45.f)); //right-bottom
        pScene->addSceneComponent<Light*>(pLight);
        pLight = new Light(Vector3{-10.f, -10.f, 0.f}, colorLB*pow2, Math::getDirection(90.f, 0.f, 45.f)); //left-bottom
        GL_CHECK_ERRORS();
        #endif
        
        //actually the order of insert is important        
        //TitleScreen scene
        TitleScreen* pTitleScreen = new TitleScreen(&physicsManager, &audioManager);
        SceneManager::LoadScene(pTitleScreen);

        SettingsScene* pSettingsScene = new SettingsScene(&physicsManager);
        pSettingsScene->setAudioManager(&audioManager);

        pSettingsScene->setActive(false); 
        SceneManager::LoadScene(pSettingsScene);

        pScene->setActive(false);
        SceneManager::LoadScene(pScene);

        GameOverScene* pGameOver = new GameOverScene(&physicsManager, dynamic_cast<SpaceScene*>(pScene));
        pGameOver->setActive(false);
        SceneManager::LoadScene(pGameOver);

        LeaderboardScene* pLeaderboardScene = new LeaderboardScene(&physicsManager, &audioManager);
        pLeaderboardScene->setActive(false);
        SceneManager::LoadScene(pLeaderboardScene);

    }

    void App::InputHandle()
    {
        /*bool token = false; 
        
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
        }*/

    }

    InputHandler& App::GetInputHandler()
    {
        return *inputHandler;
    }


    void App::Run()
    {
        SPACE_ENGINE_DEBUG("App - GameLoop");
        //token debug stuff
        
        float lastTime = static_cast<float>(glfwGetTime());
        float currentTime;
        //handle the tunneling caused by a to slow frame dt
        //fixed time step
        float fixed_dt = 1.f/30.f;
        float accumulator = 0.0;
        
        //Gathers
        std::vector<RenderObject> worldRenderables;
        std::vector<UIRenderObject> uiRenderables;
        std::vector<TextRenderObject> textRenderables;
        std::vector<ScreenRenderObject> screenRenderables;

        while(!windowManager.WindowShouldClose())
        {
            currentTime = static_cast<float>(glfwGetTime()); 
            float dt = currentTime - lastTime;
            lastTime = currentTime;
           
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

            //refresh the input data
            inputManager.Update();
            InputHandle();
            inputHandler->handleInput();

            //SPACE_ENGINE_INFO("Scene Update Start");
            //update game objects in the scene
            sceneManager.Update(dt);

            //collects the renderizable objects in the scene
            sceneManager.GatherRenderables(worldRenderables, 
                uiRenderables,
                textRenderables,
                screenRenderables);
            //gather scene object to rendering the scene
            RendererParams rParams{worldRenderables, 
                *(sceneManager.GetLights()), 
                sceneManager.GetActiveCamera(), 
                sceneManager.GetSkybox()};
            
            #if !DEBUG_RENDERERV2
            GL_CHECK_ERRORS();
            screenRenderer->render(screenRenderables);
            GL_CHECK_ERRORS();
            renderer->render(rParams);
            GL_CHECK_ERRORS();
            uiRenderer->render(uiRenderables);
            GL_CHECK_ERRORS();
            textRenderer->render(textRenderables);
            GL_CHECK_ERRORS();
            #else
            GL_CHECK_ERRORS();
            rendererV2.clear();
            GL_CHECK_ERRORS();
            rendererV2.render(screenRenderables);
            GL_CHECK_ERRORS();
            rendererV2.render(rParams);
            GL_CHECK_ERRORS();
            rendererV2.render(uiRenderables);
            GL_CHECK_ERRORS();
            rendererV2.render(textRenderables);
            GL_CHECK_ERRORS();
            rendererV2.postprocessing(sceneManager.GetActiveScene()->getPostprocessing());
            GL_CHECK_ERRORS();
            #endif

            
            sceneManager.LateUpdate();
            
            windowManager.PollEvents();
            windowManager.SwapBuffers();
        }
    }
};
