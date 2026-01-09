#include "enemyShip.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>

namespace SpaceEngine {

    EnemyShip::EnemyShip(Scene* pScene, std::string filePathModel):GameObject(pScene),
        m_type(EnemyType::NORMAL), m_pTarget(nullptr), m_speed(10.0f),
        m_shootTimer(0.0f), m_shootCooldown(2.0f), m_spawnRangeX(50.0f), m_spawnRangeY(30.0f),
        m_spawnZ(-100.0f), m_despawnZ(20.0f)
    {
        m_pMesh = MeshManager::loadMesh(filePathModel);
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
            m_pTransform->setLocalRotation(Vector3(0, 3.14f, 0)); 

            m_pTransform->setLocalScale(Vector3(2.0f));
        }

        setLayer(ELayers::ENEMY_LAYER);

        // Setup statistiche in base al tipo
        switch (m_type) {
            case EnemyType::NORMAL:
                m_speed = 3.0f;
                m_shootCooldown = 2.0f;
                m_health = 1;
                break;
            case EnemyType::AIMER:
                m_speed = 2.0f; // Più lento ma mira
                m_shootCooldown = 1.5f;
                m_health = 1;
                break;
            case EnemyType::SPREAD:
                m_speed = 4.0f; // Veloce
                m_shootCooldown = 3.0f;
                m_health = 1;       //TODO: aumentare vita navicelle AIMER e SPREAD a 3
                break;
        }
    }

    void EnemyShip::update(float dt) {
        if (!m_pTransform) return;
        performAI(dt);

        m_shootTimer += dt;

        if (m_shootTimer >= m_shootCooldown) {
            m_shootTimer = 0.0f;
            // Spara solo se è davanti alla camera (non troppo vicino)
            Shoot();
        }
    }

    void EnemyShip::Shoot() {
        Vector3 spawnPos = m_pTransform->getWorldPosition();
        spawnPos.z += 1.5f; // Un po' dietro (verso la camera)
        Quat qRot = m_pTransform->getLocalRotation();
    
        Vector3 baseRotation = glm::eulerAngles(qRot);// converte quaternione in vettore di angoli
        baseRotation.x += 1.5708f; //per sparare verso il player
        
        if (m_type == EnemyType::SPREAD) 
        {
            float angles[] = { 0.0f, 0.52f, -0.52f }; // 0, +30°, -30° in radianti
            Vector3 forwardDir(0.0f, 0.0f, 1.0f); //direzione verso il player

            for (float angleOffset : angles) {
                Bullet* pBullet = new Bullet(pScene, "Bullet.obj");
                
                pBullet->setLayer(ELayers::BULLET_LAYER);
                Vector3 shootDir = glm::rotateY(forwardDir, angleOffset);

                float rotY = atan2(shootDir.x, shootDir.z);
                Vector3 visualRot(1.5708f, rotY, 0.0f);

                pBullet->Fire(spawnPos, shootDir, visualRot, 15.0f); 
                pScene->requestInstantiate(pBullet);
            }
        }
        else if(m_type == EnemyType::AIMER && m_pTarget) {
            Bullet* pBullet = new Bullet(pScene, "Bullet.obj");
            pBullet->setLayer(ELayers::BULLET_LAYER);

            // per trovare angolo di sparo verso il player in base alla sua pos attuale
            Vector3 targetPos = m_pTarget->getTransform()->getWorldPosition();
            Vector3 direction = glm::normalize(targetPos - spawnPos);
            
            float angleY = atan2(direction.x, direction.z); //in radianti
            
            Vector3 aimRot = Vector3(1.5708f, angleY, 0); 

            pBullet->Fire(spawnPos, direction, aimRot, 15.0f);
            pScene->requestInstantiate(pBullet);
        }
        else{
            Bullet* pBullet = new Bullet(pScene, "Bullet.obj");
            pBullet->setLayer(ELayers::BULLET_LAYER);

            Vector3 shootDir(0.0f, 0.0f, 1.0f); // Dritto verso +Z
            Vector3 visualRot(1.5708f, 0.0f, 0.0f);

            pBullet->Fire(spawnPos, shootDir, visualRot, 15.0f);
            pScene->requestInstantiate(pBullet);
        }

        if (auto* audioMgr = pScene->getAudioManager()) {
            audioMgr->PlaySound("shoot_enemy");
        }
    }

    void EnemyShip::performAI(float dt) {
        // Movimento in avanti
        Vector3 currentPos = m_pTransform->getWorldPosition();
        currentPos.z += m_speed * dt;

        if (m_type == EnemyType::AIMER && m_pTarget) {
            // Aimer: si sposta lentamente su X verso il giocatore
            Vector3 targetPos = m_pTarget->getTransform()->getWorldPosition();
            if (currentPos.x < targetPos.x) currentPos.x += m_speed * 0.5f * dt;
            if (currentPos.x > targetPos.x) currentPos.x -= m_speed * 0.5f * dt;
        }
        else if (m_type == EnemyType::SPREAD) {
            // Spread: movimento a zig-zag (seno)
            currentPos.x += sin(currentPos.z * 0.5f) * 5.0f * dt; 
        }

        m_pTransform->setWorldPosition(currentPos);
    }

    void EnemyShip::onCollisionEnter(Collider* col) {
        SPACE_ENGINE_INFO("Enemy hit something!");
        m_health--;

        if (m_health <= 0) {
            if(pScene) pScene->requestDestroy(this);
            if (auto* audioMgr = pScene->getAudioManager()) {
                audioMgr->PlaySound("enemy_explosion");
            }
        }  
    }
}