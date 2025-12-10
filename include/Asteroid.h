#pragma once
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include "gameObject.h"

namespace SpaceEngine {

    class Asteroid : public GameObject {
    public:
        Asteroid(Scene *scene);
        virtual ~Asteroid();

        void Init(unsigned int textureID);
        
        virtual void update(float dt);
        virtual void onCollisionEnter(Collider* col) override;

        void Render(unsigned int shaderProgramID);

        void SetSpawnArea(float width, float height) {
            m_spawnRangeX = width / 2.0f;
            m_spawnRangeY = height / 2.0f;
        }

    private:
        glm::vec3 m_rotationAxis;
        float m_velocity;
        float m_rotationSpeed;

        float m_spawnRangeX;
        float m_spawnRangeY;
        
        float m_spawnZ, m_despawnZ;

        unsigned int m_textureID;
        unsigned int m_VAO, m_VBO, m_EBO;

        void InitMesh();
        void Spawn();
    };

} // namespace SpaceEngine