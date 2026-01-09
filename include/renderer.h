#pragma once

#include "mesh.h"
#include "utils/utils.h"
#include "material.h"
#include "camera.h"
#include "skybox.h"
#include "ui.h"
#include "light.h"
#include "font.h"

#include <vector>

namespace SpaceEngine
{

    struct RenderObject
    {
        Matrix4 modelMatrix;
        Mesh* mesh = nullptr;
    };

    struct UIRenderObject
    {
        UIMesh* pUIMesh = nullptr;
        UIMaterial* pMaterial = nullptr;
        Rect* pRect = nullptr;
    };

    struct TextRenderObject
    {
        Text* pText = nullptr;
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

    class TextRenderer
    {
        public:
            void render(const std::vector<TextRenderObject>& textRenderables);
    };
}
