#pragma once
#include "utils/stb_image.h"
#include "renderer.h"
#include "camera.h"
#include "gameObject.h"
#include "collisionDetection.h"
#include "log.h"
#include "shader.h"
#include <vector>
#include <string>
#include <queue>

using namespace std;

namespace SpaceEngine
{
    class GameObject;
    
    class Scene
    {
        public:
            Scene(PhysicsManager* pPhyManager):pPhyManager(pPhyManager){};
            ~Scene() = default;

            void Init();
            void createGameObject();

            void initSkybox(std::vector<std::string> faces); 
            void drawSkybox(const glm::mat4& view, const glm::mat4& projection);

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
            
                SPACE_ENGINE_ERROR("Component not valid!");
            }

            void gatherColliderables();
            void gatherRenderables(std::vector<RenderObject>& worldRenderables, std::vector<UIRenderObject>& uiRenderables);
            void requestDestroy(GameObject* pGameObj);
            void requestInstatiate(GameObject* pGameObj);
            void requestInstatiate(GameObject* pGameObj, float time);
            void requestInstatiate(GameObject* pGameObj, Vector3 wPos);
            void requestInstatiate(GameObject* pGameObj, float time, Vector3 wPos);
            BaseCamera* getActiveCamera();
            void Update(float dt);
        private:
            unsigned int skyboxVAO = 0;
            unsigned int skyboxVBO = 0;
            unsigned int cubemapTextureID = 0; 
            ShaderProgram* skyboxShader = nullptr;

            unsigned int LoadCubemap(const std::vector<std::string>& faces);

            PhysicsManager* pPhyManager = nullptr;
            void processDestroyQ();
            void processInstantiateQ(float dt);
            struct SpawnRequest
            {
                float timeRemaining = 0.f;
                GameObject* prefab = nullptr;
                bool overrideWorldPos = false;
                Vector3 wPos;
            };
        
            GameObject* instatiate(const SpawnRequest& sr);
            //may be is useful a table with duoble link list to keep the gameObjects
            //for avoing a complex gathering
            vector<GameObject*> gameObjects;
            std::queue<GameObject*> destroyQ;
            std::list<SpawnRequest> spawnQ;
            //cameras[0] is always the active camera
            vector<BaseCamera*> cameras;
    };
}