#include "gameObject.h"
#include "scene.h"
#include "collisionDetection.h"
#include "log.h"

namespace SpaceEngine
{
    GameObject::GameObject(const GameObject& other)
    {
        pScene = other.pScene;
        m_pMesh = other.m_pMesh;
        m_layer = other.m_layer;
        m_pTransform = new Transform(*other.m_pTransform);
    }

    Layers GameObject::getLayer()
    {
        return m_layer;
    }
    void GameObject::destroy()
    {
        if(!pendingDestroy)
        {
            pendingDestroy = true;
            pScene->requestDestroy(this);
        }
    }

    void GameObject::update(float dt)
    {

    }

    void GameObject::fixedUpdate(float fixed_dt)
    {

    }

    void GameObject::onCollisionEnter(Collider* col)
    {

    }

    GameObject::GameObject(Scene* pScene)
    {
        this->pScene = pScene;
    }

    GameObject::~GameObject()
    {
        delete m_pCollider;
        delete m_pTransform;
    }
}