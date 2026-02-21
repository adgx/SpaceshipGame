#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>
#include "scene.h"
#include "GameObject.h"
#include "renderer.h"
#include "collisionDetection.h"
#include "bullet.h"

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
        void Init(Vector3 spawnPos, EnemyType type, GameObject* pTarget = nullptr, float vel = 0.f, int ticket = 0, float bulletSpeed = 1.0f);
        virtual void update(float dt) override;
        virtual void onCollisionEnter(Collider* col) override;
        void DecreaseHealth();

        void Shoot();

        //da implementare: se supera la videocamera si deve distruggere

    private:
        GameObject* m_pTarget; //il giocatore da mirare per aimer
        PointSubject* m_pSub;
        SpawnerSubject* m_pSpawnerSub;
        const Bullet* m_pBulletPrefab;

        float m_speed;
        float m_spawnRangeX;
        float m_spawnRangeY;
        float m_spawnZ, m_despawnZ;
        float m_shootCooldown; // Ogni quanto spara
        float m_shootTimer;

        float m_bulletSpeed = 1.f;
        
        int m_health;
        int m_score = 100;
    
        EnemyType m_type;

        void performAI(float dt);
    };
}