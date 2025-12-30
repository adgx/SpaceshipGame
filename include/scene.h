#pragma once
#include "utils/stb_image.h"
#include "renderer.h"
#include "camera.h"
#include "gameObject.h"
#include "collisionDetection.h"
#include "light.h"
#include "log.h"
#include "shader.h"
#include "Asteroid.h"
#include "EnemyShip.h"
#include <vector>
#include <string>
#include <queue>
#include <stack>

using namespace std;

namespace SpaceEngine
{
    class GameObject;
    
    class Scene
    {
        public:
            Scene(PhysicsManager* pPhyManager):pPhyManager(pPhyManager)
            {
                //add a constructor where you can pass the path of skybox
                pSkybox = nullptr;
            };
            virtual ~Scene() = default;
            
            void OnLoad()
            {
                SPACE_ENGINE_INFO("On load scene");
            }
            void OnUnload()
            {
                SPACE_ENGINE_INFO("On unload scene");
            }

            void Init();
            void setActive(bool flag);
            bool isActive() const;
            void setNameScene(const std::string& name);
            std::string getNameScene() const;

            template<typename T>
            void addSceneComponent(T sceneComponent)
            {
                using PureT = std::remove_pointer_t<T>;

                if(sceneComponent == nullptr)
                {
                    SPACE_ENGINE_ERROR("The passed component is null");
                    return;
                }
                if constexpr (std::is_base_of<GameObject, PureT>::value)
                {
                    gameObjects.push_back(sceneComponent);
                    Collider * pCol = sceneComponent->getComponent<Collider>();
                    
                    if(pCol != nullptr) 
                    {
                        pPhyManager->AddCollider(pCol); 
                    }
                    
                    return;
                }
                else if constexpr (std::is_base_of<BaseCamera, PureT>::value)
                {
                    cameras.push_back(sceneComponent);
                    return;
                }
                else if constexpr (std::is_base_of<Light, PureT>::value)
                {
                    lights.push_back(sceneComponent);
                    return;
                }
                else if constexpr (std::is_base_of<UILayout, PureT>::value)
                {
                    m_vecUILayouts.push_back(sceneComponent);
                }
                SPACE_ENGINE_ERROR("Component not valid!");
            }

            void gatherColliderables();
            void gatherRenderables(std::vector<RenderObject>& worldRenderables, std::vector<UIRenderObject>& uiRenderables);
            void requestDestroy(GameObject* pGameObj);
            
            template <typename T>
            void requestInstantiate(const T* prefab, float time = 0.f)
            {
                requestInstantiateImpl(prefab, time, false, {});
            }

            template <typename T>
            void requestInstantiate(const T* prefab, Vector3 wPos)
            {
                requestInstantiateImpl(prefab, 0.0f, true, wPos);
            }

            template <typename T>
            void requestInstantiate(const T* prefab, float time, Vector3 wPos)
            {
                requestInstantiateImpl(prefab, time, true, wPos);
            }

            BaseCamera* getActiveCamera() const;
            std::vector<Light*>* getLights() const; 
            Skybox* getSkybox() const;
            void Update(float dt);

        private:
            
            struct SpawnRequest
            {
                float timeRemaining = 0.f;
                GameObject* prefab = nullptr;
                bool overrideWorldPos = false;
                Vector3 wPos;
            };            

            PhysicsManager* pPhyManager = nullptr;
            virtual void UpdateScene(float dt){}
            void processDestroyQ();
            void processInstantiateQ(float dt);
            inline void enqueueSpawn(SpawnRequest&& sr){spawnQ.push_back(std::move(sr));}
            template <typename T>
            void requestInstantiateImpl(const T* prefab,
                                        float time,
                                        bool overrideWorldPos,
                                        const Vector3& wPos)
            {
                static_assert(std::is_base_of_v<GameObject, T>,
                              "T must derive from GameObject");
                
                SpawnRequest sr;
                sr.prefab = new T(*prefab);  // ← exactly what you asked
                sr.timeRemaining = time;
                sr.overrideWorldPos = overrideWorldPos;
                sr.wPos = wPos;
                
                enqueueSpawn(std::move(sr));
            }

            //Don't use it to instantiate GameObjects directly instead use RequestInstatiate
            GameObject* instantiate(const SpawnRequest& sr);
            //may be is useful a table with duoble link list to keep the gameObjects
            //for avoing a complex gathering
            vector<GameObject*> gameObjects;
            std::queue<GameObject*> destroyQ;
            std::list<SpawnRequest> spawnQ;
            std::vector<Light*> lights;
            //cameras[0] is always the active camera
            vector<BaseCamera*> cameras;
            Skybox* pSkybox = nullptr;

            //scene property
            std::string name;
            bool active = true;
        protected:
            std::vector<UILayout*> m_vecUILayouts;
    };

    class SpaceScene : public Scene
    {
        public:
            SpaceScene(PhysicsManager* pPhyManager);
            ~SpaceScene() = default;
            void removeHealthIcon();
            
            private:
            void UpdateScene(float dt) override;
            float randomRange(float min, float max); 
            void handleSpawning(float dt);
            
            //GESTIONE SPAWN
            float m_asteroidTimer = 0.0f;
            float m_enemyTimer = 0.0f;
            // Intervalli di spawn
            float m_asteroidInterval = 3.0f; 
            float m_enemyInterval = 7.0f;
            // Limiti dell'area di gioco dove possono spawnare
            float m_spawnZ = -100.0f; // Lontano dalla camera
            float m_gameAreaX = 50.0f; // Larghezza totale area spawn
            float m_gameAreaY = 30.0f; // Altezza totale area spawn
            std::stack<UIBase*> healthIcons;
    };

    class DeathScene : public Scene
    {
        public:
        private:
    };

    class StartupScene : public Scene
    {
        public:
        private:
    };
}