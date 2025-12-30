#pragma once
#include <array>
#include <memory>
#include <unordered_map>

enum 
{
    //Mouse
    SPACE_ENGINE_MOUSE_BUTTON_FIRST,
    SPACE_ENGINE_MOUSE_BUTTON_LEFT = SPACE_ENGINE_MOUSE_BUTTON_FIRST,
    SPACE_ENGINE_MOUSE_BUTTON_RIGHT,
    SPACE_ENGINE_MOUSE_BUTTON_MIDDLE,
    SPACE_ENGINE_MOUSE_BUTTON_LAST = SPACE_ENGINE_MOUSE_BUTTON_MIDDLE,
    //Keyboard
    SPACE_ENGINE_KEY_BUTTON_0=48,
    SPACE_ENGINE_KEY_BUTTON_1=49,
    SPACE_ENGINE_KEY_BUTTON_2=50,
    SPACE_ENGINE_KEY_BUTTON_3=51,
    SPACE_ENGINE_KEY_BUTTON_4=52,
    SPACE_ENGINE_KEY_BUTTON_5=53,
    SPACE_ENGINE_KEY_BUTTON_6=54,
    SPACE_ENGINE_KEY_BUTTON_7=55,
    SPACE_ENGINE_KEY_BUTTON_8=56,
    SPACE_ENGINE_KEY_BUTTON_9=57,
    SPACE_ENGINE_KEY_BUTTON_A=65,
  	SPACE_ENGINE_KEY_BUTTON_B=66,
  	SPACE_ENGINE_KEY_BUTTON_C=67,
  	SPACE_ENGINE_KEY_BUTTON_D=68,
  	SPACE_ENGINE_KEY_BUTTON_E=69,
  	SPACE_ENGINE_KEY_BUTTON_F=70,
  	SPACE_ENGINE_KEY_BUTTON_G=71,
  	SPACE_ENGINE_KEY_BUTTON_H=72,
  	SPACE_ENGINE_KEY_BUTTON_I=73,
  	SPACE_ENGINE_KEY_BUTTON_J=74,
  	SPACE_ENGINE_KEY_BUTTON_K=75,
  	SPACE_ENGINE_KEY_BUTTON_L=76,
  	SPACE_ENGINE_KEY_BUTTON_M=77,
  	SPACE_ENGINE_KEY_BUTTON_N=78,
  	SPACE_ENGINE_KEY_BUTTON_O=79,
  	SPACE_ENGINE_KEY_BUTTON_P=80,
  	SPACE_ENGINE_KEY_BUTTON_Q=81,
  	SPACE_ENGINE_KEY_BUTTON_R=82,
  	SPACE_ENGINE_KEY_BUTTON_S=83,
  	SPACE_ENGINE_KEY_BUTTON_T=84,
  	SPACE_ENGINE_KEY_BUTTON_U=85,
  	SPACE_ENGINE_KEY_BUTTON_V=86,
  	SPACE_ENGINE_KEY_BUTTON_W=87,
  	SPACE_ENGINE_KEY_BUTTON_X=88,
  	SPACE_ENGINE_KEY_BUTTON_Y=89,
    SPACE_ENGINE_KEY_BUTTON_Z=90,
    //Function keys
    SPACE_ENGINE_KEY_BUTTON_ESCAPE=256,
    SPACE_ENGINE_KEY_BUTTON_ENTER=257,
    SPACE_ENGINE_KEY_TAB=258,
    SPACE_ENGINE_KEY_BUTTON_BACKSPACE=259,
    SPACE_ENGINE_KEY_BUTTON_SPACE=32, 
    //Joystick buttons
    SPACE_ENGINE_JK_BUTTON_A=0, 
    SPACE_ENGINE_JK_BUTTON_B=1,
    SPACE_ENGINE_JK_BUTTON_X=2,
    SPACE_ENGINE_JK_BUTTON_Y=3,
    SPACE_ENGINE_JK_BUTTON_LEFT_BUMPER=4,
    SPACE_ENGINE_JK_BUTTON_RIGHT_BUMPER=5,
    SPACE_ENGINE_JK_BUTTON_BACK=6,
    SPACE_ENGINE_JK_BUTTON_START=7,
    SPACE_ENGINE_JK_BUTTON_GUIDE=8,
    SPACE_ENGINE_JK_BUTTON_LEFT_THUMB=9,
    SPACE_ENGINE_JK_BUTTON_RIGHT_THUMB=10,
    SPACE_ENGINE_JK_BUTTON_UP=11,
    SPACE_ENGINE_JK_BUTTON_RIGHT=12,
    SPACE_ENGINE_JK_BUTTON_DOWN=13,
    SPACE_ENGINE_JK_BUTTON_LEFT=14,
    SPACE_ENGINE_JK_BUTTON_FIRST=SPACE_ENGINE_JK_BUTTON_A,
    SPACE_ENGINE_JK_BUTTON_LAST=SPACE_ENGINE_JK_BUTTON_LEFT,
    SPACE_ENGINE_JK_BUTTON_CROSS=SPACE_ENGINE_JK_BUTTON_A,
    SPACE_ENGINE_JK_BUTTON_CIRCLE=SPACE_ENGINE_JK_BUTTON_B,
    SPACE_ENGINE_JK_BUTTON_SQUARE=SPACE_ENGINE_JK_BUTTON_X,
    SPACE_ENGINE_JK_BUTTON_TRIANGLE=SPACE_ENGINE_JK_BUTTON_Y,
    //Axis
    SPACE_ENGINE_JK_AXIS_LEFT_X=0,
    SPACE_ENGINE_JK_AXIS_LEFT_Y=1,
    SPACE_ENGINE_JK_AXIS_RIGHT_X=2,
    SPACE_ENGINE_JK_AXIS_RIGHT_Y=3,
    SPACE_ENGINE_JK_AXIS_LEFT_TRIGGER=4,
    SPACE_ENGINE_JK_AXIS_RIGHT_TRIGGER=5,
    SPACE_ENGINE_JK_AXIS_FIRST=SPACE_ENGINE_JK_AXIS_LEFT_X,
    SPACE_ENGINE_JK_AXIS_LAST=SPACE_ENGINE_JK_AXIS_RIGHT_TRIGGER
};

namespace SpaceEngine
{
    enum class EAppState
    {
        START,
        PAUSE,
        RUN,
        CLOSE,
        COUNT
    };

    enum class EInputType
    {
        SPACE_ENGINE_INPUT_MOUSE,
        SPACE_ENGINE_INPUT_KEYBOARD,
        SPACE_ENGINE_INPUT_JOYSTICK,
        COUNT
    };
    class Command 
    {
        public:
          virtual ~Command() = default;
          virtual void execute(void* actor) = 0;
    };

    struct InputBinding
    {
        int inputCode;
        EInputType inputType;
        Command* command;
    };

    class InputHandler
    {
        public:
            void handleInput();
            int bindCommand(EAppState state, void* obj, const InputBinding& inputBind);
        private:
        std::unordered_map<EAppState, std::unordered_map<void*, std::vector<InputBinding>>> m_bindings;
    };

    class InputManager
    {
        public:
            void Initialize();
            void Update();
            void Shutdown();
    };
    class Mouse
    {
        public:
            static void init();
            inline static int getPosX() { return x;}
            inline static int getPosY() {return y;}
            static void hideCursor();
            static void showCursor();
            static bool isHidenCursor();
            static bool button(int id);
            static bool buttonDown(int id);
            static bool buttonUp(int id);

            static std::array<bool, SPACE_ENGINE_MOUSE_BUTTON_LAST+1> buttons;
            static std::array<bool, SPACE_ENGINE_MOUSE_BUTTON_LAST+1> buttonsLast;
            static int x;
            static int y;
        private:
            static bool cursorHideState;
    };

    class Keyboard
    {
        public:
        static void init();
        static std::unordered_map<int, bool> keys;
        static std::unordered_map<int, bool> keysLast;
        static bool key(int id);
        static bool keyDown(int id);
        static bool keyUp(int id);
    };

    class Joystick
    {
        public:
            static void init();
            static void update();
            static void destroy();  
            static bool button(int id);
            static bool buttonDown(int id);
            static bool buttonUp(int id);
            static float axis(int id);
            static void setDeadzone(bool flag);
            static void setClamp(bool flag);
            static bool getDeadzone();
            static bool getClamp();

            
        private:
            struct Controller
            {
                int cID = -1;
                std::array<bool, SPACE_ENGINE_JK_BUTTON_LAST+1> buttons;
                std::array<bool, SPACE_ENGINE_JK_BUTTON_LAST+1> buttonsLast;
                std::array<float, SPACE_ENGINE_JK_AXIS_LAST+1> axies;
                std::array<float, SPACE_ENGINE_JK_AXIS_LAST+1> axiesLast;
            };
            static bool deadzone;
            static bool clamp;
            constexpr static float deadzoneVal = 0.35f; 
            static std::unique_ptr<Controller> controller;
    };
}