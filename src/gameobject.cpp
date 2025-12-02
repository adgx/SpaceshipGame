#include "gameObject.h"
#include "scene.h"
#include "log.h"

namespace SpaceEngine
{
    void GameObject::destroy()
    {
        scene->requestDestroy(this);
    }

    int GameObject::getNumInstances()
    {
        return m_numInstances;
    }
}