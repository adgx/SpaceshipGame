#pragma once

#include "utils/utils.h"

namespace SpaceEngine
{
    class Transform
    {
        public:
            Transform() = default;
            ~Transform() = default;

            Transform(const Transform& other)
            {
                localPos = other.localPos;
                localRot = other.localRot;
                localScale = other.localScale;
                parent = other.parent;
                //no copy the children
                dirty = other.dirty;
            };

            Transform& operator=(const Transform& other) 
            {
                localPos = other.localPos;
                localRot = other.localRot;
                localScale = other.localScale;
                parent = other.parent;
                //no copy the children
                dirty = other.dirty;
            };

            Transform(Transform&& other) noexcept { moveFrom(std::move(other)); }
            Transform& operator=(Transform&& other) noexcept { moveFrom(std::move(other)); return *this; }

            //matrix computation
            Matrix4 getWorldMatrix() const
            {
                if(dirty)
                {
                    Matrix4 T = glm::translate(Math::identityMatrix4(), localPos);
                    Matrix4 R = glm::mat4_cast(localRot);
                    Matrix4 S = glm::scale(Math::identityMatrix4(), localScale);
                    Matrix4 localMatrix = T * R * S;
                    
                    if(parent)
                        cachedWorldMatrix = parent->getWorldMatrix() * localMatrix;
                    else 
                        cachedWorldMatrix = localMatrix;
                    
                    dirty = false;
                }
                return cachedWorldMatrix;
            }

            inline void markDirty()
            {
                if(!dirty)
                {
                    dirty=true; for(auto* c: children) c->markDirty();
                }
            }

            //Local translations
            inline void translateLocal(const Vector3& delta)
            {
                localPos += localRot * delta;
                markDirty();
            }

            inline void translateGlobal(const Vector3& delta)
            {
                if(parent)
                {
                    Matrix4 parentInv = Math::inverse(parent->getWorldMatrix());
                    localPos += Vector3(parentInv * Vector4(delta, 0.f));
                }
                else
                {
                    localPos +=delta;
                }
                markDirty();
            }

            Vector3 getLocalPosition() const { return localPos; }
            void setLocalPosition(const Vector3& p) { localPos = p; markDirty(); }

            Quat getLocalRotation() const { return localRot; }
            void setLocalRotation(const Quat& q) { localRot = q; markDirty(); }

            Vector3 getLocalScale() const { return localScale; }
            void setLocalScale(const Vector3& s) { localScale = s; markDirty(); }

            inline void rotateLocal(const float& degree, const Vector3& axis)
            {
                Quat delta = glm::angleAxis(Math::radians(degree), axis);

                localRot = localRot * delta;
                markDirty();
            }

            inline void rotateGlobal(const float& degree, const Vector3& axis)
            {
                Quat delta = glm::angleAxis(Math::radians(degree), axis);
                localRot = delta * localRot;
                markDirty();
            }

            inline void scale(Vector3 s)
            {
                localScale *= s;
                markDirty();
            }

            inline void scale(float x, float y, float z)
            {
                Vector3 s{x, y, z};
                scale(s);
            }

            Vector3 getWorldPosition()
            {
                Matrix4 world = getWorldMatrix();
                return Vector3(world[3]);
            }

            void setWorldPosition(const Vector3& worldPos)
            {
                if(parent)
                {
                    Matrix4 parentInv = Math::inverse(parent->getWorldMatrix());
                    localPos = Vector3(parentInv * Vector4(worldPos, 1.f));
                }
                else
                {
                    localPos = worldPos;
                }

                markDirty();
            }

            Quat getWorldRotationQuat() 
            {
                Matrix4 world = getWorldMatrix();
                Matrix3 m3 = Matrix3(world);
                Vector3 c0 = Vector3(m3[0]);
                Vector3 c1 = Vector3(m3[1]);
                Vector3 c2 = Vector3(m3[2]);
                if (glm::length(c0) != 0.0f) c0 = glm::normalize(c0);
                if (glm::length(c1) != 0.0f) c1 = glm::normalize(c1);
                if (glm::length(c2) != 0.0f) c2 = glm::normalize(c2);
                Matrix3 rotMat;
                rotMat[0] = c0; rotMat[1] = c1; rotMat[2] = c2;
                return glm::quat_cast(rotMat);
            }

            Vector3 getWorldRotationEuler(){return glm::eulerAngles(getWorldRotationQuat());}
            Vector3 getWorldRotationEulerDegree(){return glm::degrees(getWorldRotationEuler());}

            Vector3 forwardLocal() const { return localRot * Vector3(0.0f, 0.0f, -1.0f); }
            Vector3 rightLocal() const { return localRot * Vector3(1.0f, 0.0f, 0.0f); }
            Vector3 upLocal() const { return localRot * Vector3(0.0f, 1.0f, 0.0f); }

            Vector3 forwardWorld() const { return glm::normalize(Vector3(0.0f, 0.0f, 1.0f)); }
            Vector3 rightWorld() const { return glm::normalize(Vector3(1.0f, 0.0f, 0.0f)); }
            Vector3 upWorld() const { return glm::normalize(Vector3(0.0f, 1.0f, 0.0f));}

            void lookAt(const Vector3& target, const Vector3& up = Vector3(0.0f, 1.0f, 0.0f)) 
            {
                Vector3 eye = getWorldPosition();
                Matrix4 view = glm::lookAt(eye, target, up);
                Matrix4 world = glm::inverse(view);
                Vector3 newWorldPos = Vector3(world[3]);
                Matrix3 rotMat = Matrix3(world);
                Vector3 c0 = Vector3(rotMat[0]);
                Vector3 c1 = Vector3(rotMat[1]);
                Vector3 c2 = Vector3(rotMat[2]);
                if (glm::length(c0) != 0.0f) c0 = glm::normalize(c0);
                if (glm::length(c1) != 0.0f) c1 = glm::normalize(c1);
                if (glm::length(c2) != 0.0f) c2 = glm::normalize(c2);
                Matrix3 newRotMat; newRotMat[0]=c0; newRotMat[1]=c1; newRotMat[2]=c2;
                Quat newWorldRot = glm::quat_cast(newRotMat);
                if (parent) 
                {
                    Matrix4 parentInv = glm::inverse(parent->getWorldMatrix());
                    Matrix4 localWorld = parentInv * world;
                    localPos = Vector3(localWorld[3]);
                    Matrix3 localRotMat = Matrix3(localWorld);
                    Vector3 lc0=glm::normalize(Vector3(localRotMat[0]));
                    Vector3 lc1=glm::normalize(Vector3(localRotMat[1]));
                    Vector3 lc2=glm::normalize(Vector3(localRotMat[2]));
                    Matrix3 lrm; lrm[0]=lc0;lrm[1]=lc1;lrm[2]=lc2;
                    localRot = glm::quat_cast(lrm);
                } 
                else 
                {
                localPos = newWorldPos;
                localRot = newWorldRot;
                }
                
                markDirty();
            }
            
            //hierarchy management
            Transform* getParent() const { return parent; }

            void setParent(Transform* newParent) 
            {
                if (parent == newParent) return;
                // remove from old parent's children list
                if (parent) 
                {
                    auto &siblings = parent->children;
                    siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
                }

                parent = newParent;

                if (parent) parent->children.push_back(this);

                markDirty();
            }

            const std::vector<Transform*>& getChildren() const { return children; }

        public:
            Vector3 localPos{0.f};
            Quat localRot{1.f, 0.f, 0.f, 0.f};
            Vector3 localScale{1.f};
            //hierarchy
        private:
            void moveFrom(Transform&& other) 
            {
                localPos=other.localPos; localRot=other.localRot; localScale=other.localScale;
                parent=other.parent; children=std::move(other.children);
                dirty=other.dirty; cachedWorldMatrix=other.cachedWorldMatrix;
                if(parent)
                { 
                    auto &siblings = parent->children; 
                    auto it=std::find(siblings.begin(), siblings.end(), &other); 
                    if(it!=siblings.end()) 
                        *it=this;
                }

                for(auto* c: children) c->parent=this;
                
                other.parent=nullptr; other.children.clear(); other.dirty=true;
            }
        private:
            Transform* parent = nullptr;
            std::vector<Transform*> children;
            mutable bool dirty = true;
            mutable Matrix4 cachedWorldMatrix = Math::identityMatrix4();
    };
}