#pragma once
#include "gameObject.h"
#include "utils/utils.h"
#include "collisionDetection.h"
#include "renderer.h"

namespace SpaceEngine {

    class PlayerShip : public GameObject {
    public:
        PlayerShip(Scene* pScene, std::string filePathModel);
        virtual ~PlayerShip();

        void Init();

        // Gestisce l'input e aggiorna la posizione
        virtual void update(float dt) override;

        virtual void onCollisionEnter(Collider* col) override;

        RenderObject getRenderObject();

    private:
        float m_speed;

        // Limiti di movimento
        float m_limitX;
        float m_limitY;

        float m_shootCooldown;
        void HandleInput(float dt);
    };
}