#pragma once
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glad/gl.h>
#include <memory>
#include <unordered_map>

#include "utils/utils.h"
#include "material.h"

#define INVALID_MATERIAL 0xFFFFFFFF

namespace SpaceEngine
{
    class Mesh
    {
    public:
        Mesh() = default;
        ~Mesh() = default;
        void bindVAO();
        int getNumSubMesh();
        int bindMaterialToSubMeshIndex(int index, BaseMaterial *pMat);
        BaseMaterial *getMaterialBySubMeshIndex(int index);
        void drawSubMesh(unsigned int idSubMesh);

    private:
        void clear();
        void populateBuffers();

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
        std::vector<BaseMaterial *> materials;
        std::vector<MeshEntry> subMeshes;
        std::vector<uint32_t> indices;
        std::vector<Vertex> vertices;
        std::string name;
        bool isPBR = false;

    public:
        Vector3 maxPos;
        Vector3 minPos;

        friend class MeshManager;
    };

    class UIMesh
    {
    public:
        UIMesh();
        void draw();
        void bindVAO();

    protected:
        void populateBuffers();
        GLuint VAO = 0;
        GLuint buffers[2] = {0};
    };

    class TextMesh
    {
    public:
        TextMesh();
        void draw();
        void bindVAO();
        void subData(const std::array<std::array<float, 4>, 6>& vertices);


    private:
        void populateBuffers();
        GLuint VAO = 0;
        GLuint buffer = 0;
    };

    class TextMeshRenderer
    {
        public:
        TextMeshRenderer();
        
        int bindMaterial(TextMaterial *pMat);
        TextMaterial *getMaterial();
        TextMesh *getTextMesh();

    private:
        TextMesh *pMesh;
        TextMaterial *pMat;    
    };

    class MeshManager
    {
    public:
        MeshManager() = default;
        ~MeshManager() = default;
        void Initialize();
        static Mesh *loadMesh(const std::string &fileName);
        static UIMesh *getUIMesh();
        static TextMesh *getTextMesh();
        static Mesh *findMesh(const std::string &name);

        // create box
        // create square
        // create a sphere
        void Shutdown();

    private:
        static bool initFromScene(const aiScene *pScene, const std::string &fileName);
        static void countVerticesAndIndices(const aiScene *pScene, unsigned int &numVertices, unsigned int &numIndices);
        static void reserveSpace(unsigned int numVertices, unsigned int numIndices);
        static void initAllMeshes(const aiScene *pScene);
        static void initSingleMesh(uint32_t meshIndex, const aiMesh *paiMesh);
        static bool initMaterials(const aiScene *pScene, const std::string &fileName);
        static void loadTextures(const std::string &dir, const aiMaterial *pMaterial, const aiScene *pScene, int index);
        static void loadDiffuseTexture(const std::string &dir, const aiMaterial *pMaterial, const aiScene *pScene, int materialIndex);
        static void loadRoughnessTexture(const std::string &dir, const aiMaterial *pMaterial, const aiScene *pScene, int materialIndex);
        static void loadSpecularTexture(const std::string &dir, const aiMaterial *pMaterial, const aiScene *pScene, int materialIndex);
        static void loadAlbedoTexture(const std::string &dir, const aiMaterial *pMaterial, const aiScene *pScene, int materialIndex);
        static void loadMetalnessTexture(const std::string &dir, const aiMaterial *pMaterial, const aiScene *pScene, int materialIndex);
        static void loadColors(const aiMaterial *pMaterial, int index);
        static void loadNormalsTexture(const std::string &dir, const aiMaterial *pMaterial, const aiScene *pScene, int materialIndex);
        static void loadAmbientOcclusionTexture(const std::string &dir, const aiMaterial *pMaterial, const aiScene *pScene, int materialIndex);

        static Mesh *pTMPMesh;
        static std::unordered_map<std::string, Mesh *> meshMap;
        static UIMesh *pUIMesh;
        static TextMesh *pTextMesh;

    };

    class UIMeshRenderer
    {
    public:
        UIMeshRenderer();
        void bindVAO();
        int bindMaterial(UIMaterial *pMat);
        UIMaterial *getMaterial();
        UIMesh *getUIMesh();

    private:
        UIMesh *pMesh;
        UIMaterial *pMat;
    };
};