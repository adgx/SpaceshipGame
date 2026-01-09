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
    using Vector2i = glm::ivec2;
    using Quat = glm::quat;
    
    //resolution used in figma
    constexpr int REF_WIDTH  = 1920;
    constexpr int REF_HEIGHT = 1080;

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
            static void applyRatioScreenRes(Vector2 anchor, Vector2 pos, float& outScale, Vector2& outOffset, Vector2& outPos);
    };

    //WARN: An Observer can only be part of one subject's observer list
    //If necessary use a pool of list nodes.
    //So to semplify the distruction of Observer and the Subject.
    //the last object that own the pointer to the subject notify the destruction 
    //of Subject to the Observer
    template <typename EntityT, typename EventT>class Subject;

    template <typename EntityT, typename EventT>
    class Observer
    {
        friend class Subject<EntityT, EventT>;
        public:
            Observer():m_pNext(nullptr), m_pSubject(nullptr){}
            virtual ~Observer(){}

            virtual void onNotify(const EntityT& entity, const EventT& event) = 0;
            void destroyObsList()
            {
                Observer<EntityT, EventT>* pCurrent = m_pNext;
                while(pCurrent != nullptr)
                {
                    Observer<EntityT, EventT>* next = pCurrent->m_pNext;
                    pCurrent = nullptr;
                    pCurrent = next;
                }
            }
        private:
            Observer<EntityT, EventT>* m_pNext;
            Subject<EntityT, EventT>* m_pSubject;
    };

    template <typename EntityT, typename EventT>
    class Subject
    {
        public:
            Subject(): m_pObsHead(nullptr){}
            ~Subject()
            {
                m_pObsHead->destroyObsList();
                m_pObsHead = nullptr;
            }
            void addObserver(Observer<EntityT, EventT>* observer)
            {
                observer->m_pNext = m_pObsHead;
                m_pObsHead = observer;
            }

            void removeObserver(Observer<EntityT, EventT>* observer)
            {
                if(m_pObsHead == observer)
                {
                    m_pObsHead = observer->m_pNext;
                    observer->m_pNext = nullptr;
                    return;
                }

                Observer<EntityT, EventT>* current = m_pObsHead;
                while(current != nullptr)
                {
                    if(current->m_pNext == observer)
                    {
                        current->m_pNext = observer->m_pNext;
                        observer->m_pNext = nullptr;
                        return;
                    }

                    current = current->m_pNext;
                }
            }

        protected:
            void notify(const EntityT& entity, EventT event)
            {
                Observer<EntityT, EventT>* pObs = m_pObsHead;

                while(pObs != nullptr)
                {
                    pObs->onNotify(entity, event);
                    pObs = pObs->m_pNext;
                }
            }

        private:
            Observer<EntityT, EventT>* m_pObsHead;
    };

}