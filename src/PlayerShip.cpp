#include "PlayerShip.h"
#include "inputManager.h"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "utils/stb_image.h"

namespace SpaceEngine {

    /*static unsigned int loadTextureFromFile(const char* path) {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(true); 
        unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
        
        if (!data) {
            SPACE_ENGINE_ERROR("Failed to load texture: {}", path);
            return 0;
        }

        GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
        unsigned int tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        
        // Parametri texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
        return tex;
    }*/

    PlayerShip::PlayerShip(std::string filePathModel){
        m_pMesh = MeshManager::loadMesh(filePathModel);
        BaseMaterial* pMat = m_pMesh->getMaterialBySubMeshIndex(0);
        pMat->pShader = ShaderManager::findShaderProgram("simpleTex");
        m_pCollider = new Collider(this);

        m_speed = 15.0f;               
        m_limitX = 14.0f;              
        m_limitY = 9.0f;               
        m_shootCooldown = 0.0f;        
        //m_textureID = 0;
    }

    PlayerShip::~PlayerShip() {
        if(m_pMesh) delete m_pMesh;
        //if (m_textureID) glDeleteTextures(1, &m_textureID);
    }

    void PlayerShip::Init() {
        if (m_pTransform) {
            m_pTransform->setLocalPosition(glm::vec3(0.0f, 0.0f, -10.0f));
            m_pTransform->setLocalScale(glm::vec3(1.0f));
        }
        // Carica Texture
        //m_textureID = loadTextureFromFile("assets/textures/spaceship.png");
    }

    RenderObject PlayerShip::getRenderObject() {
        RenderObject obj;

        obj.mesh = m_pMesh;

        // calcola la Model Matrix
        glm::mat4 model = glm::mat4(1.0f);
        
        // Sposta la mesh alla posizione del GameObject 
        model = glm::translate(model, m_pTransform->getLocalPosition());

        obj.modelMatrix = model; 
        
        return obj;
    }

    /*void PlayerShip::InitMesh() {
        //posizione x,y,z dei vertici + coordinate texture u,v
        float vertices[] = {
             // Posizioni        // Texture Coords
             0.5f,  0.5f, 0.0f,  1.0f, 1.0f, // Alto Destra
             0.5f, -0.5f, 0.0f,  1.0f, 0.0f, // Basso Destra
            -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, // Basso Sinistra
            -0.5f,  0.5f, 0.0f,  0.0f, 1.0f  // Alto Sinistra 
        };
        
        unsigned int indices[] = {  
            0, 1, 3, // Primo Triangolo
            1, 2, 3  // Secondo Triangolo
        };

        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);

        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // Attributo 0: Posizione
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Attributo 1: Coordinate Texture
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        glBindVertexArray(0); 
    }*/

    void PlayerShip::update(float dt) {
        HandleInput(dt);

        if(m_shootCooldown > 0.0f) {
            m_shootCooldown -= dt;
        }
    }

    void PlayerShip::HandleInput(float dt) {

        glm::vec3 m_position = m_pTransform->getLocalPosition();
        float velocity = m_speed * dt;

        // --- MOVIMENTO ---
        if (Keyboard::key(SPACE_ENGINE_KEY_BUTTON_A)) m_position.x -= velocity;
        if (Keyboard::key(SPACE_ENGINE_KEY_BUTTON_D)) m_position.x += velocity;
        if (Keyboard::key(SPACE_ENGINE_KEY_BUTTON_W)) m_position.y += velocity;
        if (Keyboard::key(SPACE_ENGINE_KEY_BUTTON_S)) m_position.y -= velocity;

        // Impedisce alla nave di uscire dall'area di gioco
        if (m_position.x > m_limitX)  m_position.x = m_limitX;
        if (m_position.x < -m_limitX) m_position.x = -m_limitX;
        if (m_position.y > m_limitY)  m_position.y = m_limitY;
        if (m_position.y < -m_limitY) m_position.y = -m_limitY;

        m_pTransform->setLocalPosition(m_position);
    }

    /*void PlayerShip::Render(unsigned int shaderProgramID) {
        glUseProgram(shaderProgramID);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_textureID);
        
        glm::mat4 model = glm::mat4(1.0f);
        if (m_pTransform) {
            // Usa la posizione del transform
            model = glm::translate(model, m_pTransform->getLocalPosition());
            // Applica anche scala e rotazione se servono
            model = glm::scale(model, m_pTransform->getLocalScale());
            // model = glm::rotate(model, ...); se vogliamo applicare rotazioni durante il movimento/sparo
        } else {
            // Fallback se transform è nullo
            model = glm::translate(model, glm::vec3(0,0,-10)); 
        } 
        
        unsigned int modelLoc = glGetUniformLocation(shaderProgramID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }*/

    void PlayerShip::onCollisionEnter(Collider* col) {
        SPACE_ENGINE_INFO("PlayerShip Collision onEnter Called with Collider: {}", reinterpret_cast<std::uintptr_t>(col));
    }
}