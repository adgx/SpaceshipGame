#include "asteroid.h"
#include <iostream>
#include <ctime>

namespace SpaceEngine {

    Asteroid::Asteroid(Scene* pScene, std::string filePathModel):GameObject(pScene) {
        m_pMesh = MeshManager::loadMesh(filePathModel);
        //BaseMaterial* pMat = m_pMesh->getMaterialBySubMeshIndex(0);
        //pMat->pShader = ShaderManager::findShaderProgram("simpleTex");
        if (m_pMesh) {
            BaseMaterial* pMat = m_pMesh->getMaterialBySubMeshIndex(0);
            if(pMat) {
                pMat->pShader = ShaderManager::findShaderProgram("simpleTex");
            }
        } else {
            SPACE_ENGINE_ERROR("CRASH EVITATO: Impossibile caricare mesh {}", filePathModel);
        }
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
    }

    void Asteroid::Init(Vector3 startPos) {
        if (m_pTransform) {
            // Usa la posizione decisa dalla Scena
            m_pTransform->setWorldPosition(startPos);
            
            // per avere asteroidi di diverse dimensioni
            float scaleVar = 1.0f + static_cast<float>(rand()) / (RAND_MAX / 1.5f);
            m_pTransform->setLocalScale(glm::vec3(scaleVar));
        }

        if (m_pTransform) {
         m_pTransform->getWorldMatrix(); 
        }

        //Velocità casuale
        m_velocity = 15.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 15.0f));
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
        if (auto* audioMgr = pScene->getAudioManager()) {
            audioMgr->PlaySound("asteroid_explosion");
        }
        destroy();
    }
}