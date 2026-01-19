#include "playerShip.h"
#include "inputManager.h"
#include "scene.h"
#include "app.h"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "utils/stb_image.h"

namespace SpaceEngine {

    PlayerShip::PlayerShip(Scene* pScene, std::string filePathModel):GameObject(pScene)
    {
        m_pMesh = MeshManager::loadMesh(filePathModel);
        m_pTransform = new Transform();
        m_pCollider = new Collider(this);
        m_pBullet = new Bullet(pScene, "Bullet.obj");
        m_pBullet->setOwner(ELayers::PLAYER_LAYER);
        m_pBullet->setLayer(ELayers::BULLET_PLAYER_LAYER);
        Transform* pTransformBullet = m_pBullet->getComponent<Transform>();
        //m_pBullet->setLayer(ELayers::BULLET_LAYER);
        //set the parent of the bullet's trasform and place an offset 
        pTransformBullet->rotateGlobal(90.f, {1.f, 0.f, 0.f});
        m_layer = ELayers::PLAYER_LAYER;
        m_speed = 15.f;               
        m_limitX = 7.f;              
        m_limitY = 3.5f;               
        m_shootCooldown = 0.f;   
        
        //--------------------------------------------------------
        //-------------------set the InputHandler-----------------
        //--------------------------------------------------------
        InputHandler& inputHandler = App::GetInputHandler();
        //Command init
        m_playerMoveUp = new MoveUpCommand();
        m_playerMoveDown = new MoveDownCommand();
        m_playerMoveLeft = new MoveLeftCommand();
        m_playerMoveRight = new MoveRightCommand();
        m_playerFire = new FireCommand();

        //up-joystick  
        inputHandler.bindCommand(EAppState::RUN, this, {SPACE_ENGINE_JK_BUTTON_UP, 
            EInputType::SPACE_ENGINE_INPUT_JOYSTICK, 
            m_playerMoveUp});
        //up-keyboard
        inputHandler.bindCommand(EAppState::RUN, this, {SPACE_ENGINE_KEY_BUTTON_W, 
            EInputType::SPACE_ENGINE_INPUT_KEYBOARD, 
            m_playerMoveUp});
        //down-joystick  
        inputHandler.bindCommand(EAppState::RUN, this, {SPACE_ENGINE_JK_BUTTON_DOWN, 
            EInputType::SPACE_ENGINE_INPUT_JOYSTICK, 
            m_playerMoveDown});
        //down-keyboard
        inputHandler.bindCommand(EAppState::RUN, this, {SPACE_ENGINE_KEY_BUTTON_S, 
            EInputType::SPACE_ENGINE_INPUT_KEYBOARD, 
            m_playerMoveDown});
        //left-joystick  
        inputHandler.bindCommand(EAppState::RUN, this, {SPACE_ENGINE_JK_BUTTON_LEFT, 
            EInputType::SPACE_ENGINE_INPUT_JOYSTICK, 
            m_playerMoveLeft});
        //left-keyboard
        inputHandler.bindCommand(EAppState::RUN, this, {SPACE_ENGINE_KEY_BUTTON_A, 
            EInputType::SPACE_ENGINE_INPUT_KEYBOARD, 
            m_playerMoveLeft});
        //right-joystick  
        inputHandler.bindCommand(EAppState::RUN, this, {SPACE_ENGINE_JK_BUTTON_RIGHT, 
            EInputType::SPACE_ENGINE_INPUT_JOYSTICK, 
            m_playerMoveRight});
        //right-keyboard
        inputHandler.bindCommand(EAppState::RUN, this, {SPACE_ENGINE_KEY_BUTTON_D, 
            EInputType::SPACE_ENGINE_INPUT_KEYBOARD, 
            m_playerMoveRight});
        //fire-joystick  
        inputHandler.bindCommand(EAppState::RUN, this, {SPACE_ENGINE_JK_BUTTON_A, 
            EInputType::SPACE_ENGINE_INPUT_JOYSTICK, 
            m_playerFire});
        //fire-keyboard
        inputHandler.bindCommand(EAppState::RUN, this, {SPACE_ENGINE_KEY_BUTTON_SPACE, 
            EInputType::SPACE_ENGINE_INPUT_KEYBOARD, 
            m_playerFire});
    }

    PlayerShip::~PlayerShip() 
    {

        InputHandler& inputHandler = App::GetInputHandler();
        inputHandler.clearBindingsFor(this);
        
        delete m_playerMoveDown;
        delete m_playerMoveLeft;
        delete m_playerMoveRight;
        delete m_playerMoveUp;
        delete m_playerFire;
    }

    void PlayerShip::Init() 
    {
        if (m_pTransform) 
        {
            m_pTransform->setLocalPosition(Vector3(0.0f, 0.0f, -8.0f));
            m_pTransform->setLocalScale(Vector3(1.0f));
            m_pTransform->rotateLocal(180, {0.f,1.f, 0.f});
            m_pTransform->getWorldMatrix();
        }     
    }

    void PlayerShip::update(float dt) {
        m_dt = dt;
        //HandleInput(dt);

        if(m_shootCooldown > 0.0f) {
            m_shootCooldown -= dt;
        }
    }

    void PlayerShip::onCollisionEnter(Collider* col) 
    {
        SPACE_ENGINE_INFO("PlayerShip Collision onEnter Called with Collider: {}", reinterpret_cast<std::uintptr_t>(col));
        if(col->gameObj->getLayer() == ELayers::ENEMY_LAYER || col->gameObj->getLayer() == ELayers::ASTEROID_LAYER || col->gameObj->getLayer() == ELayers::BULLET_ENEMY_LAYER)
        {
            SPACE_ENGINE_INFO("Layer nemico confermato! Salute attuale: {}", m_health);
        
            if(m_health > 0)
            {
                m_health--;
                if (auto* audioMgr = pScene->getAudioManager()) {
                    audioMgr->PlaySound("lose_hp");
                }
                SPACE_ENGINE_INFO("Danno subito! Nuova salute: {}", m_health);

                if (SpaceScene* pSpaceScene = dynamic_cast<SpaceScene*>(pScene)) {
                    SPACE_ENGINE_INFO("Chiamata a removeHealthIcon...");
                    pSpaceScene->removeHealthIcon();
                } else {
                    SPACE_ENGINE_ERROR("ERRORE: pScene non è una SpaceScene!");
                }

                if(m_health <= 0)
                {
                    SPACE_ENGINE_INFO("GAME OVER - PlayerShip distrutta!");
                    //to avoid 
                    //if(pScene) pScene->requestDestroy(this);
                    m_pMesh = nullptr;
                    if (auto* audioMgr = pScene->getAudioManager()) {
                        audioMgr->PlaySound("player_explosion");
                        audioMgr->PlaySound("game_over");
                    }
                }
            }
        }
    }

    void PlayerShip::Fire()
    {
        if(m_shootCooldown <= 0.0f) {
            if(pScene){
                 pScene->requestInstantiate(m_pBullet)->getComponent<Transform>()->setWorldPosition(m_pTransform->getWorldPosition());
                if (auto* audioMgr = pScene->getAudioManager()) {
                    audioMgr->PlaySound("shoot_player");
                }
            }
            m_shootCooldown = 0.5f;
        } 
    }


    void PlayerShip::MoveUp()
    {        
        Vector3 m_position = m_pTransform->getWorldPosition();
        float velocity = m_speed * m_dt;
        m_position.y += velocity;

        if (m_position.y > m_limitY)  m_position.y = m_limitY;

        m_pTransform->setWorldPosition(m_position);
    }

    void PlayerShip::MoveDown()
    {
        Vector3 m_position = m_pTransform->getWorldPosition();
        float velocity = m_speed * m_dt;
        m_position.y -= velocity;

        if (m_position.y < -m_limitY)  m_position.y = -m_limitY;

        m_pTransform->setWorldPosition(m_position);
    }

    void PlayerShip::MoveLeft()
    {
        Vector3 m_position = m_pTransform->getWorldPosition();
        float velocity = m_speed * m_dt;
        m_position.x -= velocity;

        if (m_position.x < -m_limitX)  m_position.x = -m_limitX;
        
        m_pTransform->setWorldPosition(m_position);
    }

    void PlayerShip::MoveRight()
    {
        Vector3 m_position = m_pTransform->getWorldPosition();
        float velocity = m_speed * m_dt;
        m_position.x += velocity;

        if (m_position.x > m_limitX)  m_position.x = m_limitX;
        
        m_pTransform->setWorldPosition(m_position);
    }
}