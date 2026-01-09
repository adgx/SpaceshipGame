#include "scene.h"
#include <glad/gl.h>
#include "log.h"
#include "font.h"
#include "Asteroid.h"
#include "EnemyShip.h"
#include "PlayerShip.h"

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
        
        for (auto* obj : gameObjects)
            obj->update(dt);

        UpdateScene(dt);
        // cleanup gameobjects phase
        processInstantiateQ(dt);
        processDestroyQ();
        
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
    SpaceScene::SpaceScene(PhysicsManager* pPhyManager):
    Scene(pPhyManager)
    {
        name = "SpaceScene";
        m_elapsedTime = 0.0f;
        UIMaterial* iconMat = MaterialManager::createMaterial<UIMaterial>("HealthIcon");
        Texture* pTex = TextureManager::load(TEXTURES_PATH"HUD/Health.png");
        iconMat->addTexture("ui_tex", pTex);
        UIBase* healthIcon1 = new UIBase({0.f, 0.f}, 
            {150.f, 76.f},
            iconMat);
        //added 0.035*2
        UIBase* healthIcon2 = new UIBase({0.f, 0.f}, 
            {199.f, 76.f},
            iconMat);
        //added 0.035*3
        UIBase* healthIcon3 = new UIBase({0.f, 0.f},
            {248.f, 76.f}, 
            iconMat);

        //Text
        TextMaterial* pScoreMat = MaterialManager::createMaterial<TextMaterial>("ScoreMat", "Orbitron-Regular");
        Text* pTextScore = new Text({0.5f, 0.0f}, {-200.f, 90.f}, {1.f, 1.f}, pScoreMat);
        pTextScore->setString("SCORE: ");
        UILayout* pUILayout = new UILayout();
        addSceneComponent(pUILayout);
        pUILayout->addText(pTextScore);
        pUILayout->addUIElement(healthIcon1);
        pUILayout->addUIElement(healthIcon2);
        pUILayout->addUIElement(healthIcon3);
        healthIcons.push(healthIcon1);
        healthIcons.push(healthIcon2);
        healthIcons.push(healthIcon3);
    }

    void SpaceScene::UpdateScene(float dt)
    {
        m_elapsedTime +=dt;
        SpaceScene::handleSpawning(dt);
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
        if (m_asteroidTimer >= m_asteroidInterval)
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

            Asteroid* pAsteroid = new Asteroid(this, "Asteroid_LowPoly.obj");
            
            pAsteroid->Init(Vector3(x, y, z)); 
            pAsteroid->getTransform()->setWorldPosition(Vector3(x, y, z));
            
            // per avere asteroidi di diverse dimensioni
            float randomScale = randomRange(1.0f, 2.5f);
            pAsteroid->getTransform()->setLocalScale(Vector3(randomScale));

            addSceneComponent(pAsteroid);
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

            EnemyShip* pEnemy = new EnemyShip(this, "Enemy.obj");
            
            pEnemy->Init(Vector3(x, y, z), typeToSpawn, m_pPlayer);
            requestInstantiate(pEnemy); 
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

    void ScoreSys::onNotify(const GameObject& entity, const int& event)
    {
        if(const PlayerShip* pPlayer = dynamic_cast<const PlayerShip*>(&entity))
        {
            m_score += static_cast<uint32_t>(event); 
        }
        else if(const EnemyShip* pEnemy = dynamic_cast<const EnemyShip*>(&entity))
        {
            m_score += static_cast<uint32_t>(event); 
        }
    }
}
