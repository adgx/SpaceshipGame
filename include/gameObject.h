#pragma once
#include "mesh.h"
#include "transform.h"
#include "material.h"
#include "log.h"
#include <vector>
#include <string> 

namespace SpaceEngine
{
    class Scene;
    class GameObject 
    {
        public:
            GameObject(){m_numInstances = 1;};
            virtual ~GameObject() = default;
            int getNumInstances();
            void destroy(); 
            virtual void update(float dt) = 0;
            
            template<typename T>
            T* getComponent()
            {
                if constexpr (std::is_same_v<T, Mesh>)
                {
                    return m_pMesh;
                }
                else if constexpr (std::is_same_v<T, std::vector<Transform*>>)
                {
                    return &m_vecTransform;
                }
            
                SPACE_ENGINE_ERROR("Component not found");
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
                    m_vecTransform.push_back(component);
                }
            }

        private:
            //Attention
            Scene* scene = nullptr;
        protected:
            GameObject(const std::string& filePathModel);
            int m_numInstances = 0;
            std::vector<Transform*> m_vecTransform;
            Mesh* m_pMesh = nullptr;
            //TODO: add componet for the physics 
    };
}
