#include "powerUp.h"
#include "playerShip.h"
#include "scene.h"
#include "mesh.h"
#include "camera.h" // Necessario per il billboard

namespace SpaceEngine {

    PowerUp::PowerUp(Scene* pScene, PowerUpType type, std::string modelPath, std::string matName) 
        : GameObject(pScene), m_type(type) {
  
        BaseMaterial* pMat;
        
        m_pMesh = MeshManager::loadMesh(modelPath);
        pMat = MaterialManager::findMaterial(matName);

        m_pMesh->bindMaterialToSubMeshIndex(0, pMat); 
        m_pTransform = new Transform();
        m_pCollider = new Collider(this);
        m_layer = ELayers::POWERUP_LAYER; 

        m_velocity = 20.0f;
        m_despawnZ = 20.0f;
    }

    void PowerUp::Init(Vector3 position) {
        if(m_pTransform) {
            m_pTransform->setWorldPosition(position);
            m_pTransform->setLocalScale(Vector3(2.0f)); 

            m_pTransform->setLocalRotation(glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)));
        }
    }

    void PowerUp::update(float dt) {
        if (!m_pTransform) return;
        Vector3 currentPos = m_pTransform->getWorldPosition();
        currentPos.z += m_velocity * dt;
        m_pTransform->setWorldPosition(currentPos);
        /*if (m_pTransform && pScene) {
            BaseCamera* cam = pScene->getActiveCamera();
            if (cam) {
                Vector3 camPos = cam->transf.getWorldPosition();
                
                m_pTransform->lookAt(camPos);
            }
            if (m_pTransform) {
                float speed = 20.0f; 
                Vector3 currentPos = m_pTransform->getLocalPosition();
                currentPos.z += speed * dt; 
                m_pTransform->setLocalPosition(currentPos);
            }
        }*/

        if (currentPos.z > m_despawnZ) {
            pScene->requestDestroy(this);
        }
    }

    void PowerUp::onCollisionEnter(Collider* other) {
        if (other->gameObj->getLayer() == ELayers::PLAYER_LAYER) {
            
            PlayerShip* pPlayer = dynamic_cast<PlayerShip*>(other->gameObj);
            
            if (pPlayer) {
                switch (m_type) {
                    case PowerUpType::HEALTH:
                        pPlayer->Heal(); 
                        if (auto* am = pScene->getAudioManager()) {
                            am->PlaySound("heal"); 
                        }
                        break;
                    
                    case PowerUpType::RAPID_FIRE:
                        pPlayer->ActivateRapidFire(5.0f);
                        if (auto* am = pScene->getAudioManager()) {
                            am->PlaySound("rapid_fire"); 
                        }
                        break;

                    case PowerUpType::BOMB:
                        if (SpaceScene* pSpaceScene = dynamic_cast<SpaceScene*>(pScene)) {
                            pSpaceScene->TriggerBomb();
                            if (auto* am = pScene->getAudioManager()) {
                                am->PlaySound("bomb"); 
                            }
                        }
                        break;
                }

                pScene->requestDestroy(this);
            }
        }
    }
}