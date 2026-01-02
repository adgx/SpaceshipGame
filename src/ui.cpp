#include <algorithm>

#include "ui.h"
#include "app.h"
#include "renderer.h"
#include "shader.h"

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
        pUIMeshRend = new UIMeshRenderer();
        pUIMeshRend->bindMaterial(pUIMaterial);
        
        if(!pUIMaterial->pShader)
            pUIMaterial->pShader = ShaderManager::findShaderProgram("ui");
        
        pUITransf = new UITransform();
        Texture* pTex = pUIMaterial->getTexture("ui_tex");
        int w = 0;
        int h = 0;
        pTex->getImageSize(w, h);
        pUITransf->setWidth(w);
        pUITransf->setHeight(h);

    }

    UIBase::UIBase(Vector2 posAncor, UIMaterial* pUIMaterial)
    {
        pUIMeshRend = new UIMeshRenderer();
        pUIMeshRend->bindMaterial(pUIMaterial);

        if(!pUIMaterial->pShader)
            pUIMaterial->pShader = ShaderManager::findShaderProgram("ui");
        
        pUITransf = new UITransform();
        pUITransf->setAnchor(posAncor);
        Texture* pTex = pUIMaterial->getTexture("ui_tex");
        int w = 0;
        int h = 0;
        pTex->getImageSize(w, h);
        pUITransf->setWidth(w);
        pUITransf->setHeight(h);
    }

    UIBase::~UIBase()
    {
        delete pUIMeshRend;
        delete pUITransf;
    }

    //---------------------------------//
    //-----------UITrasform------------//
    //---------------------------------//
    int UITransform::resWidth = 0; 
    int UITransform::resHeight = 0;
    Rect* UITransform::getRect()
    {
        if(resWidth !=WindowManager::width || resHeight != WindowManager::height)
        {
            resWidth = WindowManager::width;
            resHeight = WindowManager::height;
            dirty = true;
        }

        if((dirty))
        {
            float w = size.x * WindowManager::width;
            float h = size.y * WindowManager::height;
            //x and y are the top left of the quad
            float x = anchor.x * WindowManager::width;
            float y = anchor.y * WindowManager::height;

            rectCached = Rect(x, y, w, h);
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
    Button::Button(Vector2 anchor, UIButtonMaterial* pMat):UIBase(anchor, pMat)
    {
        pUITransf->setAnchor(anchor);
    }

    Button::Button(Vector2 anchor, UIButtonMaterial* pMat, std::function<bool()> func):Button(anchor, pMat)
    {
        onClick = func; 
    }

    Button::Button(Vector2 anchor, Vector2 size)
    {
        pUITransf->setAnchor(anchor);
        pUITransf->setSize(size);
        UIButtonMaterial* pMat = MaterialManager::createMaterial<UIButtonMaterial>(std::to_string(reinterpret_cast<std::uintptr_t>(this)));
        pUIMeshRend->bindMaterial(pMat);
        if(!pMat->pShader)
            pMat->pShader = ShaderManager::findShaderProgram("uiButton");
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
    Background::Background()
    {
        pUITransf->setAnchor(Vector2{0.f, 0.f});
        pUITransf->setSize(Vector2{1.f, 1.f});
    }

    Background::Background(UIMaterial* pUIMaterial):UIBase(pUIMaterial)
    {
        pUITransf->setAnchor(Vector2{0.f, 0.f});
        pUITransf->setSize(Vector2{1.f, 1.f});
    }

    //---------------------------------//
    //-----------UINavigator-----------//
    //---------------------------------//
    UINavMoveDownCommand* UINavigator::m_pMoveDownCmd = nullptr;
    UINavMoveUpCommand* UINavigator::m_pMoveUpCmd = nullptr;
    UINavOnClickCommand* UINavigator::m_pOnClickCmd = nullptr;
    UINavOnPressCommand* UINavigator::m_pOnPressCmd = nullptr;
    
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
        }

        InputHandler& inputHandler = App::GetInputHandler();
        //Down command for joystick and keyboard
        inputHandler.bindCommand(EAppState::RUN, 
            this, 
            {SPACE_ENGINE_JK_BUTTON_DOWN, 
                EInputType::SPACE_ENGINE_INPUT_JOYSTICK, 
                m_pMoveDownCmd});
        inputHandler.bindCommand(EAppState::RUN, 
            this, 
            {SPACE_ENGINE_KEY_BUTTON_S, 
                EInputType::SPACE_ENGINE_INPUT_KEYBOARD, 
                m_pMoveDownCmd});
        
        //UP command for joystick and keyboard
        inputHandler.bindCommand(EAppState::RUN, 
            this, 
            {SPACE_ENGINE_JK_BUTTON_UP, 
                EInputType::SPACE_ENGINE_INPUT_JOYSTICK, 
                m_pMoveUpCmd});
        inputHandler.bindCommand(EAppState::RUN, 
            this, 
            {SPACE_ENGINE_KEY_BUTTON_W, 
                EInputType::SPACE_ENGINE_INPUT_KEYBOARD, 
                m_pMoveUpCmd});

        //Click command for joystick and keyboard
        inputHandler.bindCommand(EAppState::RUN, 
            this, 
            {SPACE_ENGINE_JK_BUTTON_A, 
                EInputType::SPACE_ENGINE_INPUT_JOYSTICK, 
                m_pOnPressCmd});
        inputHandler.bindCommand(EAppState::RUN, 
            this, 
            {SPACE_ENGINE_KEY_BUTTON_ENTER, 
                EInputType::SPACE_ENGINE_INPUT_KEYBOARD, 
                m_pOnClickCmd});
        count++;
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

}