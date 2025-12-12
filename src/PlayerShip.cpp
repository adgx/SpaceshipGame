#include "playerShip.h"
#include "inputManager.h"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "utils/stb_image.h"

namespace SpaceEngine {

    PlayerShip::PlayerShip(Scene* pScene, std::string filePathModel):GameObject(pScene)
    {
        m_pMesh = MeshManager::loadMesh(filePathModel);
        BaseMaterial* pMat = m_pMesh->getMaterialBySubMeshIndex(0);
        pMat->pShader = ShaderManager::findShaderProgram("simpleTex");
        m_pTransform = new Transform();
        m_pCollider = new Collider(this);

        m_speed = 15.0f;               
        m_limitX = 14.0f;              
        m_limitY = 9.0f;               
        m_shootCooldown = 0.0f;        
    }

    PlayerShip::~PlayerShip() {
    }

    void PlayerShip::Init() {
        if (m_pTransform) {
            m_pTransform->setLocalPosition(Vector3(0.0f, 0.0f, -8.0f));
        }
    }

    RenderObject PlayerShip::getRenderObject() {
        RenderObject obj;

        obj.mesh = m_pMesh;

        // calcola la Model Matrix
        glm::mat4 model = glm::mat4(1.0f);
        
        // Sposta la mesh alla posizione del GameObject 
        model = glm::translate(model, m_pTransform->getLocalPosition());

        obj.modelMatrix = model; 
        
        return obj;
    }

    void PlayerShip::update(float dt) {
        HandleInput(dt);

        if(m_shootCooldown > 0.0f) {
            m_shootCooldown -= dt;
        }
    }

    void PlayerShip::HandleInput(float dt) {

        glm::vec3 m_position = m_pTransform->getLocalPosition();
        float velocity = m_speed * dt;

        // --- MOVIMENTO ---
        if (Keyboard::key(SPACE_ENGINE_KEY_BUTTON_A)) m_position.x -= velocity;
        if (Keyboard::key(SPACE_ENGINE_KEY_BUTTON_D)) m_position.x += velocity;
        if (Keyboard::key(SPACE_ENGINE_KEY_BUTTON_W)) m_position.y += velocity;
        if (Keyboard::key(SPACE_ENGINE_KEY_BUTTON_S)) m_position.y -= velocity;

        // Impedisce alla nave di uscire dall'area di gioco
        if (m_position.x > m_limitX)  m_position.x = m_limitX;
        if (m_position.x < -m_limitX) m_position.x = -m_limitX;
        if (m_position.y > m_limitY)  m_position.y = m_limitY;
        if (m_position.y < -m_limitY) m_position.y = -m_limitY;

        m_pTransform->setLocalPosition(m_position);
    }

    void PlayerShip::onCollisionEnter(Collider* col) {
        SPACE_ENGINE_INFO("PlayerShip Collision onEnter Called with Collider: {}", reinterpret_cast<std::uintptr_t>(col));
    }
}