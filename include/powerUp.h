#pragma once
#include "gameObject.h"

namespace SpaceEngine {

    enum class PowerUpType {
        RAPID_FIRE,   //spara + veloce
        BOMB,         //distrugge tutti i nemici
        HEALTH        //recupera una vita
    };

    class PowerUp : public GameObject {
    public:
        PowerUp(Scene* pScene, PowerUpType type, std::string modelPath);
        virtual ~PowerUp() = default;

        void Init(Vector3 position);
        void update(float dt) override;
        void onCollisionEnter(Collider* other) override;

    private:
        PowerUpType m_type;
        float m_lifeTime = 10.0f;
        float m_velocity; 
        float m_despawnZ;
    };
}