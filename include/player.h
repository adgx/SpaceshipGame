#pragma once 

#include "gameObject.h"

namespace SpaceEngine
{
    class Player : public GameObject
    { 
        public:
            Player(std::string fileNameModel);
            void update(float dt) override;
        
        private:
            float angularVY = 10.f;
    };
}
