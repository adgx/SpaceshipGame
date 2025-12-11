#include "ui.h"
#include <algorithm>
#include "log.h"

namespace SpaceEngine
{

    UIBase::UIBase()
    {
        pUIMeshRend = new UIMeshRenderer();
        pUITransf = new UITransform();
    }

    UIBase::~UIBase()
    {
        delete pUIMeshRend;
        delete pUITransf;
    }

    //UITrasform 
    static int resWidth = 0; 
    static int resHeight = 0;
    Rect UITransform::getRect()
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

        return rectCached;
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

    //Button
    Button::Button(Vector2 anchor, Vector2 size)
    {
        pUITransf->setAnchor(anchor);
        pUITransf->setSize(size);
        UIButtonMaterial* pMat = MaterialManager::createMaterial<UIButtonMaterial>(std::to_string(reinterpret_cast<std::uintptr_t>(this)));
        pUIMeshRend->bindMaterial(pMat);
        
    }
    void Button::update(float mx,float my, bool pressed)
    {
        hovered = Rect::pointInRect(pUITransf->getRect(),mx,my);
        if(hovered && !wasHovered)
        {
            UIButtonMaterial* pMat = dynamic_cast<UIButtonMaterial*>(pUIMeshRend->getMaterial());
            pMat->setSubroutineHover(true);
        }
        if(!hovered && wasHovered)
        {
            UIButtonMaterial* pMat = dynamic_cast<UIButtonMaterial*>(pUIMeshRend->getMaterial());
            pMat->setSubroutineHover(false);
        }
        //if(hovered && !pressed && wasPressed && onClick) onClick();
        wasPressed = pressed;
        wasHovered = hovered;
    }

    //Background
    Background::Background()
    {
        pUITransf->setAnchor(Vector2{0.f, 0.f});
        pUITransf->setSize(Vector2{1.f, 1.f});
    }
}