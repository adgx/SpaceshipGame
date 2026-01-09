#include "bullet.h"
#include "collisionDetection.h"
#include "Asteroid.h"
#include "EnemyShip.h"
#include "scene.h"

namespace SpaceEngine
{
    Bullet::Bullet(Scene* pScene, std::string filePathModel):GameObject(pScene)
    {
        m_pMesh = MeshManager::loadMesh(filePathModel);
        m_pTransform = new Transform();
        m_pCollider = new Collider(this);
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
        //change for the enemy 
        if(Math::fabs(m_pTransform->getLocalPosition().z) > m_distCulling)
        {
            pScene->requestDestroy(this);
        }
    }

    void Bullet::onCollisionEnter(Collider* col)
    {
        SPACE_ENGINE_INFO("Bullet Collision onEnter Called with Collider: {}", reinterpret_cast<std::uintptr_t>(col));
        if(col->gameObj->getLayer() == ELayers::ENEMY_LAYER)
        {
            SpaceScene::pScoreSys->onNotify(*col->gameObj, 100);
            SPACE_ENGINE_INFO("Bullet hit an Enemy!");
        }
        if(col->gameObj->getLayer() == ELayers::ASTEROID_LAYER){
            SPACE_ENGINE_INFO("Bullet hit an Asteroid!");
            SpaceScene::pScoreSys->onNotify(*col->gameObj, 10);
            pScene->requestDestroy(col->gameObj);
        }
        pScene->requestDestroy(this);
    }
}