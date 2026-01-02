#include "titleScreen.h"
#include "inputManager.h"
#include "windowManager.h"
#include "log.h"
#include "sceneManager.h"

#include <functional>

namespace SpaceEngine{

    TitleScreen::TitleScreen(PhysicsManager* pPhyManager):Scene(pPhyManager)
    {
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
        Texture* pTexStart = TextureManager::load(TEXTURES_PATH"buttons/NewGame.png");
        Texture* pTexExit = TextureManager::load(TEXTURES_PATH"buttons/Exit.png");
        //now We don't have the texture for the hover case
        pStartMat->addTexture("ui_tex", pTexStart);
        pExitMat->addTexture("ui_tex", pTexExit);
        pExitMat->addTexture("ui_hover_tex", pTexExit);
        //Creation of the UI button
        //pos: x:153 y:330 | res x:1440 y:1024 | space between buttons 0.04
        Button* pStart = new Button({0.11, 0}, 
            pStartMat, 
            [this]() {return StartNewGame();}
        ); 
        //pos: x:153 y:464
        Button* pExit = new Button(
            {0.11, 0.453}, 
            pExitMat, 
            [this]() {return ExitGame();}
        );
        pUILayout->addUIElement(pStart);
        pUILayout->addUIElement(pExit);
    }

    bool TitleScreen::StartNewGame()
    {
        SPACE_ENGINE_INFO("StartNewGame");
        //for debugging
        //We assuming that SpaceScene is early loaded but set not active by the app
        if(SceneManager::ActiveScene("SpaceScene", true))//Maybe is better to use another approach
        {
            SceneManager::SwitchScene("SpaceScene");//Maybe is better to use another approach
            return true;
        }
        return false;
    }

    bool TitleScreen::ExitGame()
    {
        exit(0);
        return true;
    }
}