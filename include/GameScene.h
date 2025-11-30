#pragma once
#include "utils/stb_image.h"
#include <vector>
#include <string>
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include "log.h"
using namespace std;

namespace SpaceEngine{
    class GameScene{
    public:
        GameScene();
        ~GameScene();

        unsigned int LoadCubemap(vector<string> faces);
        void Init();
        void Render();
        void Update();
    private:
        unsigned int cubemapTexture;
        vector<string> faces = {            //TODO: sistemare i path con i file giusti
            "assets/textures/skybox/right.jpg",
            "assets/textures/skybox/left.jpg",
            "assets/textures/skybox/top.jpg",
            "assets/textures/skybox/bottom.jpg",
            "assets/texture/skybox/front.jpg",
            //"assets/textures/skybox/back.jpg"
        };
    };
}