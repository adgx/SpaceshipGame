#pragma once

#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "utils/stb_image.h"

namespace SpaceEngine {

    class PlayerShip {
    public:
        PlayerShip();
        ~PlayerShip();

        // Inizializza buffers (VAO/VBO) e texture
        void Init();

        // Gestisce l'input e aggiorna la posizione
        void Update(float dt);

        void Render(unsigned int shaderProgramID);

        // Getter per la posizione della nave per sapere da dove sparare i prouettili
        glm::vec3 GetPosition() const { return m_position; }

    private:
        glm::vec3 m_position; 
        float m_speed;

        // Limiti di movimento
        float m_limitX;
        float m_limitY;

        unsigned int m_textureID;
        unsigned int m_VAO, m_VBO, m_EBO; // Buffer per il rettangolo 3D

        void HandleInput(float dt);
        void InitMesh(); // Funzione interna per creare il rettangolo
    };
}