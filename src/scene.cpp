#include "scene.h"
#include <glad/gl.h>
#include "log.h"

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
                        bool flag; 
                        if( flag = toDestroy.count(pGameObj) != 0; flag)
                            pPhyManager->RemoveCollider(pGameObj->getComponent<Collider>());
                        return flag; 
                    }),
                gameObjects.end()
            );
    }

    void Scene::gatherRenderables(std::vector<RenderObject>& worldRenderables, std::vector<UIRenderObject>& uiRenderables)
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
        UILayout* pUILayout = new UILayout();
        addSceneComponent(pUILayout);
        pUILayout->addUIElement(healthIcon1);
        pUILayout->addUIElement(healthIcon2);
        pUILayout->addUIElement(healthIcon3);
        healthIcons.push(healthIcon1);
        healthIcons.push(healthIcon2);
        healthIcons.push(healthIcon3);
    }

    void SpaceScene::UpdateScene(float dt)
    {
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
            
            pAsteroid->Init(); 
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

            EnemyShip* pEnemy = new EnemyShip(this, "TestCube.obj");
            
            pEnemy->Init(Vector3(x, y, z), EnemyType::NORMAL, nullptr); // FIXME: Target null per ora

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

}
