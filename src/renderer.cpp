#include "renderer.h"
#include "shader.h"
#include "windowManager.h"

namespace SpaceEngine
{
    void Renderer::render(const std::vector<RenderObject>& renderables, BaseCamera& cam, Skybox* pSkybox)
    {
        if(pSkybox)
        {
            glDepthFunc(GL_LEQUAL);
            pSkybox->bindVAO();
            pSkybox->pShader->use();
            pSkybox->pShader->setUniform("view", Matrix4(Matrix3(cam.getViewMatrix())));
            pSkybox->pShader->setUniform("projection", cam.getProjectionMatrix());
            pSkybox->draw();
            glDepthFunc(GL_LESS);
        }

        for(const auto& renderObj : renderables)
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
                    shader->setUniform("view", cam.getViewMatrix());
                    shader->setUniform("projection", cam.getProjectionMatrix());
                    //call the draw for the mesh
                    renderObj.mesh->drawSubMesh(idSubMesh);
                }

            }
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