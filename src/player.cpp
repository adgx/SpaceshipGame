#include "player.h"
#include "mesh.h"
#include "CollisionDetection.h"

namespace SpaceEngine
{
    Player::Player(Scene* pScene, std::string fileNameModel):GameObject(pScene)
    {
        m_pTransform = new Transform();
        m_pMesh = MeshManager::loadMesh(fileNameModel);
        //debug
        //switch the shader for material to show a simple texture on the mesh
        BaseMaterial* pMat = m_pMesh->getMaterialBySubMeshIndex(0);
        pMat->pShader = ShaderManager::findShaderProgram("simpleTex");
        m_pCollider = new Collider(this);
    }

    void Player::update(float dt)
    {
        m_pTransform->rotateGlobal(angularVY * dt, Vector3(0.f, 1.f, 0.f));
    }

    void Player::fixedUpdate(float fixed_dt)
    {
        m_pTransform->translateGlobal(Vector3(0.f, 0.f, -0.1*fixed_dt));
    }

    void Player::onCollisionEnter(Collider* col) 
    {
        SPACE_ENGINE_INFO("GameObject:{} Collision onEnter Called!", reinterpret_cast<std::uintptr_t>(this));
    }
    
}