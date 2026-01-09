#pragma once

#include <queue>
#include <vector>

#include "../scene.h"
#include "../camera.h"
#include "../Light.h"

namespace SpaceEngine
{
    class SceneManager
    {
        public:
            SceneManager() = default;
            ~SceneManager() = default;
            void Initialize();
            void Shutdown();
            void Update(float dt);
            void LateUpdate();
            void GatherRenderables(std::vector<RenderObject>& worldRenderables, 
                std::vector<UIRenderObject>& uiRenderables, 
                std::vector<TextRenderObject>& textRenderables);
            BaseCamera* GetActiveCamera();
            std::vector<Light*>* GetLights();
            Skybox* GetSkybox();
            static Scene* GetActiveScene();
            static int ActiveScene(const std::string& nameScene, bool flag);
            static void LoadScene(Scene* pScene);
            static void UnloadScene(const std::string& nameScene);
            static void SwitchScene(const std::string & name);
            
            


        private:
            void ProcessUnload();
            static std::vector<Scene*> m_vecScenes;
            static Scene* m_currScene;
            static std::queue<Scene*> m_pendingUnloadQ;
    };
}