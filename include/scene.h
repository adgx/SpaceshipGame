#pragma once

#include "utils/stb_image.h"
#include "renderer.h"
#include "camera.h"
#include "collisionDetection.h"
#include "light.h"
#include "log.h"
#include "shader.h"
#include "managers/audioManager.h"
#include "bullet.h"

#include "sceneManager.h"
#include "pauseScene.h"
#include <vector>
#include <string>
#include <queue>
#include <stack>

using namespace std;

namespace SpaceEngine
{
    class GameObject;
    class PlayerShip;
    class Asteroid;
    class EnemyShip;
    
    class Scene
    {
        public:
            Scene(PhysicsManager* pPhyManager):pPhyManager(pPhyManager)
            {
                //add a constructor where you can pass the path of skybox
                pSkybox = nullptr;
            };
            virtual ~Scene() = default;
            
            virtual void OnLoad()
            {
                SPACE_ENGINE_INFO("On load scene");
            }

            virtual void OnSwitch()
            {
                SPACE_ENGINE_INFO("On switch on the scene");
            }

            virtual void OnUnload()
            {
                SPACE_ENGINE_INFO("On unload scene");
            }

            void Init();

            inline void setActive(bool flag){active = flag;}
            inline bool isActive() const{return active;}
            inline void setNameScene(const std::string& name){this->name = name;}
            inline std::string getNameScene() const{return name;}
            inline Skybox* getSkybox() const{return pSkybox;}
            void notifyChangeRes();

            void setAudioManager(AudioManager* am) { pAudioManager = am; }
            AudioManager* getAudioManager() { return pAudioManager; }
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
                    return;
                }
                SPACE_ENGINE_ERROR("Component not valid!");
            }

            void gatherRenderables(std::vector<RenderObject>& worldRenderables, std::vector<UIRenderObject>& uiRenderables, std::vector<TextRenderObject>& textRenderables);
            void requestDestroy(GameObject* pGameObj);
            
            template <typename T>
            T* requestInstantiate(const T* prefab, float time = 0.f)
            {
                return requestInstantiateImpl(prefab, time, false, {});
            }

            template <typename T>
            T* requestInstantiate(const T* prefab, Vector3 wPos)
            {
                return requestInstantiateImpl(prefab, 0.0f, true, wPos);
            }

            template <typename T>
            T* requestInstantiate(const T* prefab, float time, Vector3 wPos)
            {
                return requestInstantiateImpl(prefab, time, true, wPos);
            }

            BaseCamera* getActiveCamera() const;
            std::vector<Light*>* getLights() const; 
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
            T* requestInstantiateImpl(const T* prefab,
                                        float time,
                                        bool overrideWorldPos,
                                        const Vector3& wPos)
            {
                static_assert(std::is_base_of_v<GameObject, T>,
                              "T must derive from GameObject");
                
                SpawnRequest sr;
                T* pObj = new T(*prefab);
                sr.prefab = pObj;  
                sr.timeRemaining = time;
                sr.overrideWorldPos = overrideWorldPos;
                sr.wPos = wPos;
                  
                enqueueSpawn(std::move(sr));

                return pObj;
            }

            //Don't use it to instantiate GameObjects directly instead use RequestInstatiate
            GameObject* instantiate(const SpawnRequest& sr);
            //may be is useful a table with duoble link list to keep the gameObjects
            //for avoing a complex gathering
            //vector<GameObject*> gameObjects;
            std::queue<GameObject*> destroyQ;
            std::list<SpawnRequest> spawnQ;
            std::vector<Light*> lights;
            //cameras[0] is always the active camera
            vector<BaseCamera*> cameras;
            Skybox* pSkybox = nullptr;
            AudioManager* pAudioManager = nullptr;
            //scene property
            bool active = true;
        protected:
            vector<GameObject*> gameObjects;
            std::string name;
            std::vector<UILayout*> m_vecUILayouts;
    };

    class ScoreSys : public Observer<GameObject, int>
    {
        public:
            void onNotify(const GameObject& entity, const int& event) override;
            void Reset(); 
            uint32_t GetScore() const { return m_score; }
            Text* pTextPoints;
        private:
            uint32_t m_score = 0; 
    };

    class PointSubject : public Subject<GameObject, int>
    {
        public:
            PointSubject();
            void notifyPoints(GameObject& pGameObj, int score);
    };
    

    class SpaceScene : public Scene
    {
            
        public:
            SpaceScene(PhysicsManager* pPhyManager);
            ~SpaceScene();
            void removeHealthIcon();
            void SetPlayer(PlayerShip* player) { m_pPlayer = player; }
            virtual void OnSwitch() override;
            static ScoreSys* pScoreSys;
            static Bullet* pBulletEnemy; 
            void ResetGame();
            void TogglePause();
            void removePauseLayout(UILayout* layout);
            
            uint32_t GetCurrentScore();
        private:
            void UpdateScene(float dt) override;
            float randomRange(float min, float max); 
            void handleSpawning(float dt);

            //GESTIONE SPAWN
            static EnemyShip* m_pEnemy;
            static Asteroid* m_pAsteroid;
            float m_asteroidTimer = 0.0f;
            float m_enemyTimer = 0.0f;
            // Intervalli di spawn
            float m_asteroidInterval = 3.0f; 
            float m_enemyInterval = 7.0f;
            // Limiti dell'area di gioco dove possono spawnare
            float m_spawnZ = -100.0f; // Lontano dalla camera
            float m_gameAreaX = 10.0f; // Larghezza totale area spawn
            float m_gameAreaY = 10.0f; // Altezza totale area spawn
            float m_elapsedTime = 0.0f;
            unsigned int m_points = 0;
            float m_timer = 0.f;
            bool m_asteroidDebug = false;
            PlayerShip* m_pPlayer = nullptr;
            std::stack<UIBase*> healthIcons;
           
            PauseScene* m_pPauseScene = nullptr;
            bool m_escProcessed = false;
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