#pragma once
#include "utils/stb_image.h"
#include "renderer.h"
#include "camera.h"
#include "gameObject.h"
#include "log.h"
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
            Scene() = default;
            ~Scene() = default;

            unsigned int LoadCubemap(vector<string> faces);
            void Init();
            void Render();
            void createGameObject();

            template<typename T>
            void addSceneComponent(T sceneComponent)
            {
                if(sceneComponent == nullptr)
                {
                    SPACE_ENGINE_ERROR("The passed component is null");
                    return;
                }
                if constexpr (std::is_same_v<T, GameObject*>)
                {
                    gameObjects.push_back(sceneComponent);
                }
                else if constexpr (std::is_same_v<T, BaseCamera*>)
                {
                    cameras.push_back(sceneComponent);
                }
            
                SPACE_ENGINE_ERROR("Component not valid!");
            }
            void gatherRenderables(std::vector<RenderObject>& worldRenderables, std::vector<UIRenderObject>& uiRenderables);
            void requestDestroy(GameObject* pGameObj);
            BaseCamera* getActiveCamera();
            void Update(float dt);
        private:
            void processDestroyQ();
            vector<GameObject*> gameObjects;
            std::queue<GameObject*> destroyQ;
            //cameras[0] is always the active camera
            vector<BaseCamera*> cameras; 
            unsigned int cubemapTexture;
            vector<string> faces = {            //TODO: sistemare i path con i file giusti
                "assets/textures/skybox/right.jpg",
                "assets/textures/skybox/left.jpg",
                "assets/textures/skybox/top.jpg",
                "assets/textures/skybox/bottom.jpg",
                "assets/texture/skybox/front.jpg",
                //"assets/textures/skybox/back.jpg"
            };
    };
}