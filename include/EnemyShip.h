#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

struct Bullet {
    glm::vec3 position;
    glm::vec3 direction;
    float speed;
    float lifeTime; // Per distruggerlo dopo un po'
    
    // Semplice update integrato per brevità
    void Update(float dt) {
        position += direction * speed * dt;
        lifeTime -= dt;
    }
};

namespace SpaceEngine {

    enum class EnemyType {
        NORMAL,  // Tipo 1: Spara dritto
        AIMER,   // Tipo 2: Spara al giocatore
        SPREAD   // Tipo 3: Spara 3 colpi davanti, e a +-30 gradi
    };

    class EnemyShip {
    public:
        EnemyShip();
        ~EnemyShip();

        void Init(unsigned int textureID, glm::vec3 spawnPos, EnemyType type);

        // Muove la nave e gestisce il timer di sparo
        // Restituisce un vettore di proiettili (vuoto se non spara in questo frame)
        std::vector<Bullet> Update(float dt, glm::vec3 playerPos);

        void Render(unsigned int shaderProgramID);

        glm::vec3 GetPosition() const { return m_position; }
        bool IsDead() const { return m_position.z > 20.0f; } // Se supera la camera si distrugge

    private:
        glm::vec3 m_position;
        EnemyType m_type;
        float m_speed;

        float m_shootTimer;
        float m_shootCooldown; // Ogni quanto spara

        unsigned int m_textureID;
        unsigned int m_VAO, m_VBO;

        void InitMesh();
        std::vector<Bullet> Shoot(glm::vec3 playerPos); // Logica interna
    };
}