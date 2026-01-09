#include "settingsScene.h"
#include "sceneManager.h"
#include "ui.h"
#include "windowManager.h"

namespace SpaceEngine {

    SettingsScene::SettingsScene(PhysicsManager* pPhyManager) : Scene(pPhyManager) {
        Init();
        SPACE_ENGINE_DEBUG("Settings Screen opening...");
    }

    SettingsScene::~SettingsScene() {
        SPACE_ENGINE_DEBUG("Going from Settings to main menu...");
    }

    void SettingsScene::Init() {
        name = "SettingsScene"; 

        UILayout* pUILayout = new UILayout();
        addSceneComponent(pUILayout);

        UIMaterial* pBackgroundMat = MaterialManager::createMaterial<UIMaterial>("BGSettings");
        Texture* pTexBGTitleMenu = TextureManager::load(TEXTURES_PATH"backgrounds/SettingsBG.png"); 
        pBackgroundMat->addTexture("ui_tex", pTexBGTitleMenu);
        pUILayout->addUIElement(new Background(pBackgroundMat));

        pUILayout->addComponent<UINavigator>();

        //pulsante Fullscreen
        UIButtonMaterial* pFullMat = MaterialManager::createMaterial<UIButtonMaterial>("BtnFullscreen");
        Texture* pTexFull = TextureManager::load(TEXTURES_PATH"buttons/Fullscreen.png");
        pFullMat->addTexture("ui_tex", pTexFull);

        Button* pBtnFull = new Button({0.f, 0.f}, {153.f, 330.f}, pFullMat, 
            [this]() { return goFullscreen(); });

        //pulsante volume
        UIButtonMaterial* pVolumeMat = MaterialManager::createMaterial<UIButtonMaterial>("BtnFullscreen");
        Texture* pTexVol = TextureManager::load(TEXTURES_PATH"buttons/Fullscreen.png");
        pVolumeMat->addTexture("ui_tex", pTexVol);

        Background* pVolBarBg = new Background(pVolumeMat);
        pVolBarBg->pUITransf->setAnchor({0.5f, 0.5f}); 
        pVolBarBg->pUITransf->setPos({-150.f, 0.f});
        pVolBarBg->pUITransf->setFill(false);
        pVolBarBg->pUITransf->setWidth(MAX_BAR_WIDTH);
        pVolBarBg->pUITransf->setHeight(30.f);
        pUILayout->addUIElement(pVolBarBg);

        // Riempimento della barra (parte piena/verde)
        UIMaterial* pBarFillMat = MaterialManager::createMaterial<UIMaterial>("VolBarFill");
        Texture* pTexBarFill = TextureManager::load(TEXTURES_PATH"ui/BarFill.png"); // Un rettangolo verde/bianco
        pBarFillMat->addTexture("ui_tex", pTexBarFill);
        
        m_pVolumeFill = new Background(pBarFillMat);
        m_pVolumeFill->pUITransf->setAnchor({0.5f, 0.5f});
        m_pVolumeFill->pUITransf->setPos({-150.f, 0.f});
        m_pVolumeFill->pUITransf->setFill(false);
        m_pVolumeFill->pUITransf->setWidth(MAX_BAR_WIDTH); 
        m_pVolumeFill->pUITransf->setHeight(30.f);
        pUILayout->addUIElement(m_pVolumeFill);

        // button -
        UIButtonMaterial* pMinusMat = MaterialManager::createMaterial<UIButtonMaterial>("BtnMinus");
        Texture* pTexMinus = TextureManager::load(TEXTURES_PATH"buttons/Minus.png");
        pMinusMat->addTexture("ui_tex", pTexMinus);
        
        Button* pBtnMinus = new Button({0.5f, 0.5f}, {-200.f, -10.f}, pMinusMat,
            [this]() { return DecreaseVolume(); });
        pUILayout->addUIElement(pBtnMinus);

        // button +
        UIButtonMaterial* pPlusMat = MaterialManager::createMaterial<UIButtonMaterial>("BtnPlus");
        Texture* pTexPlus = TextureManager::load(TEXTURES_PATH"buttons/Plus.png");
        pPlusMat->addTexture("ui_tex", pTexPlus);
        
        Button* pBtnPlus = new Button({0.5f, 0.5f}, {160.f, -10.f}, pPlusMat, 
            [this]() { return IncreaseVolume(); });
        pUILayout->addUIElement(pBtnPlus);
        
        // pulsante back
        UIButtonMaterial* pBackMat = MaterialManager::createMaterial<UIButtonMaterial>("BtnBack");
        Texture* pTexBack = TextureManager::load(TEXTURES_PATH"buttons/Back.png");
        pBackMat->addTexture("ui_tex", pTexBack);

        Button* pBtnBack = new Button({0.f, 0.f}, {153.f, 464.f}, pBackMat, 
            [this]() { return backToTitle(); });

        pUILayout->addUIElement(pBtnFull);
        pUILayout->addUIElement(pBtnBack);
        pUILayout->addUIElement(pBtnMinus);
        pUILayout->addUIElement(pBtnPlus);
    }

    bool SettingsScene::goFullscreen() {
        SPACE_ENGINE_INFO("Toggle Fullscreen clicked");
        WindowManager::ToggleFullScreen(); 
        return true;
    }

    bool SettingsScene::IncreaseVolume()
    {
        m_currentVolume += 0.1f;
        if(m_currentVolume > 1.0f) m_currentVolume = 1.0f;
        
        UpdateVolumeBar();
        return true; 
    }

    bool SettingsScene::DecreaseVolume()
    {
        m_currentVolume -= 0.1f;
        if(m_currentVolume < 0.0f) m_currentVolume = 0.0f;
        
        UpdateVolumeBar();
        return true;
    }

    void SettingsScene::UpdateVolumeBar()
    {
        if(auto am = getAudioManager()) {
            am->SetVolume(m_currentVolume);
        }

        // aggiorna la largezza della barra del volume
        if(m_pVolumeFill) {
            float newWidth = MAX_BAR_WIDTH * m_currentVolume;
            m_pVolumeFill->pUITransf->setWidth(newWidth);
        }
    }
    
    bool SettingsScene::backToTitle() {
        SceneManager::SwitchScene("TitleScreen");
        return true;
    }
}