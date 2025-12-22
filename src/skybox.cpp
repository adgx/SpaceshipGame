#include "skybox.h"
#include "log.h"
#include "texture.h"

namespace SpaceEngine
{

    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
    };

    Skybox::Skybox()
    {
        init();
    }
    void Skybox::bindTex()
    {
        pCubeMapTex->bind();
    }
    void Skybox::init()
    {
        auto flag = glGetError();
        // Setup del cubo geometrico (VAO/VBO)
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        pCubeMapTex = TextureManager::loadCubeMap("skybox"); 
        // per caricare lo shader
        pShader = ShaderManager::findShaderProgram("skybox");
        if (!pShader) {
            SPACE_ENGINE_ERROR("Skybox shader mancante!");
            return;
        }
        
        pCubeMapTex->bind();
        pShader->use();
        pShader->setUniform("skybox", 0);

        glBindVertexArray(0);
        glUseProgram(0);
    }

    ShaderProgram* Skybox::getShader()
    {
        return pShader;
    }

    void Skybox::bindVAO()
    {
        glBindVertexArray(VAO);
    }

    void Skybox::draw()
    {
        glActiveTexture(GL_TEXTURE0); 
        if (pCubeMapTex) {
            pCubeMapTex->bind();
        }


        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        
    }
}