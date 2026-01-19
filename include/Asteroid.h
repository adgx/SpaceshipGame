#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

#include "gameObject.h"
#include "renderer.h"
#include "scene.h"

namespace SpaceEngine {

    class Asteroid : public GameObject {
    public:
        Asteroid(Scene* pScene, std::string filePathModel);
        virtual ~Asteroid();

        void Init(Vector3 startPos);
        
        virtual void update(float dt) override;
        virtual void onCollisionEnter(Collider* col) override;

        inline void SetSpawnArea(float width, float height) {
            m_spawnRangeX = width / 2.0f;
            m_spawnRangeY = height / 2.0f;
        }

    private:
        PointSubject* m_pSub; 
        Vector3 m_rotationAxis;

        float m_velocity;
        float m_rotationSpeed;
        float m_spawnRangeX;
        float m_spawnRangeY;
        float m_spawnZ, m_despawnZ;

        int m_score = 50;

        void Spawn();
    };

}