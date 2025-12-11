#include "Asteroid.h"
#include <iostream>
#include <ctime>

namespace SpaceEngine {

    Asteroid::Asteroid(std::string filePathModel) {
        m_pMesh = MeshManager::loadMesh(filePathModel);
        BaseMaterial* pMat = m_pMesh->getMaterialBySubMeshIndex(0);
        pMat->pShader = ShaderManager::findShaderProgram("simpleTex");
        m_pCollider = new Collider(this);

        m_rotationSpeed = 0.0f;
        m_velocity = 0.0f;
        m_spawnZ = -100.0f;
        m_despawnZ = 20.0f;     // Arriva fino a dietro la camera

        m_spawnRangeX = 50.0f;  // Range orizzontale di spawn
        m_spawnRangeY = 30.0f;  // Range verticale di spawn

        // Assegna un asse di rotazione casuale
        srand(static_cast<unsigned int>(time(0)));
        m_rotationAxis = glm::normalize(glm::vec3(
            static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
            static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
            static_cast <float>(rand()) / static_cast <float> (RAND_MAX)));
    }

    Asteroid::~Asteroid() {
        if(m_pMesh) delete m_pMesh;
    }

    void Asteroid::Init() {
        if (m_pTransform) {
            m_pTransform->setLocalScale(glm::vec3(2.0f)); // Dimensione di base
        }

        Spawn(); // Appena creato, posizionalo subito
    }

    void Asteroid::Spawn() {
        // Genera X e Y casuali
        
        float randomX = -m_spawnRangeX + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (m_spawnRangeX * 2)));
        float randomY = -m_spawnRangeY + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (m_spawnRangeY * 2)));

        if (m_pTransform) {
            m_pTransform->setLocalPosition(glm::vec3(randomX, randomY, m_spawnZ));
            
            float scaleVar = 1.0f + static_cast<float>(rand()) / (RAND_MAX / 1.0f);
            m_pTransform->setLocalScale(glm::vec3(scaleVar));
        }

        // Velocità leggermente diversa (es. tra 10 e 25)
        m_velocity = 10.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 10.0f));
        
        // Rotazione casuale
        m_rotationSpeed = 30.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 60.0f));
        m_rotationAxis = glm::normalize(glm::vec3((float)rand(), (float)rand(), (float)rand()));
    }

    void Asteroid::update(float dt) {
        if (!m_pTransform) return;

        // movimento in avanti (verso il player)
        glm::vec3 currentPos = m_pTransform->getLocalPosition();
        currentPos.z += m_velocity * dt;
        m_pTransform->setLocalPosition(currentPos);

        // 2. Rotazione su se stesso (effetto visivo)
        m_pTransform->rotateLocal(m_rotationSpeed * dt, m_rotationAxis);

        // 3. Controllo Uscita Schermo (Riciclo)
        if (currentPos.z > m_despawnZ) {
            Spawn(); // Resetta e ricomincia dal fondo
        }
    }

    /*void Asteroid::InitMesh() {
        float vertices[] = {
            // Back face
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1.0f, 0.0f,         
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            // Front face
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            // Left face
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            // Right face
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            // Bottom face
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            // Top face
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f
        };
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void Asteroid::Render(unsigned int shaderProgramID) {
        glUseProgram(shaderProgramID);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_textureID);

        // per posizionare
        glm::mat4 model = glm::mat4(1.0f);
        if (m_pTransform) {
            model = m_pTransform->getWorldMatrix();
        }
        
        unsigned int modelLoc = glGetUniformLocation(shaderProgramID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

        glBindVertexArray(m_VAO);
        // Disegna 36 vertici per un cubo (6 facce * 2 triangoli * 3 vertici)
        glDrawArrays(GL_TRIANGLES, 0, 36); 
        glBindVertexArray(0);
    }*/

    RenderObject Asteroid::getRenderObject() {
        RenderObject renderObj;
        renderObj.mesh = m_pMesh;
        glm::mat4 model = glm::mat4(1.0f);

        if (m_pTransform) {
            model = glm::translate(model, m_pTransform->getLocalPosition());

            glm::mat4 rot_mat = glm::mat4_cast(m_pTransform->getLocalRotation());
            
            model = model * rot_mat;
            
            model = glm::scale(model, m_pTransform->getLocalScale());
        }

        renderObj.modelMatrix = model;
        return renderObj;
    }

    void Asteroid::onCollisionEnter(Collider* col) {
        SPACE_ENGINE_INFO("PlayerShip Collision onEnter Called with Collider: {}", reinterpret_cast<std::uintptr_t>(col));
    }
}