#include "pauseScene.h"
#include "sceneManager.h"
#include "inputManager.h"
#include "windowManager.h"
#include "scene.h" 
#include "app.h"

namespace SpaceEngine {
    PauseScene::PauseScene(SpaceScene* owner) : m_pOwner(owner) {
        Init();
    }

    PauseScene::~PauseScene() {
        if (m_pLayout) delete m_pLayout;
    }

    void PauseScene::Init() {
        m_pLayout = new UILayout();

        //sfondo
        UIMaterial* pBgMat = MaterialManager::createMaterial<UIMaterial>("PauseBgMat");
        Texture* pTexBg = TextureManager::load(TEXTURES_PATH"backgrounds/Game_Paused.png"); 
        pBgMat->addTexture("ui_tex", pTexBg);

        m_pBg = new Background(pBgMat);
        m_pBg->pUITransf->setAnchor({0.5f, 0.5f});
        m_pBg->pUITransf->setPos({0.f, 0.f});
        m_pBg->pUITransf->setWidth(1920.f); 
        m_pBg->pUITransf->setHeight(1080.f);
        m_pLayout->addUIElement(m_pBg);

        // score
        TextMaterial* pScoreMat = MaterialManager::createMaterial<TextMaterial>("PauseScoreValFont", "Orbitron-Regular");
        m_pScoreText = new Text({0.5f, 0.5f}, {0.f, -195.f}, {0.8f, 0.8f}, pScoreMat);
        m_pScoreText->setString("00000");
        m_pLayout->addText(m_pScoreText);

        float btnX = -160.f; 
        float firstBtnY = -80.f; 
        float btnSpacing = 150.f;

        // bottone continue
        UIButtonMaterial* pContMat = MaterialManager::createMaterial<UIButtonMaterial>("BtnContinue");
        Texture* pTexCont = TextureManager::load(TEXTURES_PATH"buttons/Continue.png"); 
        pContMat->addTexture("ui_tex", pTexCont);
        Button* pBtnContinue = new Button({0.5f, 0.5f}, {btnX, firstBtnY}, pContMat, 
            [this]() {
                if(m_pOwner) m_pOwner->TogglePause(); 
                return true; 
            });
        m_pLayout->addUIElement(pBtnContinue);

        // bottone main menu
        UIButtonMaterial* pMenuMat = MaterialManager::createMaterial<UIButtonMaterial>("BtnMainMenu");
        Texture* pTexMenu = TextureManager::load(TEXTURES_PATH"buttons/MainMenu.png"); 
        pMenuMat->addTexture("ui_tex", pTexMenu);
        Button* pBtnMenu = new Button({0.5f, 0.5f}, {btnX, firstBtnY + btnSpacing}, pMenuMat, 
            [this]() { 
                if(m_pOwner) {
                    m_pOwner->ResetGame(); 
                    m_pOwner->TogglePause(); 
                }
                Mouse::showCursor();
                App::state = EAppState::START; 
                SceneManager::SwitchScene("TitleScreen"); 
                return true; });
        m_pLayout->addUIElement(pBtnMenu);

        // volume
        float sliderX = -140.f;
        float sliderY = 295.f;
        float barWidth = 280.f;

        UIMaterial* pVolBgMat = MaterialManager::createMaterial<UIMaterial>("PauseVolBg");
        Texture* pTexVolBg = TextureManager::load(TEXTURES_PATH"buttons/grey_bar.png");
        pVolBgMat->addTexture("ui_tex", pTexVolBg);
        pVolBgMat->setColor({0.2f, 0.2f, 0.2f, 1.0f});
        m_pVolBg = new Background(pVolBgMat);
        m_pVolBg->pUITransf->setAnchor({0.5f, 0.5f});
        m_pVolBg->pUITransf->setPos({sliderX, sliderY});
        m_pVolBg->pUITransf->setFill(false);
        m_pVolBg->pUITransf->setWidth(barWidth);
        m_pVolBg->pUITransf->setHeight(17.f);
        m_pLayout->addUIElement(m_pVolBg);

        UIMaterial* pVolFillMat = MaterialManager::createMaterial<UIMaterial>("PauseVolFill");
        Texture* pTexVolFill = TextureManager::load(TEXTURES_PATH"buttons/white_bar.png");
        pVolFillMat->addTexture("ui_tex", pTexVolFill);
        m_pVolFill = new Background(pVolFillMat);
        m_pVolFill->pUITransf->setAnchor({0.5f, 0.5f});
        m_pVolFill->pUITransf->setPos({sliderX - (barWidth/2.f), sliderY});
        m_pVolFill->pUITransf->setFill(false);
        m_pVolFill->pUITransf->setWidth(barWidth); 
        m_pVolFill->pUITransf->setHeight(17.f);
        m_pLayout->addUIElement(m_pVolFill);

        //pallino
        UIButtonMaterial* pKnobMat = MaterialManager::createMaterial<UIButtonMaterial>("PauseKnob");
        Texture* pTexKnob = TextureManager::load(TEXTURES_PATH"buttons/volume_knob.png");
        pKnobMat->addTexture("ui_tex", pTexKnob);
        m_pKnob = new Button({0.5f, 0.5f}, {sliderX, sliderY}, pKnobMat, [](){return true;});
        m_pKnob->pUITransf->setWidth(30.f);
        m_pKnob->pUITransf->setHeight(30.f);
        m_pLayout->addUIElement(m_pKnob);
    }

    void PauseScene::Show() {
        if (m_visible) return;
        m_visible = true;
        
        Mouse::showCursor();

        if (m_pOwner) {
            if (auto am = m_pOwner->getAudioManager()) m_currentVolume = am->GetVolume();

            uint32_t currentScore = m_pOwner->GetCurrentScore();
            if(m_pScoreText) {
                m_pScoreText->setString(std::to_string(currentScore));
            }

            m_pOwner->addSceneComponent(m_pLayout);
        }
        UpdateVolumeSlider();
    }

    void PauseScene::Hide() {
        if (!m_visible) return;
        m_visible = false;
        
        Mouse::hideCursor();
        if (m_pOwner) {
            m_pOwner->removePauseLayout(m_pLayout);
        }
    }

    void PauseScene::Update() {
        if (!m_visible) return;

        //logica mouse per volume
        if (Mouse::button(SPACE_ENGINE_MOUSE_BUTTON_LEFT)) {
            float mx = static_cast<float>(Mouse::getPosX());
            float my = static_cast<float>(Mouse::getPosY());

            if (m_pVolBg) {
                Rect* rect = m_pVolBg->pUITransf->getRect();
                float marginY = 30.0f;
                if (mx >= rect->pos.x && mx <= rect->pos.x + rect->size.x &&
                    my >= rect->pos.y - marginY && my <= rect->pos.y + rect->size.y + marginY) 
                {
                    float relativeX = mx - rect->pos.x;
                    float newVol = relativeX / rect->size.x;
                    if (newVol < 0.0f) newVol = 0.0f;
                    if (newVol > 1.0f) newVol = 1.0f;
                    
                    m_currentVolume = newVol;
                    if(m_pOwner) {
                        if(auto am = m_pOwner->getAudioManager()) am->SetVolume(m_currentVolume);
                    }
                }
            }
        }
        UpdateVolumeSlider();
    }

    void PauseScene::UpdateVolumeSlider() {
        float sliderX = -140.f; 
        float sliderY = 295.f; 
        float maxBarWidth = 280.f;
        
        float currentWidth = maxBarWidth * m_currentVolume;

        if(m_pVolFill) {
            m_pVolFill->pUITransf->setWidth(currentWidth);
            m_pVolFill->pUITransf->setPos({sliderX, sliderY});
        }
        if(m_pKnob) {
            float knobWidth = 30.0f;
            float knobX = sliderX + currentWidth - (knobWidth / 2.0f);
            m_pKnob->pUITransf->setPos({knobX, sliderY+ -5.5f});
        }
    }
}