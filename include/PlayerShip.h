#pragma once
#include "gameObject.h"
#include "utils/utils.h"

namespace SpaceEngine {

    class PlayerShip : public GameObject {
    public:
        PlayerShip(Scene *scene);
        virtual ~PlayerShip();

        // Inizializza buffers (VAO/VBO), texture e collider
        void Init();

        // Gestisce l'input e aggiorna la posizione
        virtual void update(float dt) override;

        void Render(unsigned int shaderProgramID);

        virtual void onCollisionEnter(Collider* col) override;

    private:
        float m_speed;

        // Limiti di movimento
        float m_limitX;
        float m_limitY;

        unsigned int m_textureID;
        unsigned int m_VAO, m_VBO, m_EBO;

        float m_shootCooldown;

        void HandleInput(float dt);
        void InitMesh(); // Funzione interna per creare il rettangolo
    };
}