#pragma once

#include "mesh.h"
#include "utils/utils.h"
#include "material.h"
#include "camera.h"
#include "skybox.h"
#include "ui.h"
#include "light.h"

#include <vector>

namespace SpaceEngine
{

    struct RenderObject
    {
        Matrix4 modelMatrix;
        //std::vector<Matrix4>modelMatrix;
        //int instances = 0;
        Mesh* mesh = nullptr;
    };

    struct UIRenderObject
    {
        UIMesh* pUIMesh = nullptr;
        UIMaterial* pMaterial = nullptr;
        Rect* pRect = nullptr;
    };

    struct RendererParams
    {
        const std::vector<RenderObject>& renderables; 
        const std::vector<Light*>& lights;
        const BaseCamera* cam;
        Skybox* pSkybox = nullptr; 
    };

    class Renderer
    {
        public:
            void render(const RendererParams& rParams);
        private:
            bool debug;
    }; 

    class UIRenderer
    {
        public:
            void render(const std::vector<UIRenderObject>& uiRenderables);
    };
}
