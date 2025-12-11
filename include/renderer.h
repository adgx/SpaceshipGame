#pragma once

#include "mesh.h"
#include "utils/utils.h"
#include "material.h"
#include "camera.h"
#include "ui.h"

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

    class Renderer
    {
        public:
            void render(const std::vector<RenderObject>& renderables, BaseCamera& cam);
        private:
            bool debug;
    };

    class UIRenderer
    {
        void render(const std::vector<UIRenderObject>& uiRenderables);
    };
}
