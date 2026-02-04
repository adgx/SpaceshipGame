#pragma once
#include "mesh.h"
#include "transform.h"
#include "material.h"
#include "log.h"
#include <vector>
#include <string> 

namespace SpaceEngine
{
    enum class ELayers
    {
        DEFAULT_LAYER,
        PLAYER_LAYER,
        ENEMY_LAYER,
        BULLET_LAYER,
        ASTEROID_LAYER,
        BULLET_PLAYER_LAYER,
        BULLET_ENEMY_LAYER,
        POWERUP_LAYER,
    };

    class Scene;
    class Collider;
    class GameObject 
    {
        public:
            GameObject(Scene* pScene);
            GameObject(const GameObject& other);
            virtual ~GameObject();
            ELayers getLayer();
            void setLayer(ELayers layer);
            void destroy(); 
            virtual void update(float dt);
            virtual void fixedUpdate(float fixed_dt);
            virtual void onCollisionEnter(Collider* col);

            Transform* getTransform() const { 
                return m_pTransform; 
            }
            
            template<typename T>
            T* getComponent()
            {
                if constexpr (std::is_same_v<T, Mesh>)
                {
                    return m_pMesh;
                }
                else if constexpr (std::is_same_v<T, Transform>)
                {
                    return m_pTransform;
                }
                else if constexpr (std::is_same_v<T, Collider>)
                {
                    return m_pCollider;
                }
            
                //SPACE_ENGINE_WARN("Component not found");
                return nullptr;
            }

            template<typename T>
            void addComponent(T component)
            {
                if(component == nullptr)
                {
                    SPACE_ENGINE_ERROR("The passed component is null");
                    return;
                }
                if constexpr (std::is_same_v<T, Mesh*>)
                {
                    m_pMesh = component;
                }
                else if constexpr (std::is_same_v<T, Transform*>)
                {
                    m_pTransform = component;
                }
                else if constexpr (std::is_same_v<T, Collider*>)
                {
                    m_pCollider = component;
                }
            }
            bool pendingDestroy = false;
        private:
        
        protected:
        GameObject(const std::string& filePathModel);
        Transform* m_pTransform;
        Mesh* m_pMesh = nullptr;
        Collider* m_pCollider = nullptr;
        ELayers m_layer = ELayers::DEFAULT_LAYER;
        //Attention
        Scene* pScene = nullptr;
    };
}
