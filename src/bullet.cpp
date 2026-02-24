#include "bullet.h"
#include "collisionDetection.h"
#include "Asteroid.h"
#include "EnemyShip.h"
#include "PlayerShip.h"
#include "scene.h"

namespace SpaceEngine
{
    Bullet::Bullet(Scene* pScene, std::string filePathModel):GameObject(pScene)
    {
        m_pMesh = MeshManager::loadMesh(filePathModel);
        m_pMesh->bindMaterialToSubMeshIndex(0, MaterialManager::findMaterial("BulletMat"));
        m_pTransform = new Transform();
        m_pCollider = new Collider(this);
    }

    Bullet::Bullet(const Bullet& other) : GameObject(other)
    {
        m_owner = other.m_owner;
    }


    void Bullet::Fire(Vector3 position, Vector3 direction, Vector3 rotation, float speed)
    {
        if(m_pTransform)
        {
            m_pTransform->setWorldPosition(position);
            m_pTransform->setLocalRotation(rotation);
        }
        m_moveDirection = glm::normalize(direction);
        m_useCustomDirection = true;
        m_vel = speed;
    }

    void Bullet::update(float dt)
    {   
        Vector3 moveDir;
         if (m_useCustomDirection)
        {
            // Se impostato manualmente (Enemy), usa quella direzione
            moveDir = m_moveDirection;
        }
        else{
            moveDir = -m_pTransform->forwardWorld();
            if (getLayer() == ELayers::ENEMY_LAYER)
            {
                moveDir = -moveDir; // Inverte il vettore direzione se è del nemico
            }
        }
        
        
        Vector3 pos = m_pTransform->getWorldPosition() + moveDir * m_vel * dt;
        m_pTransform->setWorldPosition(pos);
        //culling

        float currentZ = m_pTransform->getWorldPosition().z;

        if (m_layer == ELayers::BULLET_PLAYER_LAYER)
        {
            if (currentZ > 5.f || currentZ < -50.f)
            {
                pScene->requestDestroy(this);
            }
        }
        else if (m_layer == ELayers::BULLET_ENEMY_LAYER)
        {
            if (currentZ > 25.f || currentZ < -120.f)
            {
                pScene->requestDestroy(this);
            }
        }
    }

    void Bullet::onCollisionEnter(Collider* col)
    {
        if((m_layer == ELayers::BULLET_PLAYER_LAYER && col->gameObj->getLayer() == ELayers::PLAYER_LAYER) ||
            (m_layer == ELayers::BULLET_ENEMY_LAYER && col->gameObj->getLayer() == ELayers::ENEMY_LAYER)) 
        {
            return; //ignora la collisione se fatta con il proprietario
        }

        if (col->gameObj->getLayer() == ELayers::BULLET_ENEMY_LAYER || col->gameObj->getLayer() == ELayers::BULLET_PLAYER_LAYER) 
        {
            return;
        }
        
        pScene->requestDestroy(this);
        
        SPACE_ENGINE_INFO("Bullet Collision onEnter Called with Collider: {}", reinterpret_cast<std::uintptr_t>(col));
        
    }
}