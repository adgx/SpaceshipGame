#include "leaderboardScene.h"
#include "sceneManager.h"
#include "windowManager.h"
#include "material.h"
#include "inputManager.h"
#include "utils/utils.h" 
#include "app.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace SpaceEngine {

    LeaderboardScene::LeaderboardScene(PhysicsManager* pPhyManager, AudioManager* audioManager) 
        : Scene(pPhyManager) {
        setAudioManager(audioManager);
        Init();
    }

    LeaderboardScene::~LeaderboardScene() {}

    void LeaderboardScene::Init() {
        name = "LeaderboardScene";
        m_lastWidth = WindowManager::width;
        m_lastHeight = WindowManager::height;
        m_pLayout = new UILayout();
        addSceneComponent(m_pLayout);
        m_pLayout->addComponent<UINavigator>();

        UIMaterial* pBgMat = MaterialManager::createMaterial<UIMaterial>("LeaderboardBgMat");
        Texture* pTexBg = TextureManager::load(TEXTURES_PATH"backgrounds/LeaderboardBG.png");
        pBgMat->addTexture("ui_tex", pTexBg);
        
        Background* pBg = new Background(pBgMat);
        pBg->pUITransf->setAnchor({0.5f, 0.5f});
        pBg->pUITransf->setPos({0.f, 0.f});
        pBg->pUITransf->setWidth(static_cast<float>(WindowManager::width)); 
        pBg->pUITransf->setHeight(static_cast<float>(WindowManager::height));
        m_pLayout->addUIElement(pBg);

        TextMaterial* pFontMat = MaterialManager::createMaterial<TextMaterial>("LeaderboardFont", "Orbitron-Regular");

        float startY = -100.0f; 
        float gapY = 100.0f;
        float nameX = -200.0f;
        float scoreX = 250.0f;
        float rankX = -400.0f;

        for(int i = 0; i < 3; i++) {
            float yPos = startY + (i * gapY);

            Text* tRank = new Text({0.5f, 0.5f}, {rankX, yPos}, {0.9f, 0.9f}, pFontMat);
            tRank->setString("#" + std::to_string(i + 1)); // Scrive #1, #2, #3
            tRank->setColor({1.f, 1.f, 1.f}); // Bianco
            m_pLayout->addText(tRank);
            m_rankTexts.push_back(tRank);

            Text* tName = new Text({0.5f, 0.5f}, {nameX, yPos}, {0.8f, 0.8f}, pFontMat);
            tName->setString("---");
            m_pLayout->addText(tName);
            m_nameTexts.push_back(tName);

            Text* tScore = new Text({0.5f, 0.5f}, {scoreX, yPos}, {0.8f, 0.8f}, pFontMat);
            tScore->setString("0");
            m_pLayout->addText(tScore);
            m_scoreTexts.push_back(tScore);
        }

        UIButtonMaterial* pMenuMat = MaterialManager::createMaterial<UIButtonMaterial>("BtnLB_Menu");
        Texture* pTexMenu = TextureManager::load(TEXTURES_PATH"buttons/MainMenu.png");
        pMenuMat->addTexture("ui_tex", pTexMenu);

        m_pBtnMenu = new Button({0.5f, 0.5f}, {-200.f, 300.f}, pMenuMat, 
            [this]() { 
                SceneManager::SwitchScene("TitleScreen");
                return true; 
            });
        m_pLayout->addUIElement(m_pBtnMenu);
    }

    void LeaderboardScene::OnSwitch() {
        m_lastWidth = WindowManager::width;
        m_lastHeight = WindowManager::height;
        notifyChangeRes();
        Mouse::showCursor();
        LoadAndDisplayScores();
    }

    void LeaderboardScene::UpdateScene(float dt) {
        if (WindowManager::width != m_lastWidth || WindowManager::height != m_lastHeight)
        {
            m_lastWidth = WindowManager::width;
            m_lastHeight = WindowManager::height;
            notifyChangeRes();
        }
    }

    void LeaderboardScene::LoadAndDisplayScores() {
        std::string filename = TEXTURES_PATH + std::string("../leaderboard.json"); 
        std::vector<LeaderboardEntryLB> entries = ParseLeaderboard(filename);

        for(auto t : m_nameTexts) t->setString("---");
        for(auto t : m_scoreTexts) t->setString("...");

        for(size_t i = 0; i < entries.size() && i < 3; i++) {
            m_nameTexts[i]->setString(entries[i].name);
            m_scoreTexts[i]->setString(std::to_string(entries[i].score));
        }
    }

    std::vector<LeaderboardEntryLB> LeaderboardScene::ParseLeaderboard(const std::string& filename) {
        std::vector<LeaderboardEntryLB> entries;
        std::ifstream file(filename);
        if (!file.is_open()) return entries;

        std::string line;
        while (std::getline(file, line)) {
            size_t namePos = line.find("\"name\": \"");
            size_t scorePos = line.find("\"score\": ");

            if (namePos != std::string::npos && scorePos != std::string::npos) {
                LeaderboardEntryLB entry;
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
        
        // Ordina decrescente
        std::sort(entries.begin(), entries.end(), [](const LeaderboardEntryLB& a, const LeaderboardEntryLB& b) {
            return a.score > b.score; 
        });

        return entries;
    }
}