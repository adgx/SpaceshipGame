#pragma once

#include <vector>

#include "mesh.h"
#include "transform.h"
#include "utils/utils.h"
#include "managers/windowManager.h"
#include "managers/inputManager.h"
#include "log.h"


namespace SpaceEngine
{
    //resolution used in figma
    constexpr int REF_WIDTH  = 1920;
    constexpr int REF_HEIGHT = 1080;
    //px screen info
    struct Rect
    {
        Rect() = default;
        Rect(float x, float y, float width, float height)
        {
            pos.x = x;
            pos.y = y;
            size.x = width;
            size.y = height;
        }

        static bool pointInRect(const Rect& r, float x, float y)
        {
            //SPACE_ENGINE_DEBUG("Coordinates: x{} y{}", x, y);
            return x >= r.pos.x && x <= r.pos.x + r.size.x &&
                    y >= r.pos.y && y <= r.pos.y + r.size.y;
        }
        //add Vector2
        Vector2 pos = Vector2{0.f, 0.f};
        Vector2 size = Vector2{0.f, 0.f}; //width, height 
    };

    class UITransform
    {
        public:
            inline void setWidth(float width)
            {
                dirty = true;
                size.x = width;
            }

            inline void setHeight(float height)
            {
                dirty = true;
                size.y = height;
            }
            
            inline void setPos(Vector2 pos)
            {
                dirty = true;
                this->pos = pos;
            }

            inline void setFill(bool flag)
            {
                dirty = true;
                fill = flag;
            }

            inline void setDirty(bool flag) { dirty = flag;}
            inline Vector2 getPos(){return pos;}
            inline float getWidth(){return size.x;}
            inline float getHeight(){return size.y;}
            inline bool getFill(){return fill;}

            void setAnchor(Vector2 anchor);
            void setSize(Vector2 size);
            Vector2 getAnchor();
            int getX();
            int getY();
            Rect* getRect();
            UITransform() = default;
            ~UITransform() = default;

        private:
            //UILayout 
            //relative to screen info-> mapping the screen to the range [0, 1] to ref resolution
            Vector2 anchor = {0.f, 0.f}; // normalized 0..1 
            Vector2 size = {0.f, 0.f};   // not normalizated 
            Vector2 pos = {0.f, 0.f};
            Rect rectCached;
            bool dirty = true;
            bool fill = false;
    };

    class UIBase
    {
        public:
            UIBase();
            UIBase(UIMaterial* pUIMaterial);
            //posAncor is in the range[0,1], pos is in px
            UIBase(Vector2 posAncor, Vector2 pos, UIMaterial* pUIMaterial);
            ~UIBase();
            UIMeshRenderer* pUIMeshRend = nullptr;
            UITransform* pUITransf = nullptr;    
    };

    class Background : public UIBase
    {
        public:
            Background(UIMaterial* pUIMaterial);
            ~Background() = default;
    };

    class Button : public UIBase
    {
        public: 
            Button(Vector2 anchor, Vector2 pos, UIButtonMaterial* pMat);
            Button(Vector2 anchor, Vector2 pos, UIButtonMaterial* pMat, std::function<bool()> func);
            ~Button() = default;
            bool update(int mx, int my);
            bool isHovered();
            std::function<bool()> onClick;
        private:
            bool hovered=false;
            bool wasHovered=false;
            bool wasPressed=false;
        
    };

    class UINavMoveDownCommand;
    class UINavMoveUpCommand;
    class UINavOnClickCommand;
    class UINavOnPressCommand;
    struct UIRenderObject;

    class UINavigator
    {
        friend UINavMoveDownCommand;
        friend UINavMoveUpCommand;
        friend UINavOnClickCommand;
        friend UINavOnPressCommand;

        public:
            UINavigator();
            ~UINavigator();
            std::vector<UIRenderObject> gatherUIRenderables();
            void notifyChangeRes();
            void addButton(Button* button);
            void update();
        private:
            void move(int delta);
            void launchOnClick();
            void launchOnPress();
            std::vector<Button*> m_vecButtons;
            //Make them static in future
            static UINavMoveDownCommand* m_pMoveDownCmd;
            static UINavMoveUpCommand* m_pMoveUpCmd;
            static UINavOnClickCommand* m_pOnClickCmd;
            static UINavOnPressCommand* m_pOnPressCmd;
            static int count;
            int m_focused = 0;
    };

    class UINavMoveDownCommand : public Command
    {
        public:
            virtual void execute(void* actor) override
            {
                UINavigator* nav = static_cast<UINavigator*>(actor);
                nav->move(1);
            }
    };

    class UINavMoveUpCommand : public Command
    {
        public:
            virtual void execute(void* actor) override
            {
                UINavigator* nav = static_cast<UINavigator*>(actor);
                nav->move(-1);
            }
    };

    class UINavOnClickCommand : public Command
    {
        public:
            virtual void execute(void* actor) override
            {
                UINavigator* nav = static_cast<UINavigator*>(actor);
                nav->launchOnClick();
            }
    };

    class UINavOnPressCommand : public Command
    {
        public:
            virtual void execute(void* actor) override
            {
                UINavigator* nav = static_cast<UINavigator*>(actor);
                nav->launchOnPress();
            }
    };

    class UILayout
    {
        public:
            UILayout() = default;
            template<typename T>
            void addComponent()
            {
                if constexpr (std::is_base_of_v<UINavigator, T>)
                {
                    if(!m_pNavigator)
                    {
                        m_pNavigator = new UINavigator();
                    }

                    else{SPACE_ENGINE_ERROR("You can add only a one UINavigator per UILayout");}
                }
            }

            template<typename T>
            void addUIElement(T pUIElement)
            {
                using PureT = std::remove_pointer_t<T>;

                if(pUIElement == nullptr)
                {
                    SPACE_ENGINE_ERROR("The passed component is null");
                    return;
                }
                if constexpr (std::is_base_of<Button, PureT>::value)
                {
                    if(!m_pNavigator)
                    {
                        m_pNavigator = new UINavigator();
                    }

                    m_pNavigator->addButton(pUIElement);
                    
                    return;
                }
                else if constexpr (std::is_base_of<Background, PureT>::value)
                {
                    m_vecUIElements.push_back(pUIElement);
                    return;
                }
                else if constexpr (std::is_base_of<UIBase, PureT>::value)
                {
                    m_vecUIElements.push_back(pUIElement);
                    return;
                }
                SPACE_ENGINE_ERROR("UIElement not valid!");
            }

            inline void update()
            {
                if(m_pNavigator)
                {
                    m_pNavigator->update();
                }
            }
            void notifyChangeRes();
            int removeUIElement(const UIBase* pUIBase);
            std::vector<UIRenderObject> gatherUIRenderables();
        private:
            std::vector<UIBase*> m_vecUIElements;
            UINavigator* m_pNavigator = nullptr;
    };
}