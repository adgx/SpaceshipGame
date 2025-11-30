#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glad/gl.h>
#include <memory>
#include "utils/utils.h"
#include "material.h"

#define INVALID_MATERIAL 0xFFFFFFFF


namespace SpaceEngine
{
    class Mesh
    {
        public:
            Mesh() = default;
            ~Mesh();
            //static bool loadMesh(const std::string& fileName);
            //create box 
            //create square
            //create a sphere
            //void render();
            //void render(unsigned int drawIndex, unsigned int primID);
            //void render(unsigned int numInstannes, const Matrix4* WVPMats, const Matrix4* worldMats);
        private:
            void clear();

            void populateBuffers();
            //void setupRenderMaterialsPBR();
            const BaseMaterial& getMaterial();
            //void getLeadingVertex(uint32_t drawIndex, uint32_t primID, Vector3& vertex);


            struct Vertex 
            {
                Vector3 position;
                Vector2 texCoords;
                Vector3 normal;
            };

            enum BUFFER_TYPE
            {
                INDEX_BUFFER = 0,
                VERTEX_BUFFER = 1,
                WVP_MAT_BUFFER = 2,
                WORLD_MAT_BUFFER = 3,
                NUM_BUFFER = 4,
            };

            struct MeshEntry 
            {
                MeshEntry()
                {
                    numIndices = 0;
                    baseVertex = 0;
                    baseIndex = 0;
                    materialIndex = INVALID_MATERIAL;
                }
            
                uint32_t numIndices;
                uint32_t baseVertex;
                uint32_t baseIndex;
                uint32_t materialIndex;
            };

            GLuint VAO = 0;
            GLuint buffers[NUM_BUFFER] = {0};
            //transform
            //Matrix4 globalInverseTransform;
            std::vector<std::unique_ptr<BaseMaterial>> materials;
            std::vector<MeshEntry> meshes;
            std::vector<uint32_t> indices;
            std::vector<Vertex> vertices;
            bool isPBR = false;
            
        friend class MeshManager;
    };

    class MeshManager
    {
        public:
        MeshManager() = default;
        ~MeshManager() = default;
        void Initialize();
        void Shutdown();
        static Mesh* loadMesh(const std::string& fileName);
        private:
            static bool initFromScene(const aiScene* pScene, const std::string& fileName);
            static void countVerticesAndIndices(const aiScene* pScene, unsigned int& numVertices, unsigned int& numIndices);
            static void reserveSpace(unsigned int numVertices, unsigned int numIndices);
            static void initAllMeshes(const aiScene* pScene);
            static void initSingleMesh(uint32_t meshIndex, const aiMesh* paiMesh);
            static bool initMaterials(const aiScene* pScene, const std::string& fileName);
            static void loadTextures(const std::string& dir, const aiMaterial* pMaterial, const aiScene* pScene, int index);
            static void loadDiffuseTexture(const std::string& dir, const aiMaterial* pMaterial, const aiScene* pScene, int materialIndex);
            static void loadRoughnessTexture(const std::string& dir, const aiMaterial* pMaterial, const aiScene* pScene, int materialIndex);
            static void loadSpecularTexture(const std::string& dir, const aiMaterial* pMaterial, const aiScene* pScene, int materialIndex);
            static void loadAlbedoTexture(const std::string& dir, const aiMaterial* pMaterial, const aiScene* pScene, int materialIndex);
            static void loadMetalnessTexture(const std::string& dir, const aiMaterial* pMaterial, const aiScene* pScene, int materialIndex);
            static void loadColors(const aiMaterial* pMaterial, int index);

            static Mesh* pTMPMesh;
    };
    

};