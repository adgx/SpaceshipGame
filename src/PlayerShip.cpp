#include "PlayerShip.h"
#include "inputManager.h"


namespace SpaceEngine {

    PlayerShip::PlayerShip()
        : m_position(0.0f, 0.0f, -10.0f) // Posizionata davanti alla camera
        , m_speed(15.0f)                 // Velocità in unità/secondo
        , m_limitX(14.0f)                // Limite orizzontale (dipende dallo zoom camera)
        , m_limitY(9.0f)                 // Limite verticale
        , m_textureID(0)
    {}

    PlayerShip::~PlayerShip() {
        // Pulizia della memoria video
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_EBO);
        if (m_textureID) glDeleteTextures(1, &m_textureID);
    }

    void PlayerShip::Init() {
        // Carica Texture
        // m_textureID = loadTextureFromFile("assets/textures/spaceship.png");
        
        //Crea la mesh della nave
        InitMesh();
    }

    void PlayerShip::InitMesh() {
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
    }

    void PlayerShip::Update(float dt) {
        HandleInput(dt);
    }

    void PlayerShip::HandleInput(float dt) {
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
    }

    void PlayerShip::Render(unsigned int shaderProgramID) {
        glUseProgram(shaderProgramID);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_textureID);
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, m_position); 
        
        unsigned int modelLoc = glGetUniformLocation(shaderProgramID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}