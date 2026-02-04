#pragma once
#include "scene.h"
#include "ui.h"
#include <vector>
#include <string>

namespace SpaceEngine {

    struct LeaderboardEntryLB {
        std::string name;
        int score;
    };

    class LeaderboardScene : public Scene {
    public:
        LeaderboardScene(PhysicsManager* pPhyManager, AudioManager* audioManager);
        ~LeaderboardScene();

        void Init();
        virtual void OnSwitch() override;

    private:
        void UpdateScene(float dt) override;
        void LoadAndDisplayScores();
        std::vector<LeaderboardEntryLB> ParseLeaderboard(const std::string& filename);

        UILayout* m_pLayout = nullptr;
        
        std::vector<Text*> m_rankTexts;
        std::vector<Text*> m_nameTexts;
        std::vector<Text*> m_scoreTexts;

        int m_lastWidth = 0;
        int m_lastHeight = 0;

        Button* m_pBtnMenu = nullptr;
    };
}