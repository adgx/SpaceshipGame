#pragma once
#include "scene.h"

namespace SpaceEngine {
    class SettingsScene : public Scene {
    public:
        SettingsScene(PhysicsManager* pPhyManager);
        ~SettingsScene();
    private:
        void Init();
        bool backToTitle();
        bool goFullscreen(); 
        bool IncreaseVolume();
        bool DecreaseVolume();
        void UpdateVolumeBar();

        float m_currentVolume = 1.0f;
        //per ridimensionare la barra del volume
        Background* m_pVolumeFill = nullptr;
        const float MAX_BAR_WIDTH = 300.0f; //laarghezza massima in pixel della barra piena
    };
}