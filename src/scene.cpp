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

        // cleanup gameobjects phase
        processInstantiateQ(dt);
        processDestroyQ();
        
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
            pPhyManager->RemoveCollider(destroyQ.front()->getComponent<Collider>());
            toDestroy.insert(destroyQ.front());
            destroyQ.pop();
        }

        if(!toDestroy.empty())
            gameObjects.erase(
                std::remove_if(gameObjects.begin(), gameObjects.end(),
                    [&](GameObject* pGameObj){ return toDestroy.count(pGameObj) != 0; }),
                gameObjects.end()
            );
    }

    void Scene::requestInstatiate(GameObject* pGameObj)
    {
        SpawnRequest sr;
        sr.prefab = pGameObj;
        spawnQ.push_back(sr);
    }

    void Scene::requestInstatiate(GameObject* pGameObj, float time)
    {
        SpawnRequest sr;
        sr.prefab = pGameObj;
        sr.timeRemaining = time;
        spawnQ.push_back(sr);
    }

    void Scene::requestInstatiate(GameObject* pGameObj, Vector3 wPos)
    {
        SpawnRequest sr;
        sr.prefab = pGameObj;
        sr.overrideWorldPos = true;
        sr.wPos = wPos;
        spawnQ.push_back(sr);
    }
    
    void Scene::requestInstatiate(GameObject* pGameObj, float time, Vector3 wPos)
    {
        SpawnRequest sr;
        sr.prefab = pGameObj;
        sr.overrideWorldPos = true;
        sr.wPos = wPos;
        sr.timeRemaining = time;
        spawnQ.push_back(sr);
    }

    GameObject* Scene::instatiate(const SpawnRequest& sr)
    {
        GameObject* pCopy =  new GameObject(*sr.prefab);
        if(pCopy)
        {
            if(sr.overrideWorldPos)
                pCopy->getComponent<Transform>()->setWorldPosition(sr.wPos);
            if(Collider* pCol = pCopy->getComponent<Collider>(); pCol != nullptr)
                pPhyManager->AddCollider(pCol);
        }
        else
        {
            SPACE_ENGINE_FATAL("Inable to copy and instatiate the GameObject");
        }

        return pCopy;

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
}
