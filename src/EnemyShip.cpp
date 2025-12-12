#include "enemyShip.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace SpaceEngine {

    EnemyShip::EnemyShip(Scene* pScene, std::string filePathModel):GameObject(pScene),
        m_type(EnemyType::NORMAL), m_pTarget(nullptr), m_speed(10.0f),
        m_shootTimer(0.0f), m_shootCooldown(2.0f), m_spawnRangeX(50.0f), m_spawnRangeY(30.0f),
        m_spawnZ(-100.0f), m_despawnZ(20.0f)
    {
        m_pMesh = MeshManager::loadMesh(filePathModel);
        BaseMaterial* pMat = m_pMesh->getMaterialBySubMeshIndex(0);
        pMat->pShader = ShaderManager::findShaderProgram("simpleTex");
        m_pTransform = new Transform();
        m_pCollider = new Collider(this);

        m_speed = 10.0f;
    }

    EnemyShip::~EnemyShip() {
    }

    void EnemyShip::Init(glm::vec3 spawnPos, EnemyType type, GameObject* pTarget) {
        m_type = type;
        m_pTarget = pTarget;
        
        if (m_pTransform) {
            m_pTransform->setLocalPosition(spawnPos);
            // Ruota di 180 gradi su Y se il modello guarda verso -Z ma arriva da -Z verso +Z
            m_pTransform->setLocalRotation(glm::vec3(0, 180.0f, 0)); 
        }

        // Setup statistiche in base al tipo
        switch (m_type) {
            case EnemyType::NORMAL:
                m_speed = 8.0f;
                m_shootCooldown = 2.0f;
                break;
            case EnemyType::AIMER:
                m_speed = 6.0f; // Più lento ma mira
                m_shootCooldown = 1.5f;
                break;
            case EnemyType::SPREAD:
                m_speed = 10.0f; // Veloce
                m_shootCooldown = 3.0f;
                break;
        }
    }

    /*std::vector<Bullet> EnemyShip::update(float dt) {
        performAI(dt);

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
    }  PER SPARARE */

    void EnemyShip::update(float dt) {
        if (!m_pTransform) return;

        performAI(dt);

        // se supera la camera si distrugge
        if (m_pTransform->getWorldPosition().z > m_despawnZ) {
            destroy();
            SPACE_ENGINE_INFO("Enemy despawned (out of bounds)");
        }
    }

    void EnemyShip::performAI(float dt) {
        // Movimento in avanti
        Vector3 currentPos = m_pTransform->getWorldPosition();
        currentPos.z += m_speed * dt;

        if (m_type == EnemyType::AIMER && m_pTarget) {
            // Esempio Aimer: Si sposta lentamente su X verso il giocatore
            Vector3 targetPos = m_pTarget->getTransform()->getWorldPosition();
            if (currentPos.x < targetPos.x) currentPos.x += m_speed * 0.5f * dt;
            if (currentPos.x > targetPos.x) currentPos.x -= m_speed * 0.5f * dt;
        }
        else if (m_type == EnemyType::SPREAD) {
            // Esempio Spread: Movimento a zig-zag (seno)
            currentPos.x += sin(currentPos.z * 0.5f) * 5.0f * dt; 
        }

        m_pTransform->setWorldPosition(currentPos);
    }

    /*std::vector<Bullet> EnemyShip::Shoot(glm::vec3 playerPos) {
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
    } PER SPARARE*/

    void EnemyShip::onCollisionEnter(Collider* col) {
        SPACE_ENGINE_INFO("Enemy hit something!");
        // Logica distruzione da settare
    }
}