#pragma once

#include "mesh.h"
#include "utils/utils.h"
#include "material.h"
#include "camera.h"
#include "skybox.h"
#include "ui.h"
#include "light.h"
#include "font.h"
#include "texture.h"

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

    struct ScreenRenderObject
    {
        PlaneMesh* pPlaneMesh = nullptr;
        BaseMaterial* pMaterial = nullptr;
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

    class RenderBuffer
    {
        public:
            RenderBuffer(GLenum target, GLenum attachment);
            void init(GLenum target, GLenum attachment);
        private:
            GLenum m_renderBufferObj = 0;
            GLenum m_target = 0;
            GLenum m_attachment = 0;
    };

    class FrameBuffer
    {
    public:
        FrameBuffer();
        ~FrameBuffer(); 
        
        void init();
        void addColorBuffer();
        void addRenderBuffer();
        void drawBuffers();

        inline int bindFrameBuffer()
        {
            if(m_frameBufferObj)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferObj);
                return 0;
            }
            
            return -1;
        }

        inline void unbindFrameBuffer(){glBindFramebuffer(GL_FRAMEBUFFER, 0);}
        inline int getColorBuffer(uint32_t index)
        {
            if(index < m_vecColorBuffers.size())
            {
                return m_vecColorBuffers[index]->getTexture();
            }

            return -1;
        }
    private:
        GLenum m_frameBufferObj = 0;
        std::vector<Texture*> m_vecColorBuffers;
        RenderBuffer* m_pRenderBuffer;
    };



    class RendererV2
    {
        public:
            RendererV2() = default;
            void Initialize();
            void Shutdown();
            static void clear();
            static void render(const std::vector<ScreenRenderObject>& screenRenderables); //screen shaders renderer
            static void render(const RendererParams& rParams); //mesh renderer
            static void render(const std::vector<UIRenderObject>& uiRenderables); //UI renderer
            static void render(const std::vector<TextRenderObject>& textRenderables); //Text renderer
            static void postprocessing(bool bloomVFX);

        private:
            static bool m_preprocessing;
            static bool m_bloomVFX;
            static bool m_debug;
            static FrameBuffer m_HDRFrameBuffer;
            static FrameBuffer m_BloomFrameBuffers[2];
            static ShaderProgram* m_pHDRShader;
            static ShaderProgram* m_pBloomShader;
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

    class ScreenRenderer
    {
        public:
            void render(const std::vector<ScreenRenderObject>& screenRenderables);
    };
}
