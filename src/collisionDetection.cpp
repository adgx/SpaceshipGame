#include "collisionDetection.h"

namespace SpaceEngine
{
    void PhysicsManager::Initialization()
    {

    }

    void PhysicsManager::Shutdown()
    {

    }

    void PhysicsManager::AddCollider(Collider* col)
    {
        if(col)
        {
            lColliders.push_back(col);
            grid.AddColliderToHGrid(col);
            SPACE_ENGINE_INFO("Added collider");
        }
        else SPACE_ENGINE_FATAL("AddCollider: col nullptr");
    }

    void PhysicsManager::RemoveCollider(Collider* col)
    {
        if(col)
        {
            lColliders.remove(col);
            grid.RemoveObjectFromGrid(col);
            SPACE_ENGINE_INFO("Removed collider");
        }
        else SPACE_ENGINE_FATAL("RemoveCollider: col nullptr");
    }
    
    void PhysicsManager::AddColliders(const std::list<Collider*>& lCols)
    {
        lColliders.insert(lColliders.end(), lCols.begin(), lCols.end());
        
        for(Collider* col : lCols)
        {
            grid.AddColliderToHGrid(col);
        }
    }

    void PhysicsManager::RemoveColliders(const std::list<Collider*>& lCols)
    {
        for(auto col : lCols) RemoveCollider(col);
    }

    void PhysicsManager::HandleCollisionEvents()
    {
        for (const auto& pair : currCollisions)
        {
            if (prevCollisions.find(pair) == prevCollisions.end())
            {
                if(!pair.a->gameObj->pendingDestroy && !pair.b->gameObj->pendingDestroy) {
                    pair.a->gameObj->onCollisionEnter(pair.b);
                    pair.b->gameObj->onCollisionEnter(pair.a);
                }
            }
        }
    }

    
    void PhysicsManager::Step(float fixed_dt)
    {
        currCollisions.clear();
        
        for(Collider* col : lColliders)
        {
            if(!col->gameObj->pendingDestroy)
            {
                col->gameObj->fixedUpdate(fixed_dt);
                Vector3 oldPos = col->pos;
                float oldMaxSide = col->bbox.maxSide();

                col->updateGlobalBounds();

                if(col->pos != oldPos || abs(col->bbox.maxSide() - oldMaxSide) > 0.01f)
                {
                    grid.RemoveObjectFromGrid(col);
                    grid.AddColliderToHGrid(col);
                }
                /*
                //verify if the pos change, if yes update(remove and insert) the hgrid
                Vector3 pos = col->gameObj->getComponent<Transform>()->getWorldPosition();
                if(col->pos != pos)
                {
                    col->pos = pos;
                    grid.RemoveObjectFromGrid(col);
                    grid.AddColliderToHGrid(col);
                }*/
            }
        }
        
        grid.tick++;
        for(Collider* col : lColliders)
        {
            if(!col->gameObj->pendingDestroy)
                grid.CheckObjAgainstGrid(col, currCollisions);
        }

        HandleCollisionEvents();
        prevCollisions = currCollisions;
        //check collision on hgrid
    }


}
