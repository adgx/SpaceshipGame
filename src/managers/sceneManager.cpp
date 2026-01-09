#include "sceneManager.h"

namespace SpaceEngine
{
    std::vector<Scene*> SceneManager::m_vecScenes;
    std::queue<Scene*> SceneManager::m_pendingUnloadQ;
    Scene* SceneManager::m_currScene = nullptr;
    
    void SceneManager::Shutdown()
    {
        for(Scene* pScene : m_vecScenes)
        {
            delete pScene;
        }
    }

    void SceneManager::Initialize()
    {

    }

    void SceneManager::Update(float dt)
    {
        for(Scene* pScene : m_vecScenes)
        {
            if(pScene->isActive())
            {
                pScene->Update(dt);
            }
        }
    }

    void SceneManager::GatherRenderables(std::vector<RenderObject>& worldRenderables, 
                std::vector<UIRenderObject>& uiRenderables, 
                std::vector<TextRenderObject>& textRenderables)
    {
        worldRenderables.clear();
        uiRenderables.clear();
        textRenderables.clear();

        for(Scene* pScene: m_vecScenes)
        {
            if(pScene->isActive())
            {
                pScene->gatherRenderables(worldRenderables, uiRenderables, textRenderables);
            }
        }
    }

    void SceneManager::LoadScene(Scene* pScene)
    {
        if(!m_currScene)
        {
            m_currScene = pScene;
        }
        pScene->OnLoad();
        m_vecScenes.push_back(pScene);
    }

    void SceneManager::UnloadScene(const std::string& nameScene)
    {
        assert(m_currScene);
    
        if(m_currScene->getNameScene() == nameScene)
        {
            SPACE_ENGINE_ERROR("Cannot unload the current scene");
            return;
        }

        for(const Scene* pScene : m_vecScenes)
        {
            if(pScene->getNameScene() == nameScene)
            {
                SPACE_ENGINE_INFO("Scene: {} in pending to unload", nameScene)
                return;
            }
        }

        SPACE_ENGINE_ERROR("Scene not found");
    }

    void SceneManager::LateUpdate()
    {
        ProcessUnload();
    }


    void SceneManager::ProcessUnload()
    {
        const Scene* pendingScene;
        while(!m_pendingUnloadQ.empty())
        {
            pendingScene = m_pendingUnloadQ.front();
            m_pendingUnloadQ.pop();
            m_vecScenes.erase(std::remove(m_vecScenes.begin(),
                m_vecScenes.end(), 
                pendingScene), m_vecScenes.end());
        }
    }
    
    //TODO: better handle
    void SceneManager::SwitchScene(const std::string & name)
    {
        for(Scene* pScene : m_vecScenes)
        {
            if((!pScene->isActive()) && pScene->getNameScene() == name)
            {
                m_currScene->setActive(false);
                m_currScene = pScene;
                m_currScene->setActive(true);
            }
        }
    }

    BaseCamera* SceneManager::GetActiveCamera()
    {
        for(const Scene* pScene : m_vecScenes)
        {
            if(pScene->isActive()) 
            {
                if(BaseCamera* pActiveCam = pScene->getActiveCamera(); pActiveCam)
                    return pActiveCam;
            }
        }

        //SPACE_ENGINE_DEBUG("No active camera is found");
        return nullptr;
    }

    std::vector<Light*>* SceneManager::GetLights()
    {
        for(const Scene* pScene : m_vecScenes)
        {
            if(pScene->isActive()) 
            {
                if(std::vector<Light*>* pLights = pScene->getLights(); pLights)
                    return pLights;
            }
        }

        //SPACE_ENGINE_DEBUG("No lights are found");
        return nullptr;
    }

    Skybox* SceneManager::GetSkybox()
    {
        for(const Scene* pScene : m_vecScenes)
        {
            if(pScene->isActive()) 
            {
                if(Skybox* pSkybox = pScene->getSkybox(); pSkybox)
                    return pSkybox;
            }
        }

        //SPACE_ENGINE_DEBUG("No skybox is found");
        return nullptr;
    }

    int SceneManager::ActiveScene(const std::string& nameScene, bool flag)
    {
        for(Scene* pScene : m_vecScenes)
        {
            if(pScene->getNameScene() == nameScene)
            {
                pScene->setActive(flag);
                return 1;
            }
        }

        SPACE_ENGINE_ERROR("Scene: {} not found", nameScene);
        return 0;
    }

    Scene* SceneManager::GetActiveScene()
    {
        return m_currScene;
    }



}