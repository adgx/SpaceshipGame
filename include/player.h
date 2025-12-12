#pragma once 

#include "gameObject.h"

namespace SpaceEngine
{
    class Player : public GameObject
    { 
        public:
            Player(Scene* pScene, std::string fileNameModel);
            void update(float dt) override;
            void fixedUpdate(float fixed_dt) override;
            void onCollisionEnter(Collider* col) override;
        
        private:
            float angularVY = 10.f;
    };
}
