#include "titleScreen.h"
#include "inputManager.h"
#include "windowManager.h"
#include "log.h"
#include "sceneManager.h"

#include <functional>

namespace SpaceEngine{

    TitleScreen::TitleScreen(PhysicsManager* pPhyManager, AudioManager* am):Scene(pPhyManager)
    {
        setAudioManager(am);
        Init();
        SPACE_ENGINE_DEBUG("Title Screen opening...");
    }

    TitleScreen::~TitleScreen(){
        SPACE_ENGINE_DEBUG("Going from Title Screen to another scene...");
    }

    void TitleScreen::Init(){
        SPACE_ENGINE_DEBUG("Loading textures...");
        //assign the name(important)
        name = "TitleScreen";
        if (getAudioManager()) {
            getAudioManager()->PlayMusic("menu_music", true);
        }
        m_lastWidth = WindowManager::width;
        m_lastHeight = WindowManager::height;
        //Add UILayout 
        UILayout* pUILayout = new UILayout();
        addSceneComponent(pUILayout);
        //background
        UIMaterial* pBackgroundMat = MaterialManager::createMaterial<UIMaterial>("BGTitleMenu");
        Texture* pTexBGTitleMenu = TextureManager::load(TEXTURES_PATH"backgrounds/TitleScreen.png");
        pBackgroundMat->addTexture("ui_tex", pTexBGTitleMenu);
        Background* pBackground = new Background(pBackgroundMat);
        pUILayout->addUIElement(pBackground);
        //Add UINav
        pUILayout->addComponent<UINavigator>();
        //Buttons
        UIButtonMaterial* pStartMat = MaterialManager::createMaterial<UIButtonMaterial>("StartButton");
        UIButtonMaterial* pExitMat = MaterialManager::createMaterial<UIButtonMaterial>("ExitButton");
        UIButtonMaterial* pSettingMat = MaterialManager::createMaterial<UIButtonMaterial>("SettingsButton");
        Texture* pTexStart = TextureManager::load(TEXTURES_PATH"buttons/NewGame.png");
        Texture* pTexExit = TextureManager::load(TEXTURES_PATH"buttons/Exit.png");
        Texture* pTexSettings = TextureManager::load(TEXTURES_PATH"buttons/Options.png");
        //now We don't have the texture for the hover case
        pStartMat->addTexture("ui_tex", pTexStart);
        pExitMat->addTexture("ui_tex", pTexExit);
        pExitMat->addTexture("ui_hover_tex", pTexExit);
        pSettingMat->addTexture("ui_tex", pTexSettings);
        //Creation of the UI buttons
        //pos: x:153 y:330 | res x:1440 y:1024 | space between buttons 0.04
        Button* pStart = new Button({0.f, 0.f},
            {153.f, 330.f},
            pStartMat, 
            [this]() {return StartNewGame();}
        ); 
        //pos: x:153 y:464
        Button* pExit = new Button({0.f, 0.f}, 
            {153.f, 570.f},
            pExitMat, 
            [this]() {return ExitGame();}
        );
        //pos: x:153 y:397
        Button* pSettings = new Button({0.f, 0.f}, 
            {153.f, 450.f},
            pSettingMat, 
            [this]() {SceneManager::SwitchScene("SettingsScene");
                return true;}
        );
        pUILayout->addUIElement(pStart);
        pUILayout->addUIElement(pExit);
        pUILayout->addUIElement(pSettings);
    }

    void TitleScreen::UpdateScene(float dt)
    {
        if (WindowManager::width != m_lastWidth || WindowManager::height != m_lastHeight)
        {
            m_lastWidth = WindowManager::width;
            m_lastHeight = WindowManager::height;
            notifyChangeRes(); 
        }
        if (Keyboard::keyDown(SPACE_ENGINE_KEY_BUTTON_ESCAPE))
        {
            ExitGame(); // Chiama la tua funzione che fa exit(0)
        }
    }

    bool TitleScreen::StartNewGame()
    {
        SPACE_ENGINE_INFO("StartNewGame");
        //for debugging
        //We assuming that SpaceScene is early loaded but set not active by the app
        if (auto* am = getAudioManager()) 
        {
            am->PlayMusic("bg_music", true);
        }
        SceneManager::SwitchScene("SpaceScene");//Maybe is better to use another approach
        return true;

    }

    bool TitleScreen::ExitGame()
    {
        exit(0);
        return true;
    }
}