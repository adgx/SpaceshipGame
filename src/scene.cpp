#include "scene.h"
#include <glad/gl.h>
#include "log.h"
#include "mesh.h"
#include "font.h"
#include "Asteroid.h"
#include "EnemyShip.h"
#include "PlayerShip.h"
#include "gameObject.h"
#include "powerUp.h"
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

    void Scene::gatherRenderables(std::vector<RenderObject>& worldRenderables,
            std::vector<UIRenderObject>& uiRenderables,
            std::vector<TextRenderObject>& textRenderables,
            std::vector<ScreenRenderObject>& screenRenderables)
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

        if(m_vecScreenRendObj.size())
        {
            screenRenderables = m_vecScreenRendObj; 
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
    std::vector<Asteroid*> SpaceScene::m_asteroids; 
    PlayerShip* SpaceScene::m_pPlayer = nullptr;

    SpaceScene::SpaceScene(PhysicsManager* pPhyManager):
    Scene(pPhyManager)
    {
        name = "SpaceScene";
        m_pPauseScene = new PauseScene(this);
        m_elapsedTime = 0.0f;
        if(pSpawnerSys)
        {
            pSpawnerSys->setScene(this);
        }
        //bullet enemy
        if(!pBulletEnemy)
        {
            pBulletEnemy = new Bullet(this, "Bullet.obj");
            pBulletEnemy->setOwner(ELayers::ENEMY_LAYER);
            pBulletEnemy->setLayer(ELayers::BULLET_ENEMY_LAYER);
        }

        if(!m_pEnemy)
        {
            m_pEnemy = new EnemyShip(this, "EnemyV3.obj");
        }

        if(m_asteroids.empty())
        {
            m_asteroids.push_back(new Asteroid(this, "Asteroid_LowPoly.obj")); 
            m_asteroids.push_back(new Asteroid(this, "Asteroid02.obj"));
            m_asteroids.push_back(new Asteroid(this, "Asteroid03.obj"));
        }

        if(m_asteroidDebug && !m_asteroids.empty())
        {
            m_asteroids[0]->Init(Vector3(0.f, 0.f, -30.f), 0.f); 
            addSceneComponent(m_asteroids[0]);
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

        m_powerupInterval = 5.0f; 
        m_powerupTimer = 0.0f;

        glEnable(GL_BLEND); 
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
            pSpawnerSys->handlerSpawn(dt);
            //SpaceScene::handleSpawning(dt);
        }
    }

    void SpaceScene::ResetGame()
    {
        for (auto* obj : gameObjects)
        {
            ELayers layer = obj->getLayer();

            bool isEnemy = (layer == ELayers::ENEMY_LAYER);
            bool isAsteroid = (layer == ELayers::ASTEROID_LAYER);
            bool isPowerUp = (layer == ELayers::POWERUP_LAYER);
            
            bool isBullet = (layer == ELayers::BULLET_LAYER || 
                            layer == ELayers::BULLET_PLAYER_LAYER || 
                            layer == ELayers::BULLET_ENEMY_LAYER);

            if (isEnemy || isAsteroid || isPowerUp || isBullet)
            {
                requestDestroy(obj); 
            }
        }
        
        m_asteroidTimer = 0.0f;
        m_enemyTimer = 0.0f;
        m_powerupTimer = 0.0f;

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
        m_powerupTimer += dt;

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

            Asteroid* m_pTmpAst = requestInstantiate(m_asteroids[rand() % m_asteroids.size()]); 
            
            m_pTmpAst->Init(Vector3(x, y, z), 0.f); 
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

        // --- SPAWN POWERUP ---
        if (m_powerupTimer >= m_powerupInterval)
        {
            SPACE_ENGINE_INFO("!!!!Spawning PowerUp...");
            m_powerupTimer = 0.0f;

            float objectMargin = 2.0f; 
            float safeX = m_gameAreaX - objectMargin;
            float safeY = m_gameAreaY - objectMargin; 

            float x = randomRange(-safeX, safeX);
            float y = randomRange(-safeY, safeY);
            float z = m_spawnZ;
            //sceglie casualmente il tipo di powerup
            int PowerUprandomType = rand() % 3;

            PowerUpType type = PowerUpType::RAPID_FIRE;
            std::string modelName = "";
            std::string matName = "";
            std::string meshName = "";

            switch(PowerUprandomType) {
                case 0: 
                    type = PowerUpType::RAPID_FIRE;
                    modelName = "PowerUp/rapidFire_powerUp.png";
                    meshName = "QuadRapid.obj";
                    matName = "Mat_PowerUp_Rapid";
                    break;
                case 1: 
                    type = PowerUpType::BOMB;
                    modelName = "PowerUp/bomb_powerUp.png";
                    meshName = "QuadBomb.obj";
                    matName = "Mat_PowerUp_Nuke";
                    break;
                case 2: 
                    type = PowerUpType::HEALTH;
                    modelName = "PowerUp/Health_powerUp.png";
                    meshName = "QuadHealth.obj";
                    matName = "Mat_PowerUp_Health";
                    break;
            }

            PowerUp* pPower = new PowerUp(this, type, meshName);

            if (!pPower) {
                SPACE_ENGINE_ERROR("CRITICAL: Impossible to allocate memory for PowerUp!");
                return;
            }

            Mesh* pMesh = pPower->getComponent<Mesh>();
            if (!pMesh) {
                SPACE_ENGINE_ERROR("ERROR: Mesh '{}' NOT FOUND! PowerUp destroyed.", meshName);
                delete pPower;
                return;
            }

            PBRMaterial* pMat = MaterialManager::createMaterial<PBRMaterial>(matName);
            pMat->pShader = ShaderManager::findShaderProgram("powerup");

            if (!pMat->pShader) {
                SPACE_ENGINE_WARN("Shader 'powerup' not found! Loading/Compiling now...");
                pMat->pShader = ShaderManager::findShaderProgram("powerup");
            }
        
            if (pMat->getTexture("albedo_tex") == nullptr) {
                Texture* pTex = TextureManager::load(TEXTURES_PATH + modelName);
                if(pTex) {
                    pMat->addTexture("albedo_tex", pTex);
                    std::get<float>(pMat->props["ambient_occlusion_val"]) = 1.0f; 
                    std::get<Vector4>(pMat->props["albedo_color_val"]) = {1.f, 1.f, 1.f, 1.f};
                } else {
                     SPACE_ENGINE_ERROR("Texture NOT FOUND: {}", modelName);
                     std::get<Vector4>(pMat->props["albedo_color_val"]) = {1.f, 0.f, 0.f, 1.f};
                }
            }

            if (Mesh* pMesh = pPower->getComponent<Mesh>()) {
                pMesh->bindMaterialToSubMeshIndex(0, pMat);
            } else {
                SPACE_ENGINE_ERROR("Mesh component missing on PowerUp!");
            }
        
            pPower->Init(Vector3(x, y, z));
            addSceneComponent(pPower);
            
            SPACE_ENGINE_INFO("Spawned PowerUp Type: {}", PowerUprandomType);
        }
    }

    void SpaceScene::TriggerBomb()
    {
        SPACE_ENGINE_INFO("BOOM! Bomb triggered.");

        for (auto* obj : gameObjects)
        {
            bool isEnemy = dynamic_cast<EnemyShip*>(obj) != nullptr;
            bool isAsteroid = dynamic_cast<Asteroid*>(obj) != nullptr;
            bool isEnemyBullet = dynamic_cast<Bullet*>(obj) != nullptr && obj->getLayer() == ELayers::BULLET_ENEMY_LAYER;

            if (isEnemy || isAsteroid || isEnemyBullet)
            {
                requestDestroy(obj);
                if (pScoreSys) pScoreSys->onNotify(*obj, 50); 
            }
        }
    }

    void SpaceScene::AddHealthIcon()
    {
        if (healthIcons.size() >= 3) return;

        float startX = 150.f;
        float spacing = 49.f;
        float newX = startX + (healthIcons.size() * spacing);

        UIMaterial* iconMat = MaterialManager::createMaterial<UIMaterial>("HealthIcon");
        if (iconMat->getTexture("ui_tex") == nullptr) {
            Texture* pTex = TextureManager::load(TEXTURES_PATH"HUD/Health.png");
            iconMat->addTexture("ui_tex", pTex);
        }

        UIBase* newIcon = new UIBase({0.f, 0.f}, {newX, 76.f}, iconMat);

        if (m_pHUDLayout) {
            m_pHUDLayout->addUIElement(newIcon);
        }
        healthIcons.push(newIcon);
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
            .weights{1.f, 0.f, 0.f},
            .eStage{SPAWN_ASTEROID_EASY},
            .budget{BudgetAsteroidE},
            .minSpawn{1},
            .maxSpawn{1},
            .spawnInterval{TimeAsterorid}
        },
        {
            .weights{0.3f, 0.7f, 0.f},
            .eStage{SPAWN_ASTEROID_MED},
            .budget{BudgetAsteroidM},
            .minSpawn{1},
            .maxSpawn{2},
            .spawnInterval{TimeAsterorid * TimeAsteroridXM}
        },
        {
            .weights{0.1f, 0.3f, 0.6f},
            .eStage{SPAWN_ASTEROID_HARD},
            .budget{BudgetAsteroidH},
            .minSpawn{1},
            .maxSpawn{3},
            .spawnInterval{TimeAsterorid * TimeAsteroridXH}
        },
        {
            .weights{1.f, 0.f, 0.f},
            .eStage{SPAWN_ENEMY_EASY},
            .budget{BudgetEnemyE},
            .minSpawn{1},
            .maxSpawn{1},
            .spawnInterval{TimeEnemy}
        },
        {
            .weights{0.3f, 0.7f, 0.f},
            .eStage{SPAWN_ENEMY_MED},
            .budget{BudgetEnemyM},
            .minSpawn{1},
            .maxSpawn{2},
            .spawnInterval{TimeEnemy * TimeEnemyXM}
        },
        {
            .weights{0.2f,0.6f,0.2f},
            .eStage{SPAWN_ENEMY_HARD},
            .budget{BudgetEnemyH},
            .minSpawn{1},
            .maxSpawn{3},
            .spawnInterval{TimeEnemy * TimeEnemyXH}
        },
        {
            .weights{0.2f,0.6f,0.2f},
            .eStage{SPAWN_MIX},
            .budget{30},
            .minSpawn{1},
            .maxSpawn{3},
            .spawnInterval{TimeEnemy * TimeEnemyXH}
        }
    };


    SpawnerSys::SpawnerSys()
    {
        m_stage = m_lookupStages[ESpawnState::SPAWN_ASTEROID_EASY];
        m_pSpawnerObs = new SpawnerObs();

        m_powerupTimer = 0.0f;
        m_powerupInterval = 10.0f;
    }

    void SpawnerSys::spawnPowerUp()
    {
        float safeX = 8.0f; 
        float safeY = 4.5f; 
        float z = -100.0f;

        float x = randomRange(-safeX, safeX);
        float y = 0.f;

        int PowerUprandomType = rand() % 3;
        PowerUpType type = PowerUpType::RAPID_FIRE;
        std::string modelName = "";
        std::string matName = "";
        std::string meshName = "";

        switch(PowerUprandomType) {
            case 0: 
                type = PowerUpType::RAPID_FIRE;
                modelName = "PowerUp/rapidFire_powerUp.png";
                meshName = "QuadRapid.obj";
                matName = "Mat_PowerUp_Rapid";
                break;
            case 1: 
                type = PowerUpType::BOMB;
                modelName = "PowerUp/bomb_powerUp.png";
                meshName = "QuadBomb.obj";
                matName = "Mat_PowerUp_Nuke";
                break;
            case 2: 
                type = PowerUpType::HEALTH;
                modelName = "PowerUp/Health_powerUp.png";
                meshName = "QuadHealth.obj";
                matName = "Mat_PowerUp_Health";
                break;
        }

        if (!m_pScene) return;

        PowerUp* pPower = new PowerUp(m_pScene, type, meshName);

        PBRMaterial* pMat = MaterialManager::createMaterial<PBRMaterial>(matName);
        
        pMat->pShader = ShaderManager::findShaderProgram("powerup");
        
        if (pMat->getTexture("albedo_tex") == nullptr) {
            Texture* pTex = TextureManager::load(TEXTURES_PATH + modelName);
            if(pTex) {
                pMat->addTexture("albedo_tex", pTex);
            }
        }
        if (Mesh* pMesh = pPower->getComponent<Mesh>()) {
            pMesh->bindMaterialToSubMeshIndex(0, pMat);
        }

        pPower->Init(Vector3(x, y, z));
        
        m_pScene->addSceneComponent(pPower); 
        
        SPACE_ENGINE_INFO("SpawnerSys: Spawned PowerUp Type {}", PowerUprandomType);
    }

    void SpawnerSys::handlerSpawn(float dt)
    {
        //update timer
        m_timer += dt;

        m_powerupTimer += dt;
        if (m_powerupTimer >= m_powerupInterval)
        {
            spawnPowerUp();
            m_powerupTimer = 0.0f; 
        }

        switch(m_stage.eStage)
        {
            case SPAWN_ASTEROID_EASY:
                if(m_stage.budget == 0)
                    m_stage = m_lookupStages[SPAWN_ASTEROID_MED]; 
                break;
            case SPAWN_ASTEROID_MED:
                if(m_stage.budget == 0)
                    m_stage = m_lookupStages[SPAWN_ASTEROID_HARD];
                break;
            case SPAWN_ASTEROID_HARD:
                if(m_stage.budget == 0)
                    m_stage = m_lookupStages[SPAWN_ENEMY_EASY]; 
                    //clearSpace(); 
                break;
            case SPAWN_ENEMY_EASY:
                if(m_stage.budget == 0)
                    m_stage = m_lookupStages[SPAWN_ENEMY_MED]; 
                break;
            case SPAWN_ENEMY_MED:
                if(m_stage.budget == 0)
                    m_stage = m_lookupStages[SPAWN_ENEMY_HARD]; 
                break;
            case SPAWN_ENEMY_HARD:
                if(m_stage.budget == 0){
                    m_stage = m_lookupStages[SPAWN_MIX];
                }
                break;
            case SPAWN_MIX:
                if(m_stage.budget == 0){
                    m_stage.budget = 30;
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
            if(m_pSpawnerObs->space[i] == ESlot::FREE)
                available[dim++] = i;
    }

    int SpawnerSys::pickSlot(int prev, int index, int spawnCount)
    {
        if(spawnCount == SpawnerObs::SlotDim)
            return  index;
        
        return (prev + 1 + (PRNG::getNumber() % SpawnerObs::SlotDim)) % SpawnerObs::SlotDim; 
    }

    void SpawnerSys::spawnAsteroid(uint32_t spawnCount)
    {
        for(uint32_t i = 0, prev = -1; i < spawnCount; i++)
        {
            int index = pickSlot(prev, i, spawnCount);
            int variantIndex = rand() % SpaceScene::m_asteroids.size();
            Asteroid* pPrefab = SpaceScene::m_asteroids[variantIndex];
            Asteroid* pAsteroid = m_pScene->requestInstantiate(pPrefab);
            pAsteroid->Init(Vector3{getPosX(index), 0.f, FarDistance}, VelAsterorid/m_stage.spawnInterval, index);                       
            m_pSpawnerObs->space[index] = ESlot::ASTEROID;
            prev = index;
        }
    }

    void SpawnerSys::spawnEnemy(uint32_t spawnCount, uint32_t& nSpawned)
    {
        int availableSlots[3] = {-1,-1,-1};
        int dim = 0;
        
        getAvailableSlot(availableSlots, dim);
        
        for(int i = 0, prev = -1; i < dim && nSpawned < spawnCount; i++, nSpawned++)
        {
            int index = pickSlot(prev, i, spawnCount);
            if(m_pSpawnerObs->space[index] != ESlot::FREE)
                continue;
            EnemyType enemyType = static_cast<EnemyType>(weightedRandom(m_stage.weights, 3));
            EnemyShip* pEnemy = m_pScene->requestInstantiate(SpaceScene::m_pEnemy);

            pEnemy->Init(Vector3{getPosX(index), 0.f, -100.f}, enemyType, SpaceScene::m_pPlayer,  VelEnemy/m_stage.spawnInterval, index);
            m_pSpawnerObs->space[index] = ESlot::ENEMY;
        }
    }

    uint32_t SpawnerSys::spawnEntities(Stage& stage, uint32_t spawnCount)
    {
        if(stage.eStage >= ESpawnState::SPAWN_ASTEROID_EASY && stage.eStage <= ESpawnState::SPAWN_ASTEROID_HARD)
        {
            spawnAsteroid(spawnCount);
            
            return spawnCount;
        }

        if(stage.eStage >= ESpawnState::SPAWN_ENEMY_EASY && stage.eStage <= ESpawnState::SPAWN_ENEMY_HARD)
        {
            uint32_t nSpawned = 0;
            
            spawnEnemy(spawnCount, nSpawned);

            return nSpawned;
        }

        else if(stage.eStage == ESpawnState::SPAWN_MIX)
        {
            uint32_t nSpawned = 0;

            spawnAsteroid(spawnCount);
            spawnEnemy(spawnCount, nSpawned);
            return spawnCount;

        }

        return 0;
    }

    void SpawnerSys::spawnLogic()
    {
        //remeber to update timer
        if(m_timer < m_stage.spawnInterval)
            return;
            
        uint32_t spawnCount = weightedRandom(m_stage.weights, 3) + 1;
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
