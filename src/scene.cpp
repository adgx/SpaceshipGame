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

        m_powerupInterval = 5.0f; 
        m_powerupTimer = 0.0f;
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

        if (m_powerupTimer >= m_powerupInterval)
        {
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
