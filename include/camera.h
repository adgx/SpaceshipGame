#pragma once 

#include "transform.h"
#include "player.h"

namespace SpaceEngine
{
    class BaseCamera
    {
        public:
            BaseCamera();
            BaseCamera(float nearPlane, float farPlane);
            ~BaseCamera() = default;
            Matrix4 getViewMatrix();
            Matrix4 getProjectionMatrix(){ return projection;};
            Transform transf;
        protected:
            Matrix4 projection;
            float nearPlane;
            float farPlane;
    };

    class PerspectiveCamera: public BaseCamera
    {
        public:
            PerspectiveCamera();
            PerspectiveCamera(float nearPlane, float farPlane);
            PerspectiveCamera(float fov, float nearPlane, float farPlane);
            PerspectiveCamera(float fov, float aspectRatio, float nearPlane, float farPlane);
            ~PerspectiveCamera() = default;
            
            void setNearFarPlane(float nearPlane, float farPlane);
            void setFov(float fov);
            void setAspectRatio(float aspectRatio);
        private:
            //degree
            float fov;
            float aspectRatio;
        
        
    };
    class OrthoCamera : public BaseCamera
    {
        public:
            OrthoCamera(float width, float height, float nearPlane, float farPlane);
            OrthoCamera(float nearPlane, float farPlane);
            ~OrthoCamera() = default;

            void setNearFarPlane(float nearPlane, float farPlane);
            void setWidthHeight(float width, float height);
        private: 
            float width;
            float height;
    };
}