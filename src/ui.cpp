#include <algorithm>

#include "ui.h"
#include "app.h"
#include "renderer.h"
#include "shader.h"
#include "managers/windowManager.h"

namespace SpaceEngine
{
    //---------------------------------//
    //-------------UIBase--------------//
    //---------------------------------//
    UIBase::UIBase()
    {
        pUIMeshRend = new UIMeshRenderer();
        pUITransf = new UITransform();
    }

    UIBase::UIBase(UIMaterial* pUIMaterial):UIBase()
    {
        pUIMeshRend->bindMaterial(pUIMaterial);
        
        if(!pUIMaterial->pShader)
            pUIMaterial->pShader = ShaderManager::findShaderProgram("ui");
        
        Texture* pTex = pUIMaterial->getTexture("ui_tex");
        int w = 0;
        int h = 0;
        pTex->getImageSize(w, h);
        pUITransf->setWidth(static_cast<float>(w));
        pUITransf->setHeight(static_cast<float>(h));
        pUITransf->setPos({0.f, 0.f});
    }

    UIBase::UIBase(Vector2 posAncor, Vector2 pos, UIMaterial* pUIMaterial)
    {
        pUIMeshRend = new UIMeshRenderer();
        pUIMeshRend->bindMaterial(pUIMaterial);

        if(!pUIMaterial->pShader)
            pUIMaterial->pShader = ShaderManager::findShaderProgram("ui");
        
        pUITransf = new UITransform();
        Texture* pTex = pUIMaterial->getTexture("ui_tex");
        int w = 0;
        int h = 0;
        pUITransf->setAnchor(posAncor);
        pTex->getImageSize(w, h);
        pUITransf->setWidth(static_cast<float>(w));
        pUITransf->setHeight(static_cast<float>(h));
        pUITransf->setPos(pos);
    }

    UIBase::~UIBase()
    {
        delete pUIMeshRend;
        delete pUITransf;
    }

    //---------------------------------//
    //-----------UITrasform------------//
    //---------------------------------//
    
    Rect* UITransform::getRect()
    {
        if((dirty))
        {
            if(!fill)
            {
                float scale = 1.f;
                Vector2 offset = {0.f, 0.f};
                Vector2 outPos = {0.f, 0.f};

                Utils::applyRatioScreenRes(anchor, pos, scale, offset, outPos);                
                //space postion
                float x = outPos.x;
                float y = outPos.y;
                //size valuation
                float w = size.x * scale;
                float h = size.y * scale;
                
                rectCached = Rect(x, y, w, h);
            }
            else rectCached = Rect(0.f, 
                0.f, 
                static_cast<float>(WindowManager::width), 
                static_cast<float>(WindowManager::height));
            dirty = false;
        }

        return &rectCached;
    }

    void UITransform::setAnchor(Vector2 anchor)
    {
        this->anchor.x = anchor.x;
        this->anchor.y = anchor.y;
        dirty = true;
    }

    Vector2 UITransform::getAnchor()
    {
        return anchor;
    }

    void UITransform::setSize(Vector2 size)
    {
        if(size.x < 0.f || size.x > 1.f)
        {
            SPACE_ENGINE_ERROR("size x: out of rage 0-1, so the size is clamped");
            size.x = std::clamp(size.x, 0.f, 1.f);
        }
        
        if(size.y < 0.f || size.y > 1.f)
        {
            SPACE_ENGINE_ERROR("size x: out of rage 0-1, so the size is clamped");
            size.y = std::clamp(size.y, 0.f, 1.f);
        }

        this->size = size;
        dirty = true;
    }

    int UITransform::getX()
    {
        return static_cast<int>(anchor.x * WindowManager::width);
    }

    int UITransform::getY()
    {
        return static_cast<int>(anchor.y * WindowManager::height);
    }

    //---------------------------------//
    //-------------Button--------------//
    //---------------------------------//
    Button::Button(Vector2 anchor, Vector2 pos, UIButtonMaterial* pMat):UIBase(anchor, pos, pMat){}

    Button::Button(Vector2 anchor, Vector2 pos, UIButtonMaterial* pMat, std::function<bool()> func):Button(anchor, pos, pMat)
    {
        onClick = func; 
    }

    bool Button::update(int mx, int my)
    {
        hovered = Rect::pointInRect(*(pUITransf->getRect()), 
                                        static_cast<float>(mx),
                                        static_cast<float>(my));
        if(hovered)
        {
            UIButtonMaterial* pMat = dynamic_cast<UIButtonMaterial*>(pUIMeshRend->getMaterial());
            pMat->setSubroutineHover(true);
        }
        else
        {
            UIButtonMaterial* pMat = dynamic_cast<UIButtonMaterial*>(pUIMeshRend->getMaterial());
            pMat->setSubroutineHover(false);
        }

        return hovered;
    }

    bool Button::isHovered()
    {
        return hovered;
    }

    //---------------------------------//
    //-----------Background------------//
    //---------------------------------//

    Background::Background(UIMaterial* pUIMaterial):UIBase(pUIMaterial)
    {
        pUITransf->setFill(true);
    }

    //---------------------------------//
    //-----------UINavigator-----------//
    //---------------------------------//

    UINavMoveDownCommand* UINavigator::m_pMoveDownCmd = nullptr;
    UINavMoveUpCommand* UINavigator::m_pMoveUpCmd = nullptr;
    UINavOnClickCommand* UINavigator::m_pOnClickCmd = nullptr;
    UINavOnPressCommand* UINavigator::m_pOnPressCmd = nullptr;
    UINavMoveRightCommand* UINavigator::m_pMoveRightCmd = nullptr;
    UINavMoveLeftCommand* UINavigator::m_pMoveLeftCmd = nullptr;
    UINavigator* UINavigator::s_currentActiveNavigator = nullptr;
    
    int UINavigator::count = 0;

    UINavigator::UINavigator()
    {
        assert(count >= 0);

        if(!count)
        {
            m_pMoveDownCmd = new UINavMoveDownCommand();
            m_pMoveUpCmd = new UINavMoveUpCommand();
            m_pOnClickCmd = new UINavOnClickCommand();
            m_pOnPressCmd = new UINavOnPressCommand();
            m_pMoveRightCmd = new UINavMoveRightCommand();
            m_pMoveLeftCmd = new UINavMoveLeftCommand();
        }

        InputHandler& inputHandler = App::GetInputHandler();
        //Down command for joystick and keyboard
        /*inputHandler.bindCommand(EAppState::RUN, 
            this, 
            {SPACE_ENGINE_JK_BUTTON_DOWN, 
                EInputType::SPACE_ENGINE_INPUT_JOYSTICK, 
                m_pMoveDownCmd});
        inputHandler.bindCommand(EAppState::START, 
            this, 
            {SPACE_ENGINE_KEY_BUTTON_S, 
                EInputType::SPACE_ENGINE_INPUT_KEYBOARD, 
                m_pMoveDownCmd});
        
        //UP command for joystick and keyboard
        inputHandler.bindCommand(EAppState::START, 
            this, 
            {SPACE_ENGINE_JK_BUTTON_UP, 
                EInputType::SPACE_ENGINE_INPUT_JOYSTICK, 
                m_pMoveUpCmd});
        inputHandler.bindCommand(EAppState::START, 
            this, 
            {SPACE_ENGINE_KEY_BUTTON_W, 
                EInputType::SPACE_ENGINE_INPUT_KEYBOARD, 
                m_pMoveUpCmd});

        //Click command for joystick and keyboard
        inputHandler.bindCommand(EAppState::START, 
            this, 
            {SPACE_ENGINE_JK_BUTTON_A, 
                EInputType::SPACE_ENGINE_INPUT_JOYSTICK, 
                m_pOnPressCmd});
        inputHandler.bindCommand(EAppState::START, 
            this, 
            {SPACE_ENGINE_KEY_BUTTON_ENTER, 
                EInputType::SPACE_ENGINE_INPUT_KEYBOARD, 
                m_pOnPressCmd});
        //Click command for mouse
        inputHandler.bindCommand(EAppState::START, 
            this, 
            {SPACE_ENGINE_MOUSE_BUTTON_LEFT, 
                EInputType::SPACE_ENGINE_INPUT_MOUSE, 
                m_pOnClickCmd});*/
        count++;
    }

    void UINavigator::bindCommands()
    {
        InputHandler& inputHandler = App::GetInputHandler();
        // Down
        inputHandler.bindCommand(EAppState::START, this, {SPACE_ENGINE_JK_BUTTON_DOWN, EInputType::SPACE_ENGINE_INPUT_JOYSTICK, m_pMoveDownCmd});
        inputHandler.bindCommand(EAppState::START, this, {SPACE_ENGINE_KEY_BUTTON_S, EInputType::SPACE_ENGINE_INPUT_KEYBOARD, m_pMoveDownCmd});
        // Up
        inputHandler.bindCommand(EAppState::START, this, {SPACE_ENGINE_JK_BUTTON_UP, EInputType::SPACE_ENGINE_INPUT_JOYSTICK, m_pMoveUpCmd});
        inputHandler.bindCommand(EAppState::START, this, {SPACE_ENGINE_KEY_BUTTON_W, EInputType::SPACE_ENGINE_INPUT_KEYBOARD, m_pMoveUpCmd});
        // Click/Press
        inputHandler.bindCommand(EAppState::START, this, {SPACE_ENGINE_JK_BUTTON_A, EInputType::SPACE_ENGINE_INPUT_JOYSTICK, m_pOnPressCmd});
        inputHandler.bindCommand(EAppState::START, this, {SPACE_ENGINE_KEY_BUTTON_ENTER, EInputType::SPACE_ENGINE_INPUT_KEYBOARD, m_pOnPressCmd});
        // Mouse
        inputHandler.bindCommand(EAppState::START, this, {SPACE_ENGINE_MOUSE_BUTTON_LEFT, EInputType::SPACE_ENGINE_INPUT_MOUSE, m_pOnClickCmd});
        //destra
        inputHandler.bindCommand(EAppState::START, this, {SPACE_ENGINE_JK_BUTTON_RIGHT, EInputType::SPACE_ENGINE_INPUT_JOYSTICK, m_pMoveRightCmd});
        inputHandler.bindCommand(EAppState::START, this, {SPACE_ENGINE_KEY_BUTTON_D, EInputType::SPACE_ENGINE_INPUT_KEYBOARD, m_pMoveRightCmd});
        //sinistra
        inputHandler.bindCommand(EAppState::START, this, {SPACE_ENGINE_JK_BUTTON_LEFT, EInputType::SPACE_ENGINE_INPUT_JOYSTICK, m_pMoveLeftCmd});
        inputHandler.bindCommand(EAppState::START, this, {SPACE_ENGINE_KEY_BUTTON_A, EInputType::SPACE_ENGINE_INPUT_KEYBOARD, m_pMoveLeftCmd});
    }

    void UINavigator::unbindCommands()
    {
        InputHandler& inputHandler = App::GetInputHandler();
        inputHandler.clearBindingsFor(this);
    }   

    UINavigator::~UINavigator()
    {
        assert(count > 0);

        count--;
        if(!count)
        {
            delete m_pMoveDownCmd;
            delete m_pMoveUpCmd;
            delete m_pOnClickCmd;
            delete m_pOnPressCmd;
            delete m_pMoveRightCmd;
            delete m_pMoveLeftCmd;
        }

        InputHandler& inputHandler = App::GetInputHandler();
        inputHandler.clearBindingsFor(this);
    }

    void UINavigator::addButton(Button* button)
    {
        m_vecButtons.push_back(button);
    }

    void UINavigator::move(int delta)
    {
        m_focused = (m_focused + delta + static_cast<int>(m_vecButtons.size())) % static_cast<int>(m_vecButtons.size());
    }

    void UINavigator::inputRight()
    {
        if (m_focused >= 0 && m_focused < m_vecButtons.size())
        {
            if (m_vecButtons[m_focused]->onRight) {
                m_vecButtons[m_focused]->onRight();
            }
        }
    }

    void UINavigator::inputLeft()
    {
        if (m_focused >= 0 && m_focused < m_vecButtons.size())
        {
            if (m_vecButtons[m_focused]->onLeft) {
                m_vecButtons[m_focused]->onLeft();
            }
        }
    }

    void UINavigator::launchOnClick()
    {
        assert(m_focused >= 0 && m_focused < m_vecButtons.size());
        if(m_vecButtons[m_focused]->isHovered()){
            m_vecButtons[m_focused]->onClick();
            SPACE_ENGINE_DEBUG("Launch OnClick");
        }
    }

    void UINavigator::launchOnPress()
    {
        assert(m_focused >= 0 && m_focused < m_vecButtons.size());
        m_vecButtons[m_focused]->onClick();
        SPACE_ENGINE_DEBUG("Launch OnClick");
    }
    
    void UINavigator::update()
    {
        if (s_currentActiveNavigator != this)
        {
            if (s_currentActiveNavigator != nullptr)
            {
                s_currentActiveNavigator->unbindCommands();
            }
            bindCommands();
            s_currentActiveNavigator = this;
        }
        if(!Joystick::isConnected())
        {
            for(int i = 0; i < m_vecButtons.size(); i++)
            {
                if(m_vecButtons[i]->update(Mouse::getPosX(), Mouse::getPosY()))
                {
                    m_focused = i;
                    return;
                }
            }
        }
    }

    std::vector<UIRenderObject> UINavigator::gatherUIRenderables()
    {
        std::vector<UIRenderObject> uiRendObj;

        for(Button* button : m_vecButtons)
        {
            UIRenderObject uiRObj;
            uiRObj.pMaterial = button->pUIMeshRend->getMaterial();
            uiRObj.pRect = button->pUITransf->getRect();
            uiRObj.pUIMesh = button->pUIMeshRend->getUIMesh();
            uiRendObj.push_back(uiRObj);
        }

        return uiRendObj;
    }

    void UINavigator::notifyChangeRes()
    {
        for(Button* button : m_vecButtons)
        {
            button->pUITransf->setDirty(true);
        }
    }

    //-------------------------------------//
    //---------------UILayout--------------//
    //-------------------------------------//
    int UILayout::removeUIElement(const UIBase* pUIBase)
    {
        auto it = std::find(m_vecUIElements.begin(), m_vecUIElements.end(), pUIBase);
        if(it != m_vecUIElements.end())
        {
            m_vecUIElements.erase(it);
            return 1;
        }
        
        SPACE_ENGINE_ERROR("UILayout::removeUIElement: UIElement not found");
        return 0;
    }

    std::vector<UIRenderObject> UILayout::gatherUIRenderables()
    {
        std::vector<UIRenderObject> vecUIRenderObj;
        
        for(UIBase* uiElement : m_vecUIElements)
        {
            UIRenderObject uiRObj;
            uiRObj.pMaterial = uiElement->pUIMeshRend->getMaterial();
            uiRObj.pRect = uiElement->pUITransf->getRect();
            uiRObj.pUIMesh = uiElement->pUIMeshRend->getUIMesh();
            vecUIRenderObj.push_back(uiRObj);
        }

        if(m_pNavigator)
        {
            std::vector<UIRenderObject> vecUIRendObjNav;
            vecUIRendObjNav = m_pNavigator->gatherUIRenderables();
            
            vecUIRenderObj.insert(
                vecUIRenderObj.end(),
                std::move_iterator(vecUIRendObjNav.begin()),
                std::move_iterator(vecUIRendObjNav.end())
            );
        }
            
        return vecUIRenderObj;
    }

    std::vector<TextRenderObject> UILayout::gatherTextRenderables()
    {
        std::vector<TextRenderObject> vecTextRenderObj;
        
        for(Text* pText : m_vecText)
        {
            TextRenderObject textRObj;
            textRObj.pText = pText;
            vecTextRenderObj.push_back(textRObj);
        }

            
        return vecTextRenderObj;
    }

    void UILayout::notifyChangeRes()
    {
        for(UIBase* pUIElement : m_vecUIElements)
        {
            pUIElement->pUITransf->setDirty(true);
        }

        for(Text* pText: m_vecText)
        {
            pText->pTransf->setDirty(true);
        }        

        if(m_pNavigator)
        {
            m_pNavigator->notifyChangeRes();
        }
    }


}