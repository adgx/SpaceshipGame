#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>
#include "GameObject.h"
#include "renderer.h"
#include "collisionDetection.h"

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

    class EnemyShip : public GameObject {
    public:
        EnemyShip(Scene* pScene, std::string filePathModel);
        virtual ~EnemyShip();

        virtual void Init(glm::vec3 spawnPos, EnemyType type, GameObject* pTarget = nullptr);

        // Muove la nave e gestisce il timer di sparo
        // Restituisce un vettore di proiettili (vuoto se non spara in questo frame)
        //std::vector<Bullet> Update(float dt, glm::vec3 playerPos); //funzione update con sparo
        virtual void update(float dt) override; //funzione update senza sparo

        virtual void onCollisionEnter(Collider* col) override;

        RenderObject getRenderObject();

        //da implementare: se supera la videocamera si deve distruggere

    private:
        EnemyType m_type;
        GameObject* m_pTarget; //il giocatore da mirare per aimer

        float m_speed;

        float m_spawnRangeX;
        float m_spawnRangeY;

        float m_spawnZ, m_despawnZ;

        float m_shootTimer;
        float m_shootCooldown; // Ogni quanto spara

        void performAI(float dt);

        //std::vector<Bullet> Shoot(glm::vec3 playerPos); // Logica di sparo
    };
}