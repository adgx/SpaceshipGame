#pragma once
#include "scene.h"
#include "ui.h"

namespace SpaceEngine {

    struct LeaderboardEntry {
        std::string name;
        int32_t score;
    };

    class GameOverScene : public Scene {
    public:
        GameOverScene(PhysicsManager* pPhyManager, SpaceScene* pGameScene);
        ~GameOverScene();

        void Init();
        
        // Questa funzione viene chiamata automaticamente da SceneManager 
        void OnSwitch() override; 

        void UpdateScene(float dt) override;

    private:
        bool IsTop3(int score);
        std::vector<LeaderboardEntry> LoadLeaderboard(const std::string& filename);
        void SaveLeaderboard(const std::string& filename, const std::vector<LeaderboardEntry>& entries);
        
        //inserimento nickname in caso di record
        void HandleTextInput();
        void SubmitScore();

        SpaceScene* m_pGameScene = nullptr;//per resettare il gioco

        UILayout* m_pLayout = nullptr;
        Text* m_pScoreText = nullptr;     
        Text* m_pNewRecordText = nullptr; 
        //ui per inserimento nickname
        Text* m_pInputLabel = nullptr; 
        Text* m_pNameDisplay = nullptr;
        Text* m_pSavedText = nullptr;

        UIBase* m_pNewRecordImg = nullptr;
        
        Button* m_pBtnNewGame = nullptr;
        Button* m_pBtnMenu = nullptr;

        bool m_isInputActive = false;
        std::string m_inputBuffer = "";//buffer per il nickname
        int m_finalScore = 0;
        
        //per evitare boncing tasti
        float m_keyTimer = 0.0f;
        float m_inputStartDelay = 0.0f; // delay prima dell'inserimento del nickname per sicurezza
        bool m_hasRecord = false;
    };
}