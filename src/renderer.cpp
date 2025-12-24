#include "renderer.h"
#include "shader.h"
#include "windowManager.h"
#include <string>

namespace SpaceEngine
{
    
    void Renderer::render(const RendererParams& rParams)
    {

        for(const auto& renderObj : rParams.renderables)
        {
            if(!renderObj.mesh ) continue;
            
            for(int idSubMesh = 0, nSubMesh = renderObj.mesh->getNumSubMesh();  idSubMesh < nSubMesh; idSubMesh++)
            {
                auto glError = glGetError();
                //get shader
                ShaderProgram* shader = renderObj.mesh->getMaterialBySubMeshIndex(idSubMesh)->getShader();
                shader->use();
                glError = glGetError();
                if(shader)
                {
                    //bind material
                    renderObj.mesh->getMaterialBySubMeshIndex(idSubMesh)->bindingPropsToShader();
                    //set matrices
                    shader->setUniform("model", renderObj.modelMatrix);
                    shader->setUniform("view", rParams.cam.getViewMatrix());
                    shader->setUniform("projection", rParams.cam.getProjectionMatrix());
                    //lights bind
                    if(shader->isPresentUniform("lights[0].pos") && rParams.lights.size())
                    {
                        shader->setUniform("normalMatrix", Math::transpose(Math::inverse(Matrix3(renderObj.modelMatrix))));
                        glError = glGetError();
                        
                        for(int i = 0; i < rParams.lights.size(); i++)
                        {
                            std::string strLight = "lights[" + std::to_string(i) + "].pos"; 
                            shader->setUniform(strLight.c_str(), rParams.lights[i]->pos);
                            glError = glGetError();
                            strLight = "lights[" + std::to_string(i) + "].color"; 
                            shader->setUniform(strLight.c_str(), rParams.lights[i]->color);
                            glError = glGetError();
                        }
                    }
                    
                    //call the draw for the mesh
                    renderObj.mesh->bindVAO();
                    renderObj.mesh->drawSubMesh(idSubMesh);
                    glError = glGetError();

                }
                glUseProgram(0);
            }    
        }
        if(rParams.pSkybox)
        {
            auto glError = glGetError();
            ShaderProgram* pShaderSkybox = rParams.pSkybox->pShader;
            Matrix4 viewNoTransl = Matrix4(Matrix3(rParams.cam.getViewMatrix()));
            pShaderSkybox->use();
            pShaderSkybox->setUniform("view", viewNoTransl);
            pShaderSkybox->setUniform("projection", rParams.cam.getProjectionMatrix());
            pShaderSkybox->setUniform("skybox", 0);
            // disegna la skybox come se fosse lontanissima
            glDepthFunc(GL_LEQUAL);
            glDisable(GL_CULL_FACE);

            rParams.pSkybox->bindTex();
            rParams.pSkybox->bindVAO();
            rParams.pSkybox->draw();

            glEnable(GL_CULL_FACE);
            glDepthFunc(GL_LESS);
            glError = glGetError();
            glUseProgram(0);
        }
    }

    void UIRenderer::render(const std::vector<UIRenderObject>& uiRenderables)
    {
        for (const auto& ui : uiRenderables)
        {
            if (!ui.pUIMesh || !ui.pMaterial) continue;

            ui.pMaterial->bindingPropsToShader();
            ui.pUIMesh->bindVAO();
            ShaderProgram* shader = ui.pMaterial->getShader();
            
            if (shader)
            {
                shader->setUniform("uiPos", ui.pRect->pos);
                shader->setUniform("size", ui.pRect->size);
                shader->setUniform("projection", WindowManager::sceenProjMatrix);
            }
            
            // Draw UI mesh
            ui.pUIMesh->draw();
        }
    }

};