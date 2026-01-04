#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <assimp/matrix4x4.h>
#include <string>
#include <assimp/scene.h>

namespace SpaceEngine
{
    using Matrix4 = glm::mat4;
    using Matrix3 = glm::mat3;
    using Matrix2 = glm::mat2;
    using Vector2 = glm::vec2;
    using Vector3 = glm::vec3;
    using Vector4 = glm::vec4;
    using Quat = glm::quat;
    

    namespace Math
    {
        inline float fabs(float x)
        {
            uint32_t bits;
            std::memcpy(&bits, &x, sizeof(bits));
            bits &= 0x7FFFFFFF;
            std::memcpy(&x, &bits, sizeof(x));
            return x;
        }

        inline Matrix4 aiMat4_2_Mat4(const aiMatrix4x4& m)
        {
            Matrix4 res;
            std::memcpy(&res, &m, sizeof(Matrix4));
            return res;
        }

        inline Matrix3 transpose(Matrix3 m) 
        {
            return glm::transpose(m);
        }

        inline Matrix3 inverse(Matrix3 m) 
        {
            return glm::inverse(m);
        }

        inline Matrix4 inverse(Matrix4 m) 
        {
            return glm::inverse(m);
        }

        inline Matrix4 identityMatrix4()
        {
            return Matrix4(1.0f);
        }

        inline Matrix4 perspectiveMatrix4(float degree, float aspectRatio, float nearPlane, float farPlane)
        {
            return glm::perspective(glm::radians(degree), aspectRatio, nearPlane, farPlane);
        }

        inline Matrix4 orthoMatrix4(float width, float height, float nearPlane, float farPlane)
        {
            return glm::ortho(-width, height, nearPlane, farPlane);
        }

        inline float radians(float degree)
        {
            return glm::radians(degree);
        }

        inline float sqr(float a)
        {
            return a*a;
        }
    }

    class Utils
    {
        public:
            static std::string getDirFromFilename(const std::string& fileName);
            static std::string getFullPath(const std::string& dir, const aiString& path);
            static bool fileExists(const std::string &fileName); 
            static bool directoryExists(const std::string& pathDir);
            static std::string getFileNameFormPath(const std::string& path);
            static std::string getExtension(const char *name); 
            static std::string getFileNameNoExt(const std::string& filePath);
            static std::string joinPaths(const std::string& a, const std::string& b);
    };

}