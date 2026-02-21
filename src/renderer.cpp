#include "renderer.h"
#include "shader.h"
#include "windowManager.h"
#include <string>

namespace SpaceEngine
{
    
    void Renderer::render(const RendererParams& rParams)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if(rParams.cam)
        {
            for(const auto& renderObj : rParams.renderables)
            {
                if(!renderObj.mesh ) continue;

                for(int idSubMesh = 0, nSubMesh = renderObj.mesh->getNumSubMesh();  idSubMesh < nSubMesh; idSubMesh++)
                {
                    GL_CHECK_ERRORS();
                    //get shader
                    ShaderProgram* shader = renderObj.mesh->getMaterialBySubMeshIndex(idSubMesh)->getShader();
                    shader->use();
                    GL_CHECK_ERRORS();
                    if(shader)
                    {
                        //bind material
                        renderObj.mesh->getMaterialBySubMeshIndex(idSubMesh)->bindingPropsToShader();
                        //set matrices
                        shader->setUniform("model", renderObj.modelMatrix);
                        shader->setUniform("view", rParams.cam->getViewMatrix());
                        shader->setUniform("projection", rParams.cam->getProjectionMatrix());
                        //lights bind
                        if(shader->isPresentUniform("lights[0].pos") && rParams.lights.size())
                        {
                            shader->setUniform("normalMatrix", Math::transpose(Math::inverse(Matrix3(renderObj.modelMatrix))));
                            GL_CHECK_ERRORS();
                            

                            for(int i = 0; i < rParams.lights.size(); i++)
                            {
                                std::string strLight = "lights[" + std::to_string(i) + "].pos"; 
                                shader->setUniform(strLight.c_str(), rParams.lights[i]->pos);
                                GL_CHECK_ERRORS();
                                strLight = "lights[" + std::to_string(i) + "].color"; 
                                shader->setUniform(strLight.c_str(), rParams.lights[i]->color);
                                GL_CHECK_ERRORS();
                                strLight = "lights[" + std::to_string(i) + "].dir"; 
                                shader->setUniform(strLight.c_str(), rParams.lights[i]->dir);
                                GL_CHECK_ERRORS();
                                strLight = "lights[" + std::to_string(i) + "].type"; 
                                shader->setUniform(strLight.c_str(), rParams.lights[i]->type);
                                GL_CHECK_ERRORS();
                            }
                        }

                        //call the draw for the mesh
                        renderObj.mesh->bindVAO();
                        renderObj.mesh->drawSubMesh(idSubMesh);
                        GL_CHECK_ERRORS();
                    }
                    glUseProgram(0);
                }    
            }
        }
        if(rParams.pSkybox)
        {
            GL_CHECK_ERRORS();
            ShaderProgram* pShaderSkybox = rParams.pSkybox->pShader;
            Matrix4 viewNoTransl = Matrix4(Matrix3(rParams.cam->getViewMatrix()));
            pShaderSkybox->use();
            pShaderSkybox->setUniform("view", viewNoTransl);
            pShaderSkybox->setUniform("projection", rParams.cam->getProjectionMatrix());
            pShaderSkybox->setUniform("skybox", 0);
            // disegna la skybox come se fosse lontanissima
            glDepthFunc(GL_LEQUAL);
            glDisable(GL_CULL_FACE);

            rParams.pSkybox->bindTex();
            rParams.pSkybox->bindVAO();
            rParams.pSkybox->draw();

            glEnable(GL_CULL_FACE);
            glDepthFunc(GL_LESS);
            GL_CHECK_ERRORS();
            glUseProgram(0);
        }
    }

    void UIRenderer::render(const std::vector<UIRenderObject>& uiRenderables)
    {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        for (const auto& ui : uiRenderables)
        {
            if (!ui.pUIMesh || !ui.pMaterial) continue;

            ShaderProgram* shader = ui.pMaterial->getShader();
            GL_CHECK_ERRORS();
            if (shader)
            {
                shader->use();
                ui.pMaterial->bindingPropsToShader();
                shader->setUniform("uiPos", ui.pRect->pos);
                shader->setUniform("size", ui.pRect->size);
                shader->setUniform("projection", WindowManager::sceenProjMatrix);
            }
            // Draw UI mesh
            ui.pUIMesh->bindVAO();
            ui.pUIMesh->draw();
            GL_CHECK_ERRORS();
            glUseProgram(0);
        }
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }

    void TextRenderer::render(const std::vector<TextRenderObject>& textRenderables)
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        for(TextRenderObject textRendObj : textRenderables)
        {
            TextMaterial* pMat = textRendObj.pText->pTextMeshRend->getMaterial();
            TextMesh* pMesh = textRendObj.pText->pTextMeshRend->getTextMesh();
            ShaderProgram* pShader = pMat->getShader();
            
            if (pShader)
            {
                pShader->use();
                pMat->bindingPropsToShader();
                pShader->setUniform("projection", WindowManager::sceenProjMatrix);
                pShader->setUniform("text_tex", 0);
                std::string string = textRendObj.pText->getString();
                Transform2D& transf = *textRendObj.pText->pTransf;
                //resolution adaption
                float resScale = 1.f;
                float offsetX = 0.f;
                Vector2 finalOffset = {0.f, 0.f}; 
                Vector2 finalPos = {transf.pos.x, transf.pos.y};

                Utils::applyRatioScreenRes(transf.anchor, transf.pos, resScale, finalOffset, finalPos);
                transf.setDirty(false);
                
                
                offsetX = finalPos.x;
                pMesh->bindVAO();

                for(auto c = string.begin(); c != string.end(); ++c)
                {
                    GL_CHECK_ERRORS();
                    std::array<std::array<float, 4>, 6> data = pMat->bindCharacter(*c, offsetX, resScale, finalPos, transf);
                    pMesh->subData(data);
                    GL_CHECK_ERRORS();
                    pMesh->draw();
                    GL_CHECK_ERRORS();
                }
                glBindVertexArray(0);
                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            }
        }
        
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glUseProgram(0);
    }

    void ScreenRenderer::render(const std::vector<ScreenRenderObject>& screenRenderables)
    {
        //before rendering
        //glClearColor(0.f, 0.f, 1.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (const auto& screenR : screenRenderables)
        {
            if (!screenR.pPlaneMesh || !screenR.pMaterial) continue;

            ShaderProgram* shader = screenR.pMaterial->getShader();
            GL_CHECK_ERRORS();
            if (shader)
            {
                shader->use();
                screenR.pMaterial->bindingPropsToShader();
                shader->setUniform("res", Vector2{WindowManager::width, WindowManager::height});
            }

            //draw
            screenR.pPlaneMesh->bindVAO();
            screenR.pPlaneMesh->draw();
            GL_CHECK_ERRORS();
        glUseProgram(0);
        }
    }

    //------------------------------------------------------//    
    //---------------------RenderBuffer---------------------//    
    //------------------------------------------------------//

    RenderBuffer::RenderBuffer(GLenum target, GLenum attachment)
    {
        init(target, attachment);
    }
    
    void RenderBuffer::init(GLenum target, GLenum attachment)
    {
        m_target = target;
        m_attachment = attachment;
        glGenRenderbuffers(1, &m_renderBufferObj);
        glBindRenderbuffer(GL_RENDERBUFFER, m_renderBufferObj);
        glRenderbufferStorage(GL_RENDERBUFFER, attachment, WindowManager::width, WindowManager::height);
        //now we support only the GL_DEPTH_ATTACHMENT and GL_FRAMEBUFFER target
        if(attachment == GL_DEPTH_COMPONENT)
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_renderBufferObj);
        else SPACE_ENGINE_ERROR("now we support only the GL_DEPTH_ATTACHMENT and GL_FRAMEBUFFER target");
    }

    //------------------------------------------------------//    
    //---------------------FrameBuffer----------------------//    
    //------------------------------------------------------//

    FrameBuffer::FrameBuffer()
    {
        init();
    }

    FrameBuffer::~FrameBuffer()
    {
        if(m_pRenderBuffer)
            delete m_pRenderBuffer;
    }
    
    void FrameBuffer::init()
    {
        glGenFramebuffers(1, &m_frameBufferObj);
    }

    void FrameBuffer::addColorBuffer()
    {
        TexSetParams params = {
            0, 
            GL_RGBA16F, 
            WindowManager::width, 
            WindowManager::height, 
            0, 
            GL_RGBA, 
            GL_FLOAT, 
            NULL};
        std::string name = "ColorBuffer"+std::to_string(m_vecColorBuffers.size());
        Texture* pColorBuffer = TextureManager::genTexture(GL_TEXTURE_2D, params, name);
        //unbind??
        glFramebufferTexture2D(GL_FRAMEBUFFER, 
            GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(m_vecColorBuffers.size()), 
            GL_TEXTURE_2D, 
            pColorBuffer->getTexture(), 
            0);
    }

    void FrameBuffer::addRenderBuffer()
    {
        m_pRenderBuffer = new RenderBuffer(GL_FRAMEBUFFER, GL_DEPTH_COMPONENT);
    }

    void FrameBuffer::drawBuffers()
    {
        std::vector<uint32_t> attachments;
        
        for(int i = 0; i < m_vecColorBuffers.size(); i++)
        {
           attachments.push_back(GL_COLOR_ATTACHMENT0 + i); 
        }

        glDrawBuffers(static_cast<GLsizei>(m_vecColorBuffers.size()), attachments.data());
    }

    //------------------------------------------------------//    
    //---------------------RendererV2-----------------------//    
    //------------------------------------------------------//

    bool RendererV2::m_preprocessing = false;
    bool RendererV2::m_bloomVFX = false;
    bool RendererV2::m_debug = false;
    FrameBuffer RendererV2::m_HDRFrameBuffer;
    FrameBuffer RendererV2::m_BloomFrameBuffers[2];
    ShaderProgram* RendererV2::m_pHDRShader = nullptr;
    ShaderProgram* RendererV2::m_pBloomShader = nullptr;

    void RendererV2::Initialize()
    {
        if(!m_pHDRShader)
        {
            m_pHDRShader = ShaderManager::findShaderProgram("hdr"); 
            m_pHDRShader->use();
            m_pHDRShader->setUniform("scene", 0);
            m_pHDRShader->setUniform("highlight", 1);
            GL_CHECK_ERRORS();

            //enable HDR range for rendering
            m_HDRFrameBuffer.init();
            GL_CHECK_ERRORS();
            m_HDRFrameBuffer.addColorBuffer();
            m_HDRFrameBuffer.addColorBuffer();
            GL_CHECK_ERRORS();
            //attach depth info
            m_HDRFrameBuffer.addRenderBuffer();
            //use the color attachments for rendering
            m_HDRFrameBuffer.drawBuffers();
            GL_CHECK_FRAMEBUFFER_STATUS();
            m_HDRFrameBuffer.unbindFrameBuffer();

            //postprocessing buffers: bloom vfx
            if(!m_pBloomShader)
            {
                m_pBloomShader = ShaderManager::findShaderProgram("bloomVFX");

                //double buffer
                for(int i = 0; i < 2; i++)
                {
                    //enable HDR range for rendering
                    m_BloomFrameBuffers[i].init();
                    GL_CHECK_ERRORS();
                    m_BloomFrameBuffers[i].addColorBuffer();
                    GL_CHECK_ERRORS();
                    GL_CHECK_FRAMEBUFFER_STATUS();

                }

                m_pBloomShader->use();
                m_pBloomShader->setUniform("LumThresh", 2.f);
                m_pBloomShader->setUniform("BlurTex", 0);
                GL_CHECK_ERRORS();

                //sample the gauss filter
                float weights[10], sum, sigma2 = 25.f;

                weights[0] = Math::gauss(0, sigma2);
                sum = weights[0];

                for(int i = 1; i < 10; i++)
                {
                    weights[i] = Math::gauss(float(i), sigma2);
                    sum += 2 * weights[i]; 
                }

                //normalize weights and set the uniform
                for(int i = 0; i < 10; i++)
                {
                    std::string strWeight = "Weight[" + std::to_string(i) + "]"; 
                    float val = weights[i] / sum;
                    m_pBloomShader->setUniform(strWeight.c_str(), val);
                }
                GL_CHECK_ERRORS();
                glUseProgram(0);
            }
        }

    }

    void RendererV2::Shutdown()
    {

    }

    void RendererV2::clear()
    {
        //clear screen
        glClearColor(0.f, 0.f, 1.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        //render the scene into floating point framebuffer
        m_HDRFrameBuffer.bindFrameBuffer();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GL_CHECK_ERRORS();
    }

    //screen shaders renderer
    void RendererV2::render(const std::vector<ScreenRenderObject>& screenRenderables)
    {
        for (const auto& screenR : screenRenderables)
        {
            if (!screenR.pPlaneMesh || !screenR.pMaterial) continue;

            ShaderProgram* shader = screenR.pMaterial->getShader();
            GL_CHECK_ERRORS();
            if (shader)
            {
                shader->use();
                screenR.pMaterial->bindingPropsToShader();
                shader->setUniform("res", Vector2{WindowManager::width, WindowManager::height});
            }

            //draw
            screenR.pPlaneMesh->bindVAO();
            screenR.pPlaneMesh->draw();
            GL_CHECK_ERRORS();
            glUseProgram(0);
        }
    }
    
    //mesh renderer
    void RendererV2::render(const RendererParams& rParams)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if(rParams.cam)
        {
            for(const auto& renderObj : rParams.renderables)
            {
                if(!renderObj.mesh ) continue;

                for(int idSubMesh = 0, nSubMesh = renderObj.mesh->getNumSubMesh();  idSubMesh < nSubMesh; idSubMesh++)
                {
                    GL_CHECK_ERRORS();
                    //get shader
                    ShaderProgram* shader = renderObj.mesh->getMaterialBySubMeshIndex(idSubMesh)->getShader();
                    shader->use();
                    GL_CHECK_ERRORS();
                    if(shader)
                    {
                        //bind material
                        renderObj.mesh->getMaterialBySubMeshIndex(idSubMesh)->bindingPropsToShader();
                        //set matrices
                        shader->setUniform("model", renderObj.modelMatrix);
                        shader->setUniform("view", rParams.cam->getViewMatrix());
                        shader->setUniform("projection", rParams.cam->getProjectionMatrix());
                        //lights bind
                        if(shader->isPresentUniform("lights[0].pos") && rParams.lights.size())
                        {
                            shader->setUniform("normalMatrix", Math::transpose(Math::inverse(Matrix3(renderObj.modelMatrix))));
                            GL_CHECK_ERRORS();
                            

                            for(int i = 0; i < rParams.lights.size(); i++)
                            {
                                std::string strLight = "lights[" + std::to_string(i) + "].pos"; 
                                shader->setUniform(strLight.c_str(), rParams.lights[i]->pos);
                                GL_CHECK_ERRORS();
                                strLight = "lights[" + std::to_string(i) + "].color"; 
                                shader->setUniform(strLight.c_str(), rParams.lights[i]->color);
                                GL_CHECK_ERRORS();
                                strLight = "lights[" + std::to_string(i) + "].dir"; 
                                shader->setUniform(strLight.c_str(), rParams.lights[i]->dir);
                                GL_CHECK_ERRORS();
                                strLight = "lights[" + std::to_string(i) + "].type"; 
                                shader->setUniform(strLight.c_str(), rParams.lights[i]->type);
                                GL_CHECK_ERRORS();
                            }
                        }

                        //call the draw for the mesh
                        renderObj.mesh->bindVAO();
                        renderObj.mesh->drawSubMesh(idSubMesh);
                        GL_CHECK_ERRORS();
                    }
                    glUseProgram(0);
                }    
            }
        }
        if(rParams.pSkybox)
        {
            GL_CHECK_ERRORS();
            ShaderProgram* pShaderSkybox = rParams.pSkybox->pShader;
            Matrix4 viewNoTransl = Matrix4(Matrix3(rParams.cam->getViewMatrix()));
            pShaderSkybox->use();
            pShaderSkybox->setUniform("view", viewNoTransl);
            pShaderSkybox->setUniform("projection", rParams.cam->getProjectionMatrix());
            pShaderSkybox->setUniform("skybox", 0);
            // disegna la skybox come se fosse lontanissima
            glDepthFunc(GL_LEQUAL);
            glDisable(GL_CULL_FACE);

            rParams.pSkybox->bindTex();
            rParams.pSkybox->bindVAO();
            rParams.pSkybox->draw();

            glEnable(GL_CULL_FACE);
            glDepthFunc(GL_LESS);
            GL_CHECK_ERRORS();
            glUseProgram(0);
        }
    }
    
    //UI renderer
    void RendererV2::render(const std::vector<UIRenderObject>& uiRenderables)
    {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (const auto& ui : uiRenderables)
        {
            if (!ui.pUIMesh || !ui.pMaterial) continue;

            ShaderProgram* shader = ui.pMaterial->getShader();
            GL_CHECK_ERRORS();
            if (shader)
            {
                shader->use();
                ui.pMaterial->bindingPropsToShader();
                shader->setUniform("uiPos", ui.pRect->pos);
                shader->setUniform("size", ui.pRect->size);
                shader->setUniform("projection", WindowManager::sceenProjMatrix);
            }
            // Draw UI mesh
            ui.pUIMesh->bindVAO();
            ui.pUIMesh->draw();
            GL_CHECK_ERRORS();
            glUseProgram(0);
        }

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }
    
    //Text renderer
    void RendererV2::render(const std::vector<TextRenderObject>& textRenderables)
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        for(TextRenderObject textRendObj : textRenderables)
        {
            TextMaterial* pMat = textRendObj.pText->pTextMeshRend->getMaterial();
            TextMesh* pMesh = textRendObj.pText->pTextMeshRend->getTextMesh();
            ShaderProgram* pShader = pMat->getShader();
            
            if (pShader)
            {
                pShader->use();
                pMat->bindingPropsToShader();
                pShader->setUniform("projection", WindowManager::sceenProjMatrix);
                pShader->setUniform("text_tex", 0);
                std::string string = textRendObj.pText->getString();
                Transform2D& transf = *textRendObj.pText->pTransf;
                //resolution adaption
                float resScale = 1.f;
                float offsetX = 0.f;
                Vector2 finalOffset = {0.f, 0.f}; 
                Vector2 finalPos = {transf.pos.x, transf.pos.y};

                Utils::applyRatioScreenRes(transf.anchor, transf.pos, resScale, finalOffset, finalPos);
                transf.setDirty(false);
                
                
                offsetX = finalPos.x;
                pMesh->bindVAO();

                for(auto c = string.begin(); c != string.end(); ++c)
                {
                    GL_CHECK_ERRORS();
                    std::array<std::array<float, 4>, 6> data = pMat->bindCharacter(*c, offsetX, resScale, finalPos, transf);
                    pMesh->subData(data);
                    GL_CHECK_ERRORS();
                    pMesh->draw();
                    GL_CHECK_ERRORS();
                }
                glBindVertexArray(0);
                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            }
        }
        
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glUseProgram(0);
    }

    void RendererV2::postprocessing(bool bloomVFX)
    {
        m_HDRFrameBuffer.unbindFrameBuffer();
        PlaneMesh* pPlaneMesh = MeshManager::getPlaneMesh();
        uint32_t horizontal = 1;
        
        if(m_bloomVFX && m_pBloomShader)
        {
            uint32_t amount = 10; //apply 5 times the 2D gaussian filter
            
            //apply the gaussian filter on the scene render buffer
            m_pBloomShader->use();
            m_BloomFrameBuffers[1].bindFrameBuffer();
            m_pBloomShader->setSubroutinesUniform(GL_FRAGMENT_SHADER, 1, &horizontal);
            glBindTexture(GL_TEXTURE_2D, m_HDRFrameBuffer.getColorBuffer(1));
            pPlaneMesh->bindVAO();
            pPlaneMesh->draw();
            horizontal ^= 1;

            for(uint32_t i = 1; i < amount; i++)
            {
                m_BloomFrameBuffers[horizontal].bindFrameBuffer();
                m_pBloomShader->setSubroutinesUniform(GL_FRAGMENT_SHADER, 1, &horizontal);
                glBindTexture(GL_TEXTURE_2D, m_BloomFrameBuffers[horizontal ^ 1].getColorBuffer(0));
                pPlaneMesh->bindVAO();
                pPlaneMesh->draw();
                horizontal ^= 1;
            }

            m_BloomFrameBuffers[horizontal].unbindFrameBuffer();
        }
        glUseProgram(0);
        //blending 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_pHDRShader->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_HDRFrameBuffer.getColorBuffer(0));
        glActiveTexture(GL_TEXTURE1);
        if(bloomVFX)
            glBindTexture(GL_TEXTURE_2D, m_BloomFrameBuffers[horizontal ^ 1].getColorBuffer(0));
        else glBindTexture(GL_TEXTURE_2D, m_HDRFrameBuffer.getColorBuffer(1));
        GL_CHECK_ERRORS();
        m_pHDRShader->setUniform("exposure", 1.f);
        
        pPlaneMesh->bindVAO();
        pPlaneMesh->draw();
        glUseProgram(0);
    }

};