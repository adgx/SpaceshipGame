#include "bullet.h"
#include "collisionDetection.h"
#include "scene.h"

namespace SpaceEngine
{
    Bullet::Bullet(Scene* pScene, std::string filePathModel):GameObject(pScene)
    {
        m_pMesh = MeshManager::loadMesh(filePathModel);
        m_pTransform = new Transform();
        m_pCollider = new Collider(this);
    }

    void Bullet::update(float dt)
    {
        Vector3 pos = m_pTransform->getWorldPosition() + (-m_pTransform->forwardWorld()) * m_vel * dt;
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
        if(col->gameObj->getLayer() == ELayers::ENEMY_LAYER || col->gameObj->getLayer() == ELayers::ASTEROID_LAYER)
        {
            SPACE_ENGINE_INFO("Bullet Collision");
        }
    }


}