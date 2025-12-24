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
            ~Scene() = default;

            void Init();
            void createGameObject();

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
                SPACE_ENGINE_ERROR("Component not valid!");
            }

            void gatherColliderables();
            void gatherRenderables(std::vector<RenderObject>& worldRenderables, std::vector<UIRenderObject>& uiRenderables);
            void requestDestroy(GameObject* pGameObj);
            void requestInstatiate(GameObject* pGameObj);
            void requestInstatiate(GameObject* pGameObj, float time);
            void requestInstatiate(GameObject* pGameObj, Vector3 wPos);
            void requestInstatiate(GameObject* pGameObj, float time, Vector3 wPos);
            BaseCamera* getActiveCamera();
            std::vector<Light*>* getLights(); 
            Skybox* getSkybox();
            void Update(float dt);
        private:
            

            PhysicsManager* pPhyManager = nullptr;
            void processDestroyQ();
            void processInstantiateQ(float dt);
            
            struct SpawnRequest
            {
                float timeRemaining = 0.f;
                GameObject* prefab = nullptr;
                bool overrideWorldPos = false;
                Vector3 wPos;
            };
        
            GameObject* instatiate(const SpawnRequest& sr);
            //may be is useful a table with duoble link list to keep the gameObjects
            //for avoing a complex gathering
            vector<GameObject*> gameObjects;
            std::queue<GameObject*> destroyQ;
            std::list<SpawnRequest> spawnQ;
            std::vector<Light*> lights;
            //cameras[0] is always the active camera
            vector<BaseCamera*> cameras;
            Skybox* pSkybox = nullptr;

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

            void handleSpawning(float dt);
            float randomRange(float min, float max);
    };
}