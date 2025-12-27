#include "scene.h"
#include <glad/gl.h>
#include "log.h"

namespace SpaceEngine
{

    BaseCamera* Scene::getActiveCamera()
    {
        return cameras[0];
    }

    void Scene::Init()
    {
        pSkybox = new Skybox();
    }

    void Scene::Update(float dt)
    {
        for (auto* obj : gameObjects)
            obj->update(dt);

        handleSpawning(dt);
        // cleanup gameobjects phase
        processInstantiateQ(dt);
        processDestroyQ();
        
    }

    float Scene::randomRange(float min, float max) {
        return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
    }
    void Scene::handleSpawning(float dt)
    {
        m_asteroidTimer += dt;
        m_enemyTimer += dt;

        // --- SPAWN ASTEROIDI ---
        if (m_asteroidTimer >= m_asteroidInterval)
        {
            // Reset
            m_asteroidTimer = 0.0f;

            float viewWidth = 12.0f;  
            float viewHeight = 8.0f; 

            // Generiamo X e Y dentro lo schermo del player
            float x = randomRange(-viewWidth, viewWidth);
            float y = randomRange(-viewHeight, viewHeight);
            float z = m_spawnZ;       
           
            Asteroid* pAsteroid = new Asteroid(this, "TestCube.obj");
            
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
            SPACE_ENGINE_INFO("Tentativo di spawn nemico! Timer reset.");
            m_enemyTimer = 0.0f;

            float x = randomRange(-20.0f, 20.0f);
            float y = randomRange(-12.0f, 12.0f);
            float z = m_spawnZ;

            EnemyShip* pEnemy = new EnemyShip(this, "TestCube.obj");
            
            pEnemy->Init(Vector3(x, y, z), EnemyType::NORMAL, nullptr); // FIXME: Target null per ora

            addSceneComponent(pEnemy);
        }
    }

    void Scene::processInstantiateQ(float dt)
    {
        for(auto it = spawnQ.begin(); it != spawnQ.end();)
        {
            it->timeRemaining -= dt;

            if(it->timeRemaining <= 0.f)
            {
                gameObjects.push_back(instatiate(*it));
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

    void Scene::requestInstatiate(GameObject* pGameObj)
    {
        SpawnRequest sr = {};
        sr.prefab = pGameObj;
        spawnQ.push_back(sr);
    }

    void Scene::requestInstatiate(GameObject* pGameObj, float time)
    {
        SpawnRequest sr = {};
        sr.prefab = pGameObj;
        sr.timeRemaining = time;
        spawnQ.push_back(sr);
    }

    void Scene::requestInstatiate(GameObject* pGameObj, Vector3 wPos)
    {
        SpawnRequest sr = {};
        sr.prefab = pGameObj;
        sr.overrideWorldPos = true;
        sr.wPos = wPos;
        spawnQ.push_back(sr);
    }
    
    void Scene::requestInstatiate(GameObject* pGameObj, float time, Vector3 wPos)
    {
        SpawnRequest sr = {};
        sr.prefab = pGameObj;
        sr.overrideWorldPos = true;
        sr.wPos = wPos;
        sr.timeRemaining = time;
        spawnQ.push_back(sr);
    }

    GameObject* Scene::instatiate(const SpawnRequest& sr)
    {
        GameObject* pObj = sr.prefab;
        if(pObj)
        {
            if(sr.overrideWorldPos)
                pObj->getComponent<Transform>()->setWorldPosition(sr.wPos);
            if(Collider* pCol = pObj->getComponent<Collider>(); pCol != nullptr)
                pPhyManager->AddCollider(pCol);
        }
        else
        {
            SPACE_ENGINE_FATAL("Inable to copy and instatiate the GameObject");
        }

        return pObj;

    }

    Skybox* Scene::getSkybox()
    {
        return pSkybox;
    }

    void Scene::gatherRenderables(std::vector<RenderObject>& worldRenderables, std::vector<UIRenderObject>& uiRenderables)
    {
        worldRenderables.clear();
        uiRenderables.clear();

        for (auto& gameObj : gameObjects)
        {
            // --- World objects ---
            
            if (Mesh* mesh = gameObj->getComponent<Mesh>(); mesh)
            {
                
                for(int i = 0, nSubMeshes = mesh->getNumSubMesh(); i < nSubMeshes; ++i)
                {
                    RenderObject renderObj;
                    renderObj.mesh = mesh;
                    //renderObj.instances = gameObj->getNumInstances();
                    //std::vector<Transform*>* vecTrasf = gameObj->getComponent<std::vector<Transform*>>();
                    Transform* trasf = gameObj->getComponent<Transform>();
                    //for(int j = 0; j < renderObj.instances; j++)
                    //  renderObj.modelMatrix.push_back((*vecTrasf)[j]->getWorldMatrix());
                    //renderObj.modelMatrix.push_back(trasf->getWorldMatrix());
                    renderObj.modelMatrix = trasf->getWorldMatrix();
                    worldRenderables.push_back(renderObj);
                }
            }
            // --- UI objects ---
            if (UIMesh* ui = gameObj->getComponent<UIMesh>(); ui)
            {
                UIRenderObject uro;
                //uro.mesh = ui->mesh;
                //uro.material = ui->material;
                //uro.modelMatrix = go->transform.GetMatrix();
                uiRenderables.push_back(uro);
            }
        }
    }

    std::vector<Light*>* Scene::getLights()
    {
        return &lights;
    }
}
