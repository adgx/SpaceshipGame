#pragma once
#include "mesh.h"
#include "transform.h"
#include "utils/utils.h"
#include "managers/windowManager.h"


namespace SpaceEngine
{
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
            return x >= r.pos.x && x <= r.pos.x + r.size.x &&
                    y >= r.pos.y && y <= r.pos.y + r.size.y;
        }
        //add Vector2
        Vector2 pos = Vector2{0.f, 0.f};
        Vector2 size = Vector2{1.f, 1.f}; //width, height 
    };

    class UITransform
    {
        public:
            template<typename T>
            void setWidth(T width)
            {
                static_assert(std::is_same_v<T, int> || std::is_same_v<T, float>,
                      "T must be int or float");
                dirty = true;
                if constexpr (std::is_same_v<T, int>)
                    size.x = std::clamp(width/static_cast<float>(WindowManager::width), 0.f, 1.f);
                else if constexpr (std::is_same_v<T, float>)
                    size.x = std::clamp(width, 0.f, 1.f);
            }

            template<typename T>
            void setHeight(T height)
            {
                static_assert(std::is_same_v<T, int> || std::is_same_v<T, float>,
                      "T must be int or float");
                dirty = true;
                if constexpr (std::is_same_v<T, int>)
                    size.y = std::clamp(height/static_cast<float>(WindowManager::height), 0.f, 1.f);
                else if constexpr (std::is_same_v<T, float>)
                    size.y = std::clamp(height, 0.f, 1.f);
            }

            template<typename T>
            T getWidth()
            {
                static_assert(std::is_same_v<T, int> || std::is_same_v<T, float>,
                      "T must be int or float");
                if constexpr (std::is_same_v<T, int>)
                    return static_cast<int>(size.x * WindowManager::width);
                else if constexpr (std::is_same_v<T, float>)
                    return size.x;
            }

            template<typename T>
            T getHeight()
            {
                static_assert(std::is_same_v<T, int> || std::is_same_v<T, float>,
                      "T must be int or float");
                if constexpr (std::is_same_v<T, int>)
                    return static_cast<int>(size.y * WindowManager::width);
                else if constexpr (std::is_same_v<T, float>)
                    return size.y;
            }

            void setAnchor(Vector2 anchor);
            void setSize(Vector2 size);
            Vector2 getAnchor();
            int getX();
            int getY();
            Rect getRect();
            UITransform() = default;
            ~UITransform() = default;

        private:
            //UILayout 
            //relative to screen info-> mapping the screen to the range [0, 1]
            Vector2 anchor = {0.5f, 0.5f}; // normalized 0..1
            Vector2 offset = {0, 0};       // pixel offset
            Vector2 size = {0.2f, 0.1f};   // normalized width/height (0..1)
            Rect rectCached;
            bool dirty = true;
            static int resWidth; 
            static int resHeight; 
    };

    class UIBase
    {
        public:
            UIBase();
            ~UIBase();
            UIMeshRenderer* pUIMeshRend = nullptr;
            UITransform* pUITransf = nullptr;    
    };

    class Background : public UIBase
    {
        Background();
        ~Background() = default;
    };

    class Button : public UIBase
    {
        public:
            Button(Vector2 anchor, Vector2 size);
            ~Button() = default;
            void update(float mx,float my, bool pressed);
            std::function<void()> onClick;
        private:
            bool hovered=false;
            bool wasHovered=false;
            bool wasPressed=false;
        
    };
}