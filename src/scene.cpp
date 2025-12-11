#include "scene.h"
#include <glad/gl.h>
#include "log.h"

namespace SpaceEngine{

    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
    };

    BaseCamera* Scene::getActiveCamera()
    {
        return cameras[0];
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

    void Scene::initSkybox(std::vector<std::string> faces) {
        // per caricare lo shader
        skyboxShader = ShaderManager::findShaderProgram("skybox");
        if (!skyboxShader) {
            SPACE_ENGINE_ERROR("Skybox shader mancante!");
            return;
        }

        // Setup del cubo geometrico (VAO/VBO)
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        
        cubemapTextureID = LoadCubemap(faces); 

        if(skyboxShader){
            skyboxShader->use();
            skyboxShader->setUniform("skybox", 0);
        }
    }

    void Scene::drawSkybox(const glm::mat4& view, const glm::mat4& projection) {
        if (skyboxVAO == 0 || !skyboxShader) return;

        // per disegnare solo il cielo alla massima profondità
        glDepthFunc(GL_LEQUAL);  
        
        skyboxShader->use();
        
        // per non fare avvicinare il cielo
        glm::mat4 viewNoTrans = glm::mat4(glm::mat3(view)); 
        
        skyboxShader->setUniform("view", viewNoTrans);
        skyboxShader->setUniform("projection", projection);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);
        
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        glBindVertexArray(0);
        
        // Ripristina la profondità normale per il resto del gioco
        glDepthFunc(GL_LESS); 
    }

    unsigned int Scene::LoadCubemap(const std::vector<std::string>& faces){
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        stbi_set_flip_vertically_on_load(false);

        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 3);
            if (data)
            {
                // Sommando 'i', accediamo a destra, sinistra, sopra
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                            0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            }
            else
            {
                SPACE_ENGINE_ERROR("Cubemap texture failed to load at path: {}", faces[i]);
                stbi_image_free(data);
            }
        }
        
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        stbi_set_flip_vertically_on_load(true);

        return textureID;
    }
}
