#include "bullet.h"
#include "collisionDetection.h"

namespace SpaceEngine
{
    Bullet::Bullet(Scene* pScene, std::string filePathModel):GameObject(pScene)
    {
        m_pMesh = MeshManager::loadMesh(filePathModel);
        BaseMaterial* pMat = m_pMesh->getMaterialBySubMeshIndex(0);
        pMat->pShader = ShaderManager::findShaderProgram("simpleTex");
        m_pTransform = new Transform();
        m_pCollider = new Collider(this);
    }

    void Bullet::update(float dt)
    {
        Vector3 pos = m_pTransform->getWorldPosition() + m_pTransform->forwardLocal() * m_vel * dt;
        m_pTransform->setWorldPosition(pos); 
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