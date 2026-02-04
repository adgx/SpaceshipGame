#include "gameOverScene.h"
#include "sceneManager.h"
#include "windowManager.h"
#include "inputManager.h"
#include "scene.h"
#include "app.h" 
#include "scene.h" 
#include "material.h"
#include <fstream>
#include <algorithm> 
#include <sstream>

namespace SpaceEngine {

    GameOverScene::GameOverScene(PhysicsManager* pPhyManager, SpaceScene* pGameScene) 
        : Scene(pPhyManager), m_pGameScene(pGameScene) {
        Init();
    }

    GameOverScene::~GameOverScene() {}

    void GameOverScene::Init() {
        name = "GameOverScene";
        m_pLayout = new UILayout();
        addSceneComponent(m_pLayout);
        m_pLayout->addComponent<UINavigator>();

        UIMaterial* pBgMat = MaterialManager::createMaterial<UIMaterial>("GameOverBgMat");
        Texture* pTexBg = TextureManager::load(TEXTURES_PATH"backgrounds/GameOver.png"); 
        pBgMat->addTexture("ui_tex", pTexBg);
        Background* pBg = new Background(pBgMat);
        pBg->pUITransf->setAnchor({0.5f, 0.5f});
        pBg->pUITransf->setPos({0.f, 0.f});
        pBg->pUITransf->setWidth(1920.f); 
        pBg->pUITransf->setHeight(1080.f);
        m_pLayout->addUIElement(pBg);

        TextMaterial* pFontMat = MaterialManager::createMaterial<TextMaterial>("GameOverFont", "Orbitron-Regular");

        //score
        m_pScoreText = new Text({0.5f, 0.5f}, {90.f, -145.f}, {0.9f, 0.9f}, pFontMat);
        m_pScoreText->setString("0"); 
        m_pScoreText->setColor({1.f, 1.f, 1.f}); 
        m_pLayout->addText(m_pScoreText);

        //new recordo solo se in top 3
        UIMaterial* pRecordMat = MaterialManager::createMaterial<UIMaterial>("NewRecordMat");
        Texture* pTexRecord = TextureManager::load(TEXTURES_PATH"backgrounds/top_record.png"); 
        pRecordMat->addTexture("ui_tex", pTexRecord);
        m_pNewRecordImg = new UIBase(pRecordMat);
        m_pNewRecordImg->pUITransf->setAnchor({0.5f, 0.5f});
        m_pNewRecordImg->pUITransf->setPos({-200.f, -80.f});
        m_pNewRecordImg->setActive(false);
        m_pNewRecordImg->pUITransf->setWidth(400.f);
        m_pNewRecordImg->pUITransf->setHeight(40.f);
        m_pLayout->addUIElement(m_pNewRecordImg);

        m_pInputLabel = new Text({0.5f, 0.5f}, {-120.f, -20.f}, {0.7f, 0.7f}, pFontMat); 
        m_pInputLabel->setString("ENTER NAME:");
        m_pInputLabel->setColor({0.8f, 0.8f, 0.8f});
        m_pInputLabel->setActive(false);
        m_pLayout->addText(m_pInputLabel);

        m_pNameDisplay = new Text({0.5f, 0.5f}, {-110.f, 30.f}, {1.0f, 1.0f}, pFontMat);
        m_pNameDisplay->setString("PLAYER_");
        m_pNameDisplay->setColor({0.f, 1.f, 0.f});
        m_pNameDisplay->setActive(false);
        m_pLayout->addText(m_pNameDisplay);

        float btnX = 0.f; 
        float startY = 120.f;
        float spacing = 100.f;

        //bottone wnew game
        UIButtonMaterial* pNewGameMat = MaterialManager::createMaterial<UIButtonMaterial>("BtnNewGame");
        Texture* pTexNewGame = TextureManager::load(TEXTURES_PATH"buttons/NewGame.png");
        pNewGameMat->addTexture("ui_tex", pTexNewGame);
        m_pBtnNewGame = new Button({0.5f, 0.5f}, {-160.f, 70.f}, pNewGameMat, 
            [this]() { 
                if(!m_pBtnNewGame->isActive()) return false;
                App::state = EAppState::RUN;
                if(m_pGameScene) m_pGameScene->ResetGame();
                SceneManager::SwitchScene("SpaceScene");
                return true; 
            });
        m_pLayout->addUIElement(m_pBtnNewGame);

        //button main menu
        UIButtonMaterial* pMenuMat = MaterialManager::createMaterial<UIButtonMaterial>("BtnGOMenu");
        Texture* pTexMenu = TextureManager::load(TEXTURES_PATH"buttons/MainMenu.png");
        pMenuMat->addTexture("ui_tex", pTexMenu);
        m_pBtnMenu = new Button({0.5f, 0.5f}, {-160.f, 220.f}, pMenuMat, 
            [this]() { 
                if(!m_pBtnMenu->isActive()) return false;
                App::state = EAppState::START;
                if(m_pGameScene) m_pGameScene->ResetGame();
                Mouse::showCursor();
                SceneManager::SwitchScene("TitleScreen");
                return true; 
            });
        m_pLayout->addUIElement(m_pBtnMenu);

        m_pSavedText = new Text({0.5f, 0.5f}, {-180.f, -20.f}, {0.9f, 0.9f}, pFontMat);
        m_pSavedText->setString("SCORE SAVED!");
        m_pSavedText->setColor({0.f, 1.f, 0.f}); // Verde
        m_pSavedText->setActive(false); // Parte nascosto
        m_pLayout->addText(m_pSavedText);
    }

    void GameOverScene::OnSwitch() {
        Mouse::showCursor();

        m_finalScore = 0;
        if (SpaceScene::pScoreSys) {
            m_finalScore = SpaceScene::pScoreSys->GetScore();
        }

        if (m_pScoreText) {
            m_pScoreText->setString(std::to_string(m_finalScore));
        }

        m_hasRecord = IsTop3(m_finalScore);
        if (m_hasRecord) {
            m_inputBuffer = ""; //reset buffer
            m_keyTimer = 0.0f;
            
            //nasconde i bottoni
            m_pBtnNewGame->setActive(false);
            m_pBtnMenu->setActive(false);
            
            if(m_pNewRecordImg) m_pNewRecordImg->setActive(true);
            m_keyTimer = 0.0f;
            m_pInputLabel->setActive(true);
            m_pNameDisplay->setActive(true);
            m_pNameDisplay->setString("_");
            m_pSavedText->setActive(false);

            m_isInputActive = false; //input BLOCCATO inizialmente
            m_inputStartDelay = 1.0f;
        } 
        else {
            m_isInputActive = false;
            m_keyTimer = 0.0f;
            m_hasRecord = false;
            m_inputStartDelay = 0.0f;
            
            m_pBtnNewGame->setActive(true);
            m_pBtnMenu->setActive(true);
            
            if(m_pNewRecordImg) m_pNewRecordImg->setActive(false);
            m_pInputLabel->setActive(false);
            m_pNameDisplay->setActive(false);
            m_pSavedText->setActive(false);
        }
    }

    void GameOverScene::UpdateScene(float dt) {
        if (m_inputStartDelay > 0.0f) {
            m_inputStartDelay -= dt;
            if (m_inputStartDelay <= 0.0f) {
                if (m_hasRecord) {
                    m_isInputActive = true;
                }
            }
            return; 
        }

        if (m_isInputActive) {
            if (m_keyTimer > 0.0f) m_keyTimer -= dt;
            else HandleTextInput();
        }
        else{
            if (m_keyTimer > 0.0f) {
                m_keyTimer -= dt;

                if (m_keyTimer <= 0.0f) {
                    m_pBtnNewGame->setActive(true);
                    m_pBtnMenu->setActive(true);
                    
                    m_keyTimer = 0.0f;
                }
            }
        }
    }

    void GameOverScene::HandleTextInput() {
        bool keyPressed = false;

        if (Keyboard::keyDown(SPACE_ENGINE_KEY_BUTTON_ENTER)) {
            if (m_inputBuffer.length() > 0) {
                SubmitScore();
                return;
            }
        }

        if (Keyboard::keyDown(SPACE_ENGINE_KEY_BUTTON_BACKSPACE)) {
            if (m_inputBuffer.length() > 0) {
                m_inputBuffer.pop_back();
                keyPressed = true;
            }
        }

        for (int key = SPACE_ENGINE_KEY_BUTTON_A; key <= SPACE_ENGINE_KEY_BUTTON_Z; ++key) {
            if (Keyboard::keyDown(key)) {
                if (m_inputBuffer.length() < 10) { 
                    m_inputBuffer += (char)key;
                    keyPressed = true;
                }
            }
        }

        for (int key = SPACE_ENGINE_KEY_BUTTON_0; key <= SPACE_ENGINE_KEY_BUTTON_9; ++key) {
            if (Keyboard::keyDown(key)) {
                if (m_inputBuffer.length() < 10) { 
                    m_inputBuffer += (char)key;
                    keyPressed = true;
                }
            }
        }
        
        if (keyPressed) {
            m_pNameDisplay->setString(m_inputBuffer + "_");
            m_keyTimer = 0.20f;//per evitare bouncing
        }
    }

    void GameOverScene::SubmitScore() {
        m_isInputActive = false;
        
        std::string filename = "assets/leaderboard.json";
        std::vector<LeaderboardEntry> entries = LoadLeaderboard(filename);

        entries.push_back({m_inputBuffer, m_finalScore});

        std::sort(entries.begin(), entries.end(), [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
            return a.score > b.score; 
        });

        if (entries.size() > 3) entries.resize(3);

        SaveLeaderboard(filename, entries);

        m_pInputLabel->setActive(false);
        m_pNameDisplay->setActive(false);
        
        if(m_pNewRecordImg) m_pNewRecordImg->setActive(false);
        if(m_pSavedText) m_pSavedText->setActive(true);
        m_keyTimer = 0.5f;
    }

    bool GameOverScene::IsTop3(int score) {
        if (score == 0) return false;

        std::vector<LeaderboardEntry> entries = LoadLeaderboard("assets/leaderboard.json");
        
        if (entries.size() < 3) return true;

        if (score > entries.back().score) return true;

        return false;
    }

    std::vector<LeaderboardEntry> GameOverScene::LoadLeaderboard(const std::string& filename) {
        std::vector<LeaderboardEntry> entries;
        std::ifstream file(filename);
        if (!file.is_open()) return entries;

        std::string line;
        while (std::getline(file, line)) {
            size_t namePos = line.find("\"name\": \"");
            size_t scorePos = line.find("\"score\": ");

            if (namePos != std::string::npos && scorePos != std::string::npos) {
                LeaderboardEntry entry;
                size_t startQuote = namePos + 9; 
                size_t endQuote = line.find("\"", startQuote);
                entry.name = line.substr(startQuote, endQuote - startQuote);

                size_t startScore = scorePos + 9; 
                std::string scoreStr = line.substr(startScore);
                size_t endNum = scoreStr.find_first_not_of("0123456789");
                if (endNum != std::string::npos) scoreStr = scoreStr.substr(0, endNum);
                
                try {
                    entry.score = std::stoi(scoreStr);
                    entries.push_back(entry);
                } catch (...) {}
            }
        }
        file.close();
        
        std::sort(entries.begin(), entries.end(), [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
            return a.score > b.score; 
        });

        return entries;
    }

    void GameOverScene::SaveLeaderboard(const std::string& filename, const std::vector<LeaderboardEntry>& entries) {
        std::ofstream file(filename);
        if (file.is_open()) {
            file << "[\n";
            for (size_t i = 0; i < entries.size(); ++i) {
                file << "  { \"name\": \"" << entries[i].name << "\", \"score\": " << entries[i].score << " }";
                if (i < entries.size() - 1) file << ","; 
                file << "\n";
            }
            file << "]\n";
            file.close();
            SPACE_ENGINE_INFO("Leaderboard saved successfully to: {}", filename);
        } else {
            SPACE_ENGINE_ERROR("FAILED to save leaderboard! Could not open file: {}", filename);
            SPACE_ENGINE_ERROR("Make sure the 'assets' folder exists in the executable directory.");
        }
    }
}