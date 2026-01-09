#pragma once
#include "gameObject.h"

namespace SpaceEngine
{
    class Bullet : public GameObject
    {
        public:
            Bullet(Scene* pScene, std::string filePathModel);
            ~Bullet() = default;

            void Fire(Vector3 position, Vector3 direction, Vector3 rotation, float speed);
            void update(float dt) override;
            void onCollisionEnter(Collider* col) override;

        private:
            float m_vel = 5.f;
            float m_distCulling = 200.0f;
            bool m_useCustomDirection = false;
            Vector3 m_moveDirection;
    };
}