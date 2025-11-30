#pragma once
#include "mesh.h"
#include "transform.h"
#include "material.h"
#include "vector"
#include <string> 

namespace SpaceEngine
{
    class GameObject 
    {
        GameObject() = default;
        ~GameObject() = default;
        protected:
        GameObject(std::string filePathModel);
        int mNumInstances = 0;
        std::vector<Transform*> m_vecTransform;
        Mesh* m_pMesh = nullptr;
        BaseMaterial* m_pMat = nullptr;
        //TODO: add componet for the physics 
    };

    
}
