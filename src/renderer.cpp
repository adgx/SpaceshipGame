#include "renderer.h"
#include "shader.h"
#include "windowManager.h"
#include <string>

namespace SpaceEngine
{
    
    void Renderer::render(const RendererParams& rParams)
    {
        //before rendering
        glClearColor(1.f, 1.f, 1.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

                
                if(transf.dirty)
                {
                    Utils::applyRatioScreenRes(transf.anchor, transf.pos, resScale, finalOffset, finalPos);
                    transf.setDirty(false);
                }
                
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
};