#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
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
    
    namespace Math
    {
        inline Matrix4 aiMat4_2_Mat4(const aiMatrix4x4& m)
        {
            Matrix4 res;
            std::memcpy(&res, &m, sizeof(Matrix4));
            return res;
        }

        inline Matrix4 inverse(Matrix4 m)
        {
            return glm::inverse(m);
        }
    }

    class Utils
    {
        public:
            static std::string getDirFromFilename(const std::string& fileName);
            static std::string getFullPath(const std::string& dir, const aiString& path);
            static bool fileExists(const std::string &fileName); 
            static std::string getFileNameFormPath(const std::string& path);
            static std::string getExtension(const char *name); 
    };

}