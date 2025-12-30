#include "titleScreen.h"
#include "inputManager.h"
#include "windowManager.h"
#include "log.h"

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

        UIButtonMaterial* pStartMat = MaterialManager::createMaterial<UIButtonMaterial>("StartButton");
        UIButtonMaterial* pExitMat = MaterialManager::createMaterial<UIButtonMaterial>("ExitButton");
        Texture* pTexStart = TextureManager::load(TEXTURES_PATH"buttons/NewGame.png");
        Texture* pTexExit = TextureManager::load(TEXTURES_PATH"buttons/Exit.png");
        //now We don't have the texture for the hover case
        pStartMat->addTexture("ui_tex", pTexStart);
        pStartMat->addTexture("ui_hover_tex", pTexStart);
        pExitMat->addTexture("ui_tex", pTexExit);
        pExitMat->addTexture("ui_hover_tex", pTexExit);
        //Creation of the UI button
        //x:153 y:330 | res x:1440 y:1024 | space between buttons 0.04
        Button* pStart = new Button({0.11, 0.32}, pStartMat); 
        Button* pExit = new Button({0.11, 0.36}, pExitMat);
        addSceneComponent(pStart);
        addSceneComponent(pExit);
        //TODO: do it after
        //Texture* ptexOptions    = TextureManager::load(TEXTURES_PATH"buttons/.png");
        //Texture* ptexLeaderboard= TextureManager::load(TEXTURES_PATH"buttons/.png");
        //Texture* ptexLogo       = TextureManager::load(TEXTURES_PATH"buttons/.png");

        //m_buttons.push_back({ posX, 200.0f, btnW, btnH, texStart,      TitleResult::PLAY,        false });
        //m_buttons.push_back({ posX, 270.0f, btnW, btnH, texOptions,    TitleResult::OPTIONS,     false });
        //m_buttons.push_back({ posX, 340.0f, btnW, btnH, texLeaderboard,TitleResult::LEADERBOARD, false });
        //m_buttons.push_back({ posX, 410.0f, btnW, btnH, texExit,       TitleResult::EXIT,        false });
        //SPACE_ENGINE_INFO("Bottoni caricati: {}", m_buttons.size())
    }

    /*
    TitleResult TitleScreen::getInput(){
        SPACE_ENGINE_DEBUG("Managing input from user");
        int mx = Mouse::x; 
        int my = Mouse::y;

        for (auto& btn : m_buttons) {
        
        bool isInsideX = (mx >= btn.x && mx <= (btn.x + btn.w));
        bool isInsideY = (my >= btn.y && my <= (btn.y + btn.h));

        if (isInsideX && isInsideY) {
            btn.isHovered = true; //se vogliamo illuminare il bottone al passaggio del mouse
            if(Mouse::buttonDown(0)){
                switch (btn.action) {
                    case TitleResult::PLAY:
                        SPACE_ENGINE_DEBUG("New Game starting...");
                        return TitleResult::PLAY;
                        break;

                    case TitleResult::OPTIONS:
                        SPACE_ENGINE_DEBUG("Opening options");
                        return TitleResult::OPTIONS;
                        break;

                    case TitleResult::LEADERBOARD:
                        SPACE_ENGINE_DEBUG("Opening Leaderboard");
                        return TitleResult::LEADERBOARD;
                        break;

                    case TitleResult::EXIT:
                        SPACE_ENGINE_DEBUG("Quitting Game");
                        return TitleResult::EXIT;
                        break;
                        
                    default:
                        break;
                }
            }    
        }else {
            btn.isHovered = false;
        }
        return TitleResult::NONE;
    }
    */
}