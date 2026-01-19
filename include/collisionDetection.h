#pragma once
#include "utils/utils.h"
#include "gameObject.h"
#include "log.h"
#include <cstdint>
#include <algorithm>
#include <list>

//#define HGRID_MAX_LEVELS 2
//#define NUM_BUCKETS 1024
//#define MIN_CELL_SIZE 2


namespace SpaceEngine
{
    struct AABB
    {
        Vector3 c;
        float r[3];


        float maxSide()
        {
            return std::max(std::max(r[0], r[1]), r[2]);
        }

        static int test(const AABB& a, const AABB& b)
        {
            if(abs(a.c[0]-b.c[0]) > (a.r[0] + b.r[0])) return 0;
            if(abs(a.c[1]-b.c[1]) > (a.r[1] + b.r[1])) return 0;
            if(abs(a.c[2]-b.c[2]) > (a.r[2] + b.r[2])) return 0;

            return 1;
        }
    };

    class Collider
    {
        public:
            Collider* pNext = nullptr;
            Collider* pPrev = nullptr;
            AABB bbox;
            Vector3 pos;
            int bucket;
            int level;
            GameObject* gameObj = nullptr;

            Collider(GameObject* gameObj):gameObj(gameObj)
            {
                //x
                bbox.c.x = gameObj->getComponent<Mesh>()->maxPos.x * gameObj->getComponent<Transform>()->localScale.x;
                bbox.r[0] = (bbox.c.x - gameObj->getComponent<Mesh>()->minPos.x * gameObj->getComponent<Transform>()->localScale.x) / 2.f;
                bbox.c.x -= bbox.r[0];
                //y
                bbox.c.y = gameObj->getComponent<Mesh>()->maxPos.y * gameObj->getComponent<Transform>()->localScale.y;
                bbox.r[1] = (bbox.c.y - gameObj->getComponent<Mesh>()->minPos.y * gameObj->getComponent<Transform>()->localScale.y) / 2.f;
                bbox.c.y -= bbox.r[1];
                //z
                bbox.c.z = gameObj->getComponent<Mesh>()->maxPos.z * gameObj->getComponent<Transform>()->localScale.z;
                bbox.r[2] = (bbox.c.z - gameObj->getComponent<Mesh>()->minPos.z * gameObj->getComponent<Transform>()->localScale.z) / 2.f;
                bbox.c.z -= bbox.r[2];

                pos = bbox.c;

                SPACE_ENGINE_DEBUG("Collider: center: {}, {}, {} radious: {}, {}, {}", 
                    bbox.c.x, bbox.c.y, bbox.c.z,
                    bbox.r[0], bbox.r[1], bbox.r[2])
            }


            static int testCollidersLocalSpace(const Collider* a, const Collider* b)
            {
                // World matrices
                const Transform* tA = a->gameObj->getComponent<Transform>();
                const Transform* tB = b->gameObj->getComponent<Transform>();
                        
                Matrix4 worldA = tA->getWorldMatrix();
                Matrix4 worldB = tB->getWorldMatrix();
                Matrix4 invWorldA = Math::inverse(worldA);
                        
                // AABB A stays in its own local space
                AABB bboxA = a->bbox;
                bboxA.c = Vector3{0.f}; // centered in A local space
                        
                // Transform B's center into A's local space
                Vector4 bCenterWorld = worldB * Vector4(b->bbox.c, 1.f);
                Vector4 bCenterInALocal = invWorldA * bCenterWorld;
                        
                AABB bboxB = b->bbox;
                bboxB.c = Vector3{
                    bCenterInALocal.x,
                    bCenterInALocal.y,
                    bCenterInALocal.z
                };
            
                return AABB::test(bboxA, bboxB);
            }

            void fixedUpdate()
            {
                pos = gameObj->getComponent<Transform>()->getWorldPosition() + bbox.c;
            }

        
    };

    struct Cell 
    {
        int x, y, z, w;
        Cell(int x, int y, int z, int w):x(x), y(y), z(z), w(w)
        {}
    };

    

    class PhysicsManager
    {
        private:
            static const int HGRID_MAX_LEVELS = 6;
            static const int NUM_BUCKETS = 1024;
            static const int MIN_CELL_SIZE = 2;
            //this allows to create a margin for the cell where is palced the object
            static constexpr float SPHERE_TO_CELL_RATIO = 1.f/4.f;//is considered the diameter
            static constexpr float CELL_TO_CELL_RATIO = 2.f;

            struct CollisionPair
            {
                Collider* a;
                Collider* b;
            
                bool operator==(const CollisionPair& o) const
                {
                    return (a == o.a && b == o.b) || (a == o.b && b == o.a);
                }
            };

            struct CollisionPairHash
            {
                size_t operator()(const CollisionPair& p) const
                {
                    return ((size_t)p.a >> 4) ^ ((size_t)p.b >> 4);
                }
            };

            

            struct HGrid
            {
                uint32_t occupiedLevelsMask = 0;
                int collidersAtLevel[HGRID_MAX_LEVELS] = {0};
                Collider* colliderBucket[NUM_BUCKETS] = {nullptr};
                int timeStamp[NUM_BUCKETS] = {0};
                int tick = 0;

                int ComputeHashBucketIndex(Cell cellPos)
                {
                    const int h1 = 0x8da6b343;
                    const int h2 = 0xd8163841;
                    const int h3 = 0xcb1ab31f;
                    const int h4 = 0x165667b1;

                    int n = h1 * cellPos.x + h2 * cellPos.y + h3 * cellPos.z + h4 * cellPos.w;

                    n = n % NUM_BUCKETS;

                    if(n < 0) n += NUM_BUCKETS;

                    return n;

                }

                void AddColliderToHGrid(Collider* col)
                {
                    int level;
                    float size = MIN_CELL_SIZE, diameter = col->bbox.maxSide();

                    //find the lowest level where objcet fully fits inside cell
                    for(level = 0; size * SPHERE_TO_CELL_RATIO < diameter; level++)
                        size *= CELL_TO_CELL_RATIO;

                    assert(level < HGRID_MAX_LEVELS);

                    Cell cellPos(static_cast<int>((col->bbox.c.x + col->pos.x)/ size), 
                        static_cast<int>((col->bbox.c.y + col->pos.y) / size), 
                        static_cast<int>((col->bbox.c.z + col->pos.z) / size), 
                        level);
                    int bucket = ComputeHashBucketIndex(cellPos);
                    col->bucket = bucket;
                    col->level = level;
                    col->pPrev = nullptr;
                    if(colliderBucket[bucket]) colliderBucket[bucket]->pPrev = col;
                    col->pNext = colliderBucket[bucket];
                    colliderBucket[bucket] = col;

                    collidersAtLevel[level]++;
                    occupiedLevelsMask |= (1 << level);
                }

                void RemoveObjectFromGrid(Collider* col)
                {
                    if(--collidersAtLevel[col->level] == 0)
                        occupiedLevelsMask &= ~(1 << col->level);

                    int bucket = col->bucket;

                    if (col->pPrev)
                        col->pPrev->pNext = col->pNext;
                    else
                        colliderBucket[bucket] = col->pNext;  
                                
                    if (col->pNext)
                        col->pNext->pPrev = col->pPrev;
                }

                void CheckObjAgainstGrid(Collider* col, std::unordered_set<CollisionPair, CollisionPairHash>& currCollisions)
                {
                    float size = MIN_CELL_SIZE;
                    int startLevel = 0;
                    uint32_t occupiedLevelsMask = this->occupiedLevelsMask;
                    Vector3 pos = col->bbox.c + col->pos;

                    //tick++;

                    for(int level = startLevel; level < HGRID_MAX_LEVELS; 
                        size *= CELL_TO_CELL_RATIO, occupiedLevelsMask >>= 1, level++)
                    {
                        //no colliders in the HGrid
                        if(occupiedLevelsMask == 0)
                            break;
                        //no colliders at this level
                        if((occupiedLevelsMask & 1) == 0)
                            continue;

                        float delta = col->bbox.maxSide() + size * SPHERE_TO_CELL_RATIO + MIN_CELL_SIZE/2.f;
                        float ooSize = 1.f / size;

                        int x1 = static_cast<int>(floorf((pos.x - delta) * ooSize));
                        int y1 = static_cast<int>(floorf((pos.y - delta) * ooSize));
                        int z1 = static_cast<int>(floorf((pos.z - delta) * ooSize));
                        int x2 = static_cast<int>(ceilf((pos.x + delta) * ooSize));
                        int y2 = static_cast<int>(ceilf((pos.y + delta) * ooSize));
                        int z2 = static_cast<int>(ceilf((pos.z + delta) * ooSize));

                        for(int x = x1; x <= x2; x++)
                            for(int y = y1; y <= y2; y++)
                                for(int z = z1; z <= z2; z++)
                                {
                                    Cell cellPos(x, y, z, level);
                                    int bucket = ComputeHashBucketIndex(cellPos);

                                    if(timeStamp[bucket] == tick) 
                                        continue;

                                    timeStamp[bucket] = tick;

                                    Collider *p = colliderBucket[bucket];

                                    while(p)
                                    {
                                        if(p != col && !p->gameObj->pendingDestroy)
                                        {
                                            if(Collider::testCollidersLocalSpace(col, p))
                                            {
                                                CollisionPair pair{col, p};
                                                currCollisions.insert(pair);
                                                //col->gameObj->onCollisionEnter(p);
                                                //p->gameObj->onCollisionEnter(col);
                                            }
                                        }
                                        p = p->pNext;
                                    }
                                }
                    }

                }
            };


        public:
            PhysicsManager() = default;
            ~PhysicsManager() = default;

            void Step(float fixed_dt);
            void Initialization();
            void Initialization(const std::list<Collider*>& lCols);
            void AddColliders(const std::list<Collider*>& lCols);
            void AddCollider(Collider* col);
            void RemoveColliders(const std::list<Collider*>& lCols);
            void RemoveCollider(Collider* col);
            void Shutdown();
        
        private:
            void HandleCollisionEvents();
            std::unordered_set<CollisionPair, CollisionPairHash> prevCollisions;
            std::unordered_set<CollisionPair, CollisionPairHash> currCollisions;
            std::list<Collider*> lColliders;
            HGrid grid;
    };
}
