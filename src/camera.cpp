#include "camera.h"
#include "utils/utils.h"
#include "managers/windowManager.h"

namespace SpaceEngine
{
    //camera
    BaseCamera::BaseCamera() : nearPlane(0.1f), farPlane(100.f){}
    
    BaseCamera::BaseCamera(float nearPlane, float farPlane) : nearPlane(nearPlane), farPlane(farPlane){}

    Matrix4 BaseCamera::getViewMatrix() const
    {
        return Math::inverse(transf.getWorldMatrix());
    }

    //PerspectiveCamera
    PerspectiveCamera::PerspectiveCamera() : fov(45), aspectRatio(WindowManager::aspectRatio)
    {
        projection = Math::perspectiveMatrix4(fov, aspectRatio, nearPlane, farPlane);
    }

    PerspectiveCamera::PerspectiveCamera(float fov, float nearPlane, float farPlane) : BaseCamera(nearPlane, farPlane), fov(fov), aspectRatio(WindowManager::aspectRatio)
    {
        projection = Math::perspectiveMatrix4(fov, aspectRatio, nearPlane, farPlane);
    }

    PerspectiveCamera::PerspectiveCamera(float fov, float aspectRatio, float nearPlane, float farPlane) : BaseCamera(nearPlane, farPlane), fov(fov), aspectRatio(aspectRatio)
    {
        projection = Math::perspectiveMatrix4(fov, aspectRatio, nearPlane, farPlane);
    }

    void PerspectiveCamera::setNearFarPlane(float nearPlane, float farPlane)
    {
        this->nearPlane = nearPlane;
        this->farPlane = farPlane;
        projection = Math::perspectiveMatrix4(fov, aspectRatio, nearPlane, farPlane);
    }

    void PerspectiveCamera::setFov(float fov)
    {
        this->fov = fov;
        projection = Math::perspectiveMatrix4(fov, aspectRatio, nearPlane, farPlane);
    }

    void PerspectiveCamera::setAspectRatio(float aspectRatio)
    {
        this->aspectRatio = aspectRatio;
        projection = Math::perspectiveMatrix4(fov, aspectRatio, nearPlane, farPlane);
    }

    //OrthoCamera
    OrthoCamera::OrthoCamera(float width, float height, float nearPlane, float farPlane): BaseCamera(nearPlane, farPlane), width(width), height(height)
    {
        projection = Math::orthoMatrix4(width, height, nearPlane, farPlane);
    }

    OrthoCamera::OrthoCamera(float nearPlane, float farPlane): BaseCamera(nearPlane, farPlane)
    {
        width = WindowManager::aspectRatio;
        height = 1.f;
        projection = Math::orthoMatrix4(width, height, nearPlane, farPlane);
    }

    void OrthoCamera::setNearFarPlane(float nearPlane, float farPlane)
    {
        this->nearPlane = nearPlane;
        this->farPlane = farPlane;
        projection = Math::orthoMatrix4(width, height, nearPlane, farPlane);
    }

    void OrthoCamera::setWidthHeight(float width, float height)
    {
        this->width = width;
        this->height = height;
        projection = Math::orthoMatrix4(width, height, nearPlane, farPlane);
    }
}