#include "renderer.h"
#include "shader.h"
#include "windowManager.h"

namespace SpaceEngine
{
    void Renderer::render(const std::vector<RenderObject>& renderables, BaseCamera& cam)
    {
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
                    for(int i = 0; i < renderObj.instances; i++)
                    {
                        shader->setUniform("model", renderObj.modelMatrix[i]);
                        shader->setUniform("view", cam.getViewMatrix());
                        //shader->setUniform("view",glm::lookAt(
                        //glm::vec3(3.0f, 3.0f, 3.0f),  // camera position
                        //glm::vec3(0.0f, 0.0f, 0.0f),  // target (center of cube)
                        //glm::vec3(0.0f, 1.0f, 0.0f)));   // world up 
                        shader->setUniform("projection", cam.getProjectionMatrix());
                            

                        //call the draw for the mesh
                        renderObj.mesh->drawSubMesh(idSubMesh);
                    }
                }

            }
        }
        
    }

    //TODO
    void UIRenderer::render(const std::vector<UIRenderObject>& uiRenderables)
    {
        for (const auto& ui : uiRenderables)
        {
            if (!ui.mesh || !ui.material) continue;

            // UIMaterial::Apply() automatically binds ortho projection
            //ui.material->Apply();

            // Set model matrix
            ShaderProgram* shader = ui.material->getShader();
            if (shader)
                shader->setUniform("model", ui.modelMatrix);
                shader->setUniform("projection", ui.modelMatrix);

            // Draw UI mesh
            //ui.mesh->Draw();
        }
    }

};