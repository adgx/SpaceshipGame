#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include "gameObject.h"
#include "scene.h"
#include "collisionDetection.h"
#include "renderer.h"

namespace SpaceEngine {

    class Asteroid : public GameObject {
    public:
        Asteroid(Scene* pScene, std::string filePathModel);
        virtual ~Asteroid();

        void Init(Vector3 startPos);
        
        virtual void update(float dt) override;
        virtual void onCollisionEnter(Collider* col) override;

        RenderObject getRenderObject();

        void SetSpawnArea(float width, float height) {
            m_spawnRangeX = width / 2.0f;
            m_spawnRangeY = height / 2.0f;
        }

    private:
        Vector3 m_rotationAxis;
        float m_velocity;
        float m_rotationSpeed;

        float m_spawnRangeX;
        float m_spawnRangeY;
        
        float m_spawnZ, m_despawnZ;

        void Spawn();
    };

}