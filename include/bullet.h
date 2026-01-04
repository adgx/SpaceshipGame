#pragma once
#include "gameObject.h"

namespace SpaceEngine
{
    class Bullet : public GameObject
    {
        public:
            Bullet(Scene* pScene, std::string filePathModel);
            ~Bullet() = default;
            void update(float dt) override;
            void onCollisionEnter(Collider* col) override;

        private:
            float m_vel = 4.f;
            float m_distCulling = 35.f;
    };
}