#pragma once
#include "ui.h"
#include "audioManager.h"

namespace SpaceEngine {

    class SpaceScene; 

    class PauseScene {
    public:
        PauseScene(SpaceScene* owner);
        ~PauseScene();

        void Init();
        void Update();
        
        void Show();
        void Hide();
        
        bool isVisible() const { return m_visible; }

    private:
        SpaceScene* m_pOwner = nullptr;
        UILayout* m_pLayout = nullptr;
        bool m_visible = false;
        Background* m_pBg = nullptr;
        Text* m_pScoreText = nullptr;
        
        Background* m_pVolBg = nullptr;
        Background* m_pVolFill = nullptr;
        Button* m_pKnob = nullptr;
        float m_currentVolume = 1.0f;

        void UpdateVolumeSlider();
    };
}