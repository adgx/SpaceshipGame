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

            void gatherRenderables(std::vector<RenderObject>& worldRenderables, 
                std::vector<UIRenderObject>& uiRenderables, 
                std::vector<TextRenderObject>& textRenderables,
                std::vector<ScreenRenderObject>& screenRenderables);
                
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
            //may be is useful a table with double link list to keep the gameObjects
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
            std::vector<ScreenRenderObject> m_vecScreenRendObj;
            std::string name;
            std::vector<UILayout*> m_vecUILayouts;
    };

    class SpawnerObs;

    class SpawnerSys
    {
        public:
            enum ESlot
            {
                FREE,
                ASTEROID,
                ENEMY
            };

            SpawnerSys();
            ~SpawnerSys() = default;

            void handlerSpawn(float dt);
            void clearSpace();
            inline void setScene(Scene* pScene){ m_pScene = pScene;}
            inline SpawnerObs* getObserver() { return m_pSpawnerObs;}


        private:
        enum ESpawnState
        {
            SPAWN_ASTEROID_EASY,
            SPAWN_ASTEROID_MED,
            SPAWN_ASTEROID_HARD,
            SPAWN_ENEMY_EASY,
            SPAWN_ENEMY_MED,
            SPAWN_ENEMY_HARD,
            SPAWN_MIX
        };
        
        struct Stage
        {
            float weights[3];
            ESpawnState eStage;
            uint32_t budget;
            uint32_t minSpawn;
            uint32_t maxSpawn;
            float spawnInterval;
            float bulletSpeed;
        };
        
        inline float getPosX(int index){return StartOffsetSpace + index * OffsetSpace;}
        int weightedRandom(float* weight, int dim);
        void getAvailableSlot(int* available, int& dim);
        int pickSlot(int prev, int index, int spawnCount);
        uint32_t spawnEntities(Stage& stage, uint32_t spawnCount);
        void spawnLogic();
        void spawnAsteroid(uint32_t spawnCount);
        void spawnEnemy(uint32_t spawnCount, uint32_t& nSpawned);

        void spawnPowerUp(); 
        float m_powerupTimer = 0.0f;
        float m_powerupInterval = 10.0f;

        float randomRange(float min, float max) {
            return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
        }
        
        static constexpr int32_t BudgetAsteroidE = 5;
        static constexpr int32_t BudgetAsteroidM = 9;
        static constexpr int32_t BudgetAsteroidH = 12;
        static constexpr int32_t BudgetEnemyE = 5;
        static constexpr int32_t BudgetEnemyM = 5;
        static constexpr int32_t BudgetEnemyH = 5;
        
        static constexpr float FarDistance = -80.f;
        static constexpr float DeltaSpace = 2.5f;          
        static constexpr float StartOffsetSpace = -4.67f - DeltaSpace; //-7.0 + (14/6) = -4.67; 14/6 = 2.33  
        static constexpr float OffsetSpace = 2.33f + DeltaSpace;

        static constexpr float VelAsterorid = 40.f;            
        static constexpr float TimeAsterorid = 3.5f;
        static constexpr float TimeAsteroridXM = 0.75f;
        static constexpr float TimeAsteroridXH = 0.6f;
        
        static constexpr float VelEnemy = 50.f;
        static constexpr float TimeEnemy = 3.5f;
        static constexpr float TimeEnemyXM = 0.75f;
        static constexpr float TimeEnemyXH = 0.6f;

        static Stage m_lookupStages[];

        float m_timer = TimeAsterorid;
        Stage m_stage;
        Scene* m_pScene;
        SpawnerObs* m_pSpawnerObs;
    };

    class SpawnerObs : public Observer<GameObject, int>
    {
        public: 
            void onNotify(const GameObject& entity, const int& event) override;
            static constexpr int32_t SlotDim = 3;
            int8_t space[SlotDim] = {SpawnerSys::ESlot::FREE, SpawnerSys::ESlot::FREE, SpawnerSys::ESlot::FREE};
    };

    class SpawnerSubject : public Subject<GameObject, int>
    {
        public:
            SpawnerSubject(int ticket);
            inline void setTicket(int ticket){ m_ticket = ticket;}
            inline int getTicket(){ return m_ticket;}
            void notifyDestroy(GameObject& pGameObj);

            private:
            int m_ticket = 0;
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
            virtual void OnSwitch() override;
            
            void removeHealthIcon();
            void ResetGame();
            void TogglePause();
            void removePauseLayout(UILayout* layout);
            void TriggerBomb();
            void AddHealthIcon();
            
            uint32_t GetCurrentScore();
            
            inline void SetPlayer(PlayerShip* player) { m_pPlayer = player; }
            inline PlayerShip* GetPlayer() { return m_pPlayer;}
            
            //static
            static Bullet* pBulletEnemy; 
            static ScoreSys* pScoreSys;
            static SpawnerSys* pSpawnerSys;
            //GESTIONE SPAWN
            static EnemyShip* m_pEnemy;
            static std::vector<Asteroid*> m_asteroids;
            static PlayerShip* m_pPlayer; 

        private:
            void UpdateScene(float dt) override;
            float randomRange(float min, float max); 
            void handleSpawning(float dt);
            void ResetHealthIcons();
            
            UILayout* m_pHUDLayout = nullptr;
            PauseScene* m_pPauseScene = nullptr;
            
            float m_asteroidTimer = 0.0f;
            float m_powerupTimer = 0.0f;
            float m_enemyTimer = 0.0f;
            // Intervalli di spawn
            float m_asteroidInterval = 3.0f; 
            float m_enemyInterval = 7.0f;
            float m_powerupInterval = 5.0f;
            // Limiti dell'area di gioco dove possono spawnare
            float m_spawnZ = -100.0f; // Lontano dalla camera
            float m_gameAreaX = 10.0f; // Larghezza totale area spawn
            float m_gameAreaY = 10.0f; // Altezza totale area spawn
            float m_elapsedTime = 0.0f;
            unsigned int m_points = 0;
            float m_timer = 0.f;
            bool m_asteroidDebug = false;
            
            bool m_escProcessed = false;
            std::stack<UIBase*> healthIcons;
    };

    class StartupScene : public Scene
    {
        public:
        private:
    };
}