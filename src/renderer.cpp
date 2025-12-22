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
                //bind material
                renderObj.mesh->getMaterialBySubMeshIndex(idSubMesh)->bindingPropsToShader();
                renderObj.mesh->bindVAO();
                //get shader
                ShaderProgram* shader = renderObj.mesh->getMaterialBySubMeshIndex(idSubMesh)->getShader();
                shader->use();
                //set matrices
                if(shader)
                {
                    shader->setUniform("model", renderObj.modelMatrix);
                    shader->setUniform("view", rParams.cam.getViewMatrix());
                    shader->setUniform("projection", rParams.cam.getProjectionMatrix());
                    //lights bind
                    if(shader->isPresentUniform("lights") && rParams.lights.size())
                    {
                        for(int i = 0; i < rParams.lights.size(); i++)
                        {
                            std::string strLight = "lights[" + std::to_string(i) + "].pos"; 
                            shader->setUniform(strLight.c_str(), rParams.lights[i]->pos);
                            strLight = "lights[" + std::to_string(i) + "].color"; 
                            shader->setUniform(strLight.c_str(), rParams.lights[i]->color);
                        }
                    }

                    //subroutines 

                    //call the draw for the mesh
                    renderObj.mesh->drawSubMesh(idSubMesh);
                }
            }    
        }
        if(rParams.pSkybox){
            ShaderProgram* pShaderSkybox = rParams.pSkybox->pShader;
            glm::mat4 viewNoTransl = glm::mat4(glm::mat3(rParams.cam.getViewMatrix()));

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