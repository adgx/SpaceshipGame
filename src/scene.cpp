#include "scene.h"
#include <glad/gl.h>
#include "log.h"

namespace SpaceEngine{

    BaseCamera* Scene::getActiveCamera()
    {
        return cameras[0];
    }

    void Scene::Update(float dt)
    {
        for (auto* obj : gameObjects)
            obj->update(dt);

        // cleanup gameobjects phase
        processDestroyQ();
        
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
                    [&](GameObject* pGameObj){ return toDestroy.count(pGameObj) != 0; }),
                gameObjects.end()
            );
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
                    renderObj.instances = gameObj->getNumInstances();
                    std::vector<Transform*>* vecTrasf = gameObj->getComponent<std::vector<Transform*>>();
                    for(int j = 0; j < renderObj.instances; j++)
                        renderObj.modelMatrix.push_back((*vecTrasf)[j]->getWorldMatrix());
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

    unsigned int Scene::LoadCubemap(std::vector<std::string> faces){
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                // Sommando 'i', accediamo a destra, sinistra, sopra
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                            0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
                );
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

        return textureID;
    }
}
