#include "scene.h"
#include <glad/gl.h>
#include "log.h"
#include "font.h"
#include "Asteroid.h"
#include "EnemyShip.h"
#include "PlayerShip.h"
#include "gameObject.h"
#include "app.h"

namespace SpaceEngine
{

    //-----------------------------------------//
    //---------------Scene---------------------//
    //-----------------------------------------//
    BaseCamera* Scene::getActiveCamera() const
    {
        if(cameras.size() != 0 )
            return cameras[0];

        return nullptr;
    }

    void Scene::Init()
    {
        pSkybox = new Skybox();
    }

    void Scene::Update(float dt)
    {
        for(UILayout* pUILayout : m_vecUILayouts)
        {
            pUILayout->update();
        }

        if (App::state == EAppState::RUN) 
        {
            for (auto* obj : gameObjects)
                obj->update(dt);

            // cleanup gameobjects phase
            processInstantiateQ(dt);
            processDestroyQ();
        }

        UpdateScene(dt);
    }

    GameObject* Scene::instantiate(const SpawnRequest& sr)
    {
        if(sr.prefab)
        {
            if(sr.overrideWorldPos)
                sr.prefab->getComponent<Transform>()->setWorldPosition(sr.wPos);
            if(Collider* pCol = sr.prefab->getComponent<Collider>(); pCol != nullptr)
                pPhyManager->AddCollider(pCol);
        }
        else
        {
            SPACE_ENGINE_FATAL("Inable to copy and instatiate the GameObject");
        }

        return sr.prefab;
    }

    void Scene::processInstantiateQ(float dt)
    {
        for(auto it = spawnQ.begin(); it != spawnQ.end();)
        {
            it->timeRemaining -= dt;

            if(it->timeRemaining <= 0.f)
            {
                gameObjects.push_back(instantiate(*it));
                it = spawnQ.erase(it);
            }
            else ++it;
        }
    }

    void Scene::requestDestroy(GameObject* pGameObj)
    {
            destroyQ.push(pGameObj);
    }

    void Scene::processDestroyQ()
    {
        std::unordered_set<GameObject*> toDestroy;

        while(!destroyQ.empty())
        {
            toDestroy.insert(destroyQ.front());
            destroyQ.pop();
        }

        if(!toDestroy.empty())
            gameObjects.erase(
                std::remove_if(gameObjects.begin(), gameObjects.end(),
                    [&](GameObject* pGameObj)
                    {
                        bool flag = toDestroy.count(pGameObj) != 0;
                        if(flag){
                            if (auto col = pGameObj->getComponent<Collider>())
                                pPhyManager->RemoveCollider(col);
                            
                            delete pGameObj; 
                        }
                        return flag; 
                    }),
                gameObjects.end()
            );
    }

    void Scene::gatherRenderables(std::vector<RenderObject>& worldRenderables, std::vector<UIRenderObject>& uiRenderables, std::vector<TextRenderObject>& textRenderables)
    {
        for (auto& gameObj : gameObjects)
        {
            // --- World objects ---
            
            if (Mesh* mesh = gameObj->getComponent<Mesh>(); mesh)
            {
                
                for(int i = 0, nSubMeshes = mesh->getNumSubMesh(); i < nSubMeshes; ++i)
                {
                    RenderObject renderObj;
                    renderObj.mesh = mesh;
                    Transform* trasf = gameObj->getComponent<Transform>();
                    renderObj.modelMatrix = trasf->getWorldMatrix();
                    worldRenderables.push_back(renderObj);
                }
            }
        }
        
        // --- UI objects ---
        for(UILayout* pLayout : m_vecUILayouts)
        {
            if (std::vector<UIRenderObject> vecUIRendObjs = pLayout->gatherUIRenderables(); vecUIRendObjs.size())
            {
                uiRenderables.insert(
                    uiRenderables.end(),
                    std::move_iterator(vecUIRendObjs.begin()),
                    std::move_iterator(vecUIRendObjs.end())
                );
            }

            if (std::vector<TextRenderObject> vecTextRendObjs = pLayout->gatherTextRenderables(); vecTextRendObjs.size())
            {
                textRenderables.insert(
                    textRenderables.end(),
                    std::move_iterator(vecTextRendObjs.begin()),
                    std::move_iterator(vecTextRendObjs.end())
                );
            }
        }
    }

    std::vector<Light*>* Scene::getLights() const
    {
        if(lights.size() != 0)
            return const_cast<std::vector<Light*>*>(&lights);
        return nullptr;
    }

    void Scene::notifyChangeRes()
    {
        for(UILayout* pUILayout : m_vecUILayouts)
        {
            pUILayout->notifyChangeRes();
        }
    }


    //-----------------------------------------// 
    //---------------SpaceScene----------------//
    //-----------------------------------------// 
    ScoreSys* SpaceScene::pScoreSys = new ScoreSys();
    SpawnerSys* SpaceScene::pSpawnerSys = new SpawnerSys();
    Bullet* SpaceScene::pBulletEnemy = nullptr;
    EnemyShip* SpaceScene::m_pEnemy = nullptr;
    Asteroid* SpaceScene::m_pAsteroid = nullptr;

    SpaceScene::SpaceScene(PhysicsManager* pPhyManager):
    Scene(pPhyManager)
    {
        name = "SpaceScene";
        m_pPauseScene = new PauseScene(this);
        m_elapsedTime = 0.0f;
        //bullet enemy
        if(!pBulletEnemy)
        {
            pBulletEnemy = new Bullet(this, "Bullet.obj");
            pBulletEnemy->setOwner(ELayers::ENEMY_LAYER);
            pBulletEnemy->setLayer(ELayers::BULLET_ENEMY_LAYER);
        }

        if(!m_pEnemy)
        {
            m_pEnemy = new EnemyShip(this, "Enemy.obj");
        }

        if(!m_pAsteroid)
        {
            m_pAsteroid = new Asteroid(this, "Asteroid_LowPoly.obj");
        }
        
        if(m_asteroidDebug)
        {
            m_pAsteroid->Init(Vector3(0.f, 0.f, -30.f)); 
            addSceneComponent(m_pAsteroid);
        }

        m_pHUDLayout = new UILayout();
        addSceneComponent(m_pHUDLayout);

        //Text
        TextMaterial* pScoreMat = MaterialManager::createMaterial<TextMaterial>("ScoreMat", "Orbitron-Regular");
        Text* pTextScore = new Text({0.5f, 0.0f}, {-200.f, 90.f}, {1.f, 1.f}, pScoreMat);
        Text* pTextPoints = new Text({0.5f, 0.0f}, {100.f, 90.f}, {1.f, 1.f}, pScoreMat);
        pScoreSys->pTextPoints = pTextPoints;
        pTextScore->setString("SCORE: ");
        pTextPoints->setString("0");

        m_pHUDLayout->addText(pTextScore);
        m_pHUDLayout->addText(pTextPoints);
        
        ResetHealthIcons();
    }

    void SpaceScene::ResetHealthIcons()
    {
        while(!healthIcons.empty())
        {
            UIBase* icon = healthIcons.top();
            healthIcons.pop();
            if(m_pHUDLayout) m_pHUDLayout->removeUIElement(icon);
        }

        UIMaterial* iconMat = MaterialManager::createMaterial<UIMaterial>("HealthIcon");
        if (iconMat->getTexture("ui_tex") == nullptr)
        {
            Texture* pTex = TextureManager::load(TEXTURES_PATH"HUD/Health.png");
            iconMat->addTexture("ui_tex", pTex);
        }

        UIBase* healthIcon1 = new UIBase({0.f, 0.f}, {150.f, 76.f}, iconMat);
        UIBase* healthIcon2 = new UIBase({0.f, 0.f}, {199.f, 76.f}, iconMat);
        UIBase* healthIcon3 = new UIBase({0.f, 0.f}, {248.f, 76.f}, iconMat);

        if(m_pHUDLayout) {
            m_pHUDLayout->addUIElement(healthIcon1);
            m_pHUDLayout->addUIElement(healthIcon2);
            m_pHUDLayout->addUIElement(healthIcon3);
        }

        healthIcons.push(healthIcon1);
        healthIcons.push(healthIcon2);
        healthIcons.push(healthIcon3);
    }

    void SpaceScene::OnSwitch()
    {
        App::state = EAppState::RUN;
    }

    void SpaceScene::removePauseLayout(UILayout* layout) {
        auto it = std::find(m_vecUILayouts.begin(), m_vecUILayouts.end(), layout);
        if(it != m_vecUILayouts.end()) {
            m_vecUILayouts.erase(it);
        }
    }

    SpaceScene::~SpaceScene() {
        if(m_pPauseScene) delete m_pPauseScene;
    }

    void SpaceScene::TogglePause() {
        if (App::state == EAppState::RUN) {
            App::state = EAppState::PAUSE;
            if(m_pPauseScene) m_pPauseScene->Show();
            if(m_pHUDLayout) m_pHUDLayout->setActive(false);
        } else if(App::state == EAppState::PAUSE) {
            App::state = EAppState::RUN;
            m_pPauseScene->Hide();
            if(m_pHUDLayout) m_pHUDLayout->setActive(true);
        }
    }

    void SpaceScene::UpdateScene(float dt)
    {
        //to fix there is the command pattern
        if (Keyboard::key(SPACE_ENGINE_KEY_BUTTON_ESCAPE)) {
            if (!m_escProcessed) {
                TogglePause();
                m_escProcessed = true; 
            }
        } else {
            m_escProcessed = false; 
        }

        if (App::state == EAppState::PAUSE) {
            m_pPauseScene->Update();
            return; 
        }
        m_elapsedTime +=dt;
        if (App::state == EAppState::RUN) {
            SpaceScene::handleSpawning(dt);
        }
    }

    void SpaceScene::ResetGame()
    {
        for (auto* obj : gameObjects)
        {
            if (dynamic_cast<Asteroid*>(obj) || dynamic_cast<EnemyShip*>(obj))
            {
                requestDestroy(obj); 
            }
        }
        
        m_asteroidTimer = 0.0f;
        m_enemyTimer = 0.0f;

        if (m_pPlayer )m_pPlayer->Reset();
        if(pScoreSys) pScoreSys->Reset(); 
        ResetHealthIcons();
        
        SPACE_ENGINE_INFO("Game Reset Complete");
    }

    uint32_t SpaceScene::GetCurrentScore()
    {
        if(pScoreSys) return pScoreSys->GetScore();
        return 0;
    }

    float SpaceScene::randomRange(float min, float max) 
    {
        return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
    }

    void SpaceScene::handleSpawning(float dt)
    {
        m_asteroidTimer += dt;
        m_enemyTimer += dt;

        // --- SPAWN ASTEROIDI ---
        if (!m_asteroidDebug && m_asteroidTimer >= m_asteroidInterval)
        {
            // Reset
            m_asteroidTimer = 0.0f;

            //imposta un margine per evitare spawn troppo ai bordi
            float objectMargin = 3.0f; 

            // Calcola i limiti di spawn
            float safeX = m_gameAreaX - objectMargin;
            float safeY = m_gameAreaY - objectMargin;

            // Genera posizione random
            float x = randomRange(-safeX, safeX);
            float y = randomRange(-safeY, safeY);
            float z = m_spawnZ;

            Asteroid* m_pTmpAst = requestInstantiate(m_pAsteroid); 
            
            m_pTmpAst->Init(Vector3(x, y, z)); 
            m_pTmpAst->getTransform()->setWorldPosition(Vector3(x, y, z));
            
            // per avere asteroidi di diverse dimensioni
            float randomScale = randomRange(1.0f, 2.5f);
            m_pTmpAst->getTransform()->setLocalScale(Vector3(randomScale));
        }

        // --- SPAWN NEMICI ---
        if (m_enemyTimer >= m_enemyInterval)
        {
            m_enemyTimer = 0.0f;

            float objectMargin = 2.0f; 
            float safeX = m_gameAreaX - objectMargin;
            float safeY = m_gameAreaY - objectMargin; 

            float x = randomRange(-safeX, safeX);
            float y = randomRange(-safeY, safeY);
            float z = m_spawnZ;

            EnemyType typeToSpawn = EnemyType::NORMAL;

            if (m_elapsedTime < 10.0f) 
            {
                //primi 10 secondi: solo NORMAL
                typeToSpawn = EnemyType::NORMAL;
            }
            else if (m_elapsedTime < 25.0f) 
            {
                //tra 10 e 20 secondi: random tra NORMAL e SPREAD
                typeToSpawn = (rand() % 2 == 0) ? EnemyType::NORMAL : EnemyType::SPREAD;
            }
            else 
            {
                //dopo 20 secondi: random tra SPREAD e AIMER
                typeToSpawn = (rand() % 2 == 0) ? EnemyType::SPREAD : EnemyType::AIMER;
            }
            
            EnemyShip* pTmpE = requestInstantiate(m_pEnemy); 
            pTmpE->Init(Vector3(x, y, z), typeToSpawn, m_pPlayer);
        }
    }

    void SpaceScene::removeHealthIcon()
    {
        if(healthIcons.empty())
        {
            SPACE_ENGINE_ERROR("Icons Stack is empty: you cannot remove icon");
            return;
        }

        UIBase* pIcon2rm = healthIcons.top(); 
        healthIcons.pop();

        for(UILayout* pLayout : m_vecUILayouts)
        {
            if(pLayout->removeUIElement(pIcon2rm))
                return;
        }
    }

    //-----------------------------------------------------//
    //--------------------SpawnerSys-----------------------//
    //-----------------------------------------------------//

    SpawnerSys::Stage SpawnerSys::m_lookupStages[] =
    {
        {
            .eStage{SPAWN_ASTEROID_MED},
            .minSpawn{1},
            .maxSpawn{2},
            .budget{BudgetAsteroridM},
            .weights{0.3f, 0.7f, 0.f},
            .spawnInterval{TimeAsterorid * TimeAsteroridXM}
        },
        {
            .eStage{SPAWN_ASTEROID_HARD},
            .minSpawn{1},
            .maxSpawn{3},
            .budget{BudgetAsteroridH},
            .weights{0.1f, 0.3f, 0.6f},
            .spawnInterval{TimeAsterorid * TimeAsteroridXH}
        },
        {
            .eStage{SPAWN_ENEMY_EASY},
            .minSpawn{1},
            .maxSpawn{1},
            .budget{BudgetEnemyE},
            .weights{1.f, 0.f, 0.f},
            .spawnInterval{TimeEnemy}
        },
        {
            .eStage{SPAWN_ENEMY_MED},
            .minSpawn{minSpawn = 1},
            .maxSpawn{maxSpawn = 2},
            .budget{budget = BudgetEnemyM},
            .weights{0.3f, 0.7f, 0.f},
            .spawnInterval{spawnInterval = TimeEnemy * TimeEnemyXM}
        },
        {
            .eStage{SPAWN_ENEMY_HARD},
            .minSpawn{1},
            .maxSpawn{3},
            .budget{BudgetEnemyM},
            .weights{0.2f,0.7f,0.1f},
            .spawnInterval{TimeEnemy * TimeEnemyXH}
        }
    };


    SpawnerSys::SpawnerSys()
    {
        SpawnerObs* m_pSpawnerObs = new SpawnerObs();
    }

    void SpawnerSys::handlerSpawn()
    {
        switch(m_stage.eStage)
        {
            case SPAWN_ASTEROID_EASY:
                if(m_stage.budget == 0)
                {
                    m_stage.eStage = SPAWN_ASTEROID_MED;
                    m_stage.minSpawn = 1; 
                    m_stage.maxSpawn = 2;
                    m_stage.budget = BudgetAsteroridM;
                    m_stage.weights[0] = 0.3f;
                    m_stage.weights[1] = 0.7f;
                    m_stage.weights[2] = 0.f;
                    m_stage.spawnInterval = TimeAsterorid * TimeAsteroridXM; 
                }
                break;
            case SPAWN_ASTEROID_MED:
                if(m_stage.budget == 0)
                {
                    m_stage.eStage = SPAWN_ASTEROID_HARD;
                    m_stage.minSpawn = 1; 
                    m_stage.maxSpawn = 3;
                    m_stage.budget = BudgetAsteroridH;
                    m_stage.weights[0] = 0.1f;
                    m_stage.weights[1] = 0.3f;
                    m_stage.weights[2] = 0.6f;
                    m_stage.spawnInterval = TimeAsterorid * TimeAsteroridXH; 
                }
                break;
            case SPAWN_ASTEROID_HARD:
                if(m_stage.budget == 0)
                {
                    m_stage.eStage = SPAWN_ENEMY_EASY;
                    m_stage.minSpawn = 1; 
                    m_stage.maxSpawn = 1;
                    m_stage.budget = BudgetEnemyE;
                    m_stage.weights[0] = 1.f;
                    m_stage.weights[1] = 0.f;
                    m_stage.weights[2] = 0.f;
                    m_stage.spawnInterval = TimeEnemy; 
                    clearSpace(); 
                }

                break;
            case SPAWN_ENEMY_EASY:
                if(m_stage.budget == 0)
                {
                    m_state = SPAWN_ENEMY_MED; 
                    m_stage.minSpawn = 1; 
                    m_stage.maxSpawn = 2;
                    m_stage.budget = BudgetEnemyM;
                    m_stage.weights[0] = 0.3f;
                    m_stage.weights[1] = 0.7f;
                    m_stage.weights[2] = 0.f;
                    m_stage.spawnInterval = TimeEnemy * TimeEnemyXM;
                }
                break;
            case SPAWN_ENEMY_MED:
                if(m_stage.budget == 0)
                {
                    m_state = SPAWN_ENEMY_HARD; 
                    m_stage.minSpawn = 1; 
                    m_stage.maxSpawn = 3;
                    m_stage.budget = BudgetEnemyM;
                    m_stage.weights[0] = 0.2f;
                    m_stage.weights[1] = 0.7f;
                    m_stage.weights[2] = 0.1f;
                    m_stage.spawnInterval = TimeEnemy * TimeEnemyXH;
                }
                break;
            case SPAWN_ENEMY_HARD:
                if(m_stage.budget == 0)
                {
                    m_state = SPAWN_MIX; 
                }
                break;
        }

        spawnLogic();
    }

    void SpawnerSys::clearSpace()
    {
        for(int i = 0; i < SpawnerObs::SlotDim; i++)
            m_pSpawnerObs->space[i] = ESlot::FREE;
    }

    int SpawnerSys::weightedRandom(float* weight, int dim)
    {
        float rand =static_cast<float>(PRNG::getNumber()) / 0xFFFF'FFFF; 
        float comulative = 0.f;

        for(int i = 0; i < dim; i++)
        {
            comulative += weight[i];
            if(comulative >= rand)
                return i; 
        }

        return dim-1;
    }

    
    void SpawnerSys::getAvailableSlot(int* available, int& dim)
    {
        dim = 0;    
        for(int i = 0; i < SpawnerObs::SlotDim; i++)
            if(m_pSpawnerObs->space[i] = ESlot::FREE)
                available[dim++] = i;
    }

    int SpawnerSys::pickSlot(int prev, int index, int spawnCount)
    {
        if(spawnCount == SpawnerObs::SlotDim)
            return  index;
        
        return (prev + 1 + PRNG::getNumber() % (SpawnerObs::SlotDim - 1)) % SpawnerObs::SlotDim; 
    }

    uint32_t SpawnerSys::spawnEntities(Stage& stage, uint32_t spawnCount)
    {
        if(stage.eStage >= ESpawnState::SPAWN_ASTEROID_EASY && stage.eStage <= ESpawnState::SPAWN_ASTEROID_HARD)
        {

            for(int i = 0, prev = -1; i < spawnCount; i++)
            {
                int index = pickSlot(prev, index, spawnCount);
                Asteroid* pAsteroid = m_pScene->requestInstantiate(SpaceScene::m_pAsteroid);
                pAsteroid->Init(Vector3{getPosX(index), 0.f, -100.f}, index);                       
                m_pSpawnerObs->space[index] = ESlot::ASTEROID;
                prev = index;
            }
            return spawnCount;
        }

        int availableSlots[3] = {-1,-1,-1};
        int dim = 0;
        
        getAvailableSlot(availableSlots, dim);

        if(stage.eStage >= ESpawnState::SPAWN_ENEMY_EASY && stage.eStage <= ESpawnState::SPAWN_ENEMY_HARD)
        {
            uint32_t nSpawned = 0;
            int prev = -1;
            
            for(int i = 0, prev = -1; i < dim; i++, nSpawned++)
            {
                int index = pickSlot(prev, index, spawnCount);
                EnemyType enemyType = static_cast<EnemyType>(weightedRandom(m_stage.weights, 3));
                EnemyShip* pEnemy = m_pScene->requestInstantiate(SpaceScene::m_pEnemy);

                pEnemy->Init(Vector3{getPosX(index), 0.f, -100.f}, enemyType, SpaceScene::m_pPlayer, index);
                m_pSpawnerObs->space[index] = ESlot::ENEMY;
            }

            return nSpawned;
        }

        else if(stage.eStage == ESpawnState::SPAWN_MIX)
        {

        }

        return 0;
    }

    void SpawnerSys::spawnLogic()
    {
        //remeber to update timer
        if(m_timer < m_stage.spawnInterval)
            return;
            
        uint32_t spawnCount = weightedRandom(m_stage.weights, 3);
        spawnCount = std::min(spawnCount, m_stage.budget);
        
        uint32_t nSpawn = spawnEntities(m_stage, spawnCount);
        
        if(nSpawn)
        {
            m_timer = 0.f;
            m_stage.budget -= spawnCount;
        }
        
            
    }

    //-----------------------------------------------------//
    //--------------------SpawnerObs-----------------------//
    //-----------------------------------------------------//

    void SpawnerObs::onNotify(const GameObject& entity, const int& event)
    {
        
        if(const EnemyShip* pEnemy = dynamic_cast<const EnemyShip*>(&entity))
        {
            space[event] = SpawnerSys::ESlot::FREE;
        }
        else if(const Asteroid* pAsteroid = dynamic_cast<const Asteroid*>(&entity))
        {
            space[event] = SpawnerSys::ESlot::FREE;
        }
        else
        {
            SPACE_ENGINE_FATAL("SpawnerObs: entity doesn't handle");
        }
    }

    //-----------------------------------------------------//
    //------------------SpawnerSubject---------------------//
    //-----------------------------------------------------//

    SpawnerSubject::SpawnerSubject(int ticket)
    {
        addObserver(SpaceScene::pSpawnerSys->getObserver());
        m_ticket = ticket;
    }

    void SpawnerSubject::notifyDestroy(GameObject& pGameObj)
    {
        notify(pGameObj, m_ticket);
    }

    //-----------------------------------------------------//
    //---------------------ScoreSys------------------------//
    //-----------------------------------------------------//

    void ScoreSys::onNotify(const GameObject& entity, const int& event)
    {
        if(const PlayerShip* pPlayer = dynamic_cast<const PlayerShip*>(&entity))
        {
            //m_score += static_cast<uint32_t>(event);
            //pTextPoints->setString(std::to_string(m_score));
        }
        else if(const EnemyShip* pEnemy = dynamic_cast<const EnemyShip*>(&entity))
        {
            m_score += static_cast<uint32_t>(event); 
            pTextPoints->setString(std::to_string(m_score));
        }
        else if(const Asteroid* pAsteroid = dynamic_cast<const Asteroid*>(&entity))
        {
            m_score += static_cast<uint32_t>(event); 
            pTextPoints->setString(std::to_string(m_score));
        }
    }

    void ScoreSys::Reset()
    {
        m_score = 0;
        if(pTextPoints) {
            pTextPoints->setString("0");
        }
    }

    //-----------------------------------------------------//
    //-------------------PointSubject----------------------//
    //-----------------------------------------------------//

    PointSubject::PointSubject()
    {
        addObserver(SpaceScene::pScoreSys);
    }

    void PointSubject::notifyPoints(GameObject& pGameObj, int score)
    {
        notify(pGameObj, score);
    }
}
