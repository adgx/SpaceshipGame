#include "EnemyShip.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace SpaceEngine {

    EnemyShip::EnemyShip() 
        : m_position(0.0f), m_speed(10.0f), m_shootTimer(0.0f), m_shootCooldown(2.0f)
    {
        InitMesh();
    }

    EnemyShip::~EnemyShip() {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
    }

    void EnemyShip::Init(unsigned int textureID, glm::vec3 spawnPos, EnemyType type) {
        m_textureID = textureID;
        m_position = spawnPos;
        m_type = type;
        
        // Per decidere velocità di fuoco in base al tipo
        if (m_type == EnemyType::SPREAD) m_shootCooldown = 3.0f;
        if (m_type == EnemyType::AIMER) m_shootCooldown = 3.0f;
    }

    std::vector<Bullet> EnemyShip::Update(float dt, glm::vec3 playerPos) {
        m_position.z += m_speed * dt;

        m_shootTimer += dt; //
        
        std::vector<Bullet> newBullets;

        if (m_shootTimer >= m_shootCooldown) {
            m_shootTimer = 0.0f;
            // Spara solo se è davanti alla camera (non troppo vicino)
            if (m_position.z < playerPos.z - 5.0f) {
                newBullets = Shoot(playerPos);
            }
        }

        return newBullets; // Ritorna i proiettili alla scena
    }

    std::vector<Bullet> EnemyShip::Shoot(glm::vec3 playerPos) {
        std::vector<Bullet> bullets;
        float bulletSpeed = 40.0f; // Velocità del proiettile

        // Posizione di partenza del colpo (leggermente avanti alla nave)
        glm::vec3 startPos = m_position; 
        startPos.z += 1.0f; 

        switch (m_type) {
            case EnemyType::NORMAL: {
                Bullet b;
                b.position = startPos;
                b.direction = glm::vec3(0.0f, 0.0f, 1.0f); // Dritto verso camera
                b.speed = bulletSpeed;
                b.lifeTime = 5.0f;
                bullets.push_back(b);
                break;
            }

            case EnemyType::AIMER: {
                Bullet b;
                b.position = startPos;
                // Calcolo vettore direzione: (Destinazione - Origine) normalizzato
                b.direction = glm::normalize(playerPos - m_position);
                b.speed = bulletSpeed;
                b.lifeTime = 5.0f;
                bullets.push_back(b);
                break;
            }

            case EnemyType::SPREAD: {
                float angles[] = { 0.0f, 0.52f, -0.52f }; //0.52rad = 30 gradi

                for (float angle : angles) {
                    Bullet b;
                    b.position = startPos;
                    b.speed = bulletSpeed;
                    b.lifeTime = 5.0f;
                    // Direzione calcolata con angolo
                    b.direction = glm::vec3(sin(angle), 0.0f, cos(angle));
                    
                    bullets.push_back(b);
                }
                break;
            }
        }

        return bullets;
    }

    void EnemyShip::Render(unsigned int shaderProgramID) {
        // ... (Codice Render standard: glUseProgram, bind texture, model matrix, draw arrays) ...
        // Usa una matrice Model con translate(m_position) e rotate(180, Y) perché la nave guarda verso di noi
    }
    
    void EnemyShip::InitMesh() {
        // ... (Copia il codice di creazione VAO/VBO da PlayerShip o Asteroid) ...
        // Assicurati che i vertici formino un quadrato o un cubo
    }
}