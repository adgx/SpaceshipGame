#include "asteroid.h"

#include "collisionDetection.h"
#include "scene.h"

#include <iostream>
#include <ctime>


namespace SpaceEngine {

    Asteroid::Asteroid(Scene* pScene, std::string filePathModel):GameObject(pScene) {
        m_pMesh = MeshManager::loadMesh(filePathModel);
        m_pTransform = new Transform();
        m_pCollider = new Collider(this);
        
        m_rotationSpeed = 10.0f;
        m_velocity = 0.0f;
        m_spawnZ = -100.0f;
        m_despawnZ = 20.0f;     // Arriva fino a dietro la camera

        // Assegna un asse di rotazione casuale
        srand(static_cast<unsigned int>(time(0)));
        m_rotationAxis = glm::normalize(glm::vec3(
            static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
            static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
            static_cast <float>(rand()) / static_cast <float> (RAND_MAX)));
    }

    Asteroid::~Asteroid() {
        if(m_pSub)
            delete m_pSub;
    }

    void Asteroid::Init(Vector3 startPos) {
        m_pSub = new PointSubject();
        if (m_pTransform) {
            // Usa la posizione decisa dalla Scena
            m_pTransform->setWorldPosition(startPos);
            
            // per avere asteroidi di diverse dimensioni
            float scaleVar = 1.0f + static_cast<float>(rand()) / (RAND_MAX / 1.5f);
            m_pTransform->setLocalScale(glm::vec3(scaleVar));
        }

        this->setLayer(ELayers::ASTEROID_LAYER);

        if (m_pTransform) {
         m_pTransform->getWorldMatrix(); 
        }

        //Velocità casuale
        m_velocity = 1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 15.0f));
        // Rotazione casuale
        m_rotationSpeed = 30.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 60.0f));
    }

    void Asteroid::update(float dt) {
        if (!m_pTransform) return;

        // movimento in avanti (verso il player)
        Vector3 currentPos = m_pTransform->getWorldPosition();
        currentPos.z += m_velocity * dt;
        m_pTransform->setWorldPosition(currentPos);

        // 2. Rotazione su se stesso (effetto visivo)
        m_pTransform->rotateLocal(m_rotationSpeed * dt, m_rotationAxis);

        // 3. Controllo Uscita Schermo (Riciclo)
        if (currentPos.z > m_despawnZ) {
            destroy();
        }
    }

    void Asteroid::onCollisionEnter(Collider* col) {
        SPACE_ENGINE_INFO("PlayerShip Collision onEnter Called with Collider: {}", reinterpret_cast<std::uintptr_t>(col));
        if(col->gameObj->getLayer() == ELayers::PLAYER_LAYER)
        {
            if (auto* audioMgr = pScene->getAudioManager()) 
                audioMgr->PlaySound("asteroid_explosion");
            pScene->requestDestroy(this);
        }
        else if(col->gameObj->getLayer() == ELayers::BULLET_PLAYER_LAYER)
        {
            m_pSub->notifyPoints(*this, m_score);
            if (auto* audioMgr = pScene->getAudioManager()) 
                audioMgr->PlaySound("asteroid_explosion");
            pScene->requestDestroy(this);
        }
    }
}