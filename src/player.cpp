#include "player.h"
#include "mesh.h"

namespace SpaceEngine
{
    Player::Player(std::string fileNameModel)
    {
        m_vecTransform.push_back(new Transform());
        m_pMesh = MeshManager::loadMesh(fileNameModel);
        //debug
        //switch the shader for material to show a simple texture on the mesh
        BaseMaterial* pMat = m_pMesh->getMaterialBySubMeshIndex(0);
        pMat->pShader = ShaderManager::findShaderProgram("simpleTex");

    }

    void Player::update(float dt)
    {
        m_vecTransform[0]->rotateGlobal(angularVY * dt, Vector3(0.f, 1.f, 0.f));
    }
}