#include "mesh.h"
#include "log.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "utils/utils.h"
#include "texture.h"

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenSmoothNormals | \
     aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices)

#define POSITION_LOCATION  0
#define TEX_COORD_LOCATION 1
#define NORMAL_LOCATION    2

#define COLOR_TEXTURE_UNIT              GL_TEXTURE0
#define COLOR_TEXTURE_UNIT_INDEX        0
#define SHADOW_TEXTURE_UNIT             GL_TEXTURE1
#define SHADOW_TEXTURE_UNIT_INDEX       1
#define NORMAL_TEXTURE_UNIT             GL_TEXTURE2
#define NORMAL_TEXTURE_UNIT_INDEX       2
#define RANDOM_TEXTURE_UNIT             GL_TEXTURE3
#define RANDOM_TEXTURE_UNIT_INDEX       3
#define DISPLACEMENT_TEXTURE_UNIT       GL_TEXTURE4
#define DISPLACEMENT_TEXTURE_UNIT_INDEX 4
#define ALBEDO_TEXTURE_UNIT             GL_TEXTURE5
#define ALBEDO_TEXTURE_UNIT_INDEX       5          
#define ROUGHNESS_TEXTURE_UNIT          GL_TEXTURE6
#define ROUGHNESS_TEXTURE_UNIT_INDEX    6
#define MOTION_TEXTURE_UNIT             GL_TEXTURE7
#define MOTION_TEXTURE_UNIT_INDEX       7
#define SPECULAR_EXPONENT_UNIT             GL_TEXTURE8
#define SPECULAR_EXPONENT_UNIT_INDEX       8
#define CASCACDE_SHADOW_TEXTURE_UNIT0               SHADOW_TEXTURE_UNIT
#define CASCACDE_SHADOW_TEXTURE_UNIT0_INDEX         SHADOW_TEXTURE_UNIT_INDEX
#define CASCACDE_SHADOW_TEXTURE_UNIT1               GL_TEXTURE9
#define CASCACDE_SHADOW_TEXTURE_UNIT1_INDEX         9
#define CASCACDE_SHADOW_TEXTURE_UNIT2               GL_TEXTURE10
#define CASCACDE_SHADOW_TEXTURE_UNIT2_INDEX         10
#define SHADOW_CUBE_MAP_TEXTURE_UNIT                GL_TEXTURE11
#define SHADOW_CUBE_MAP_TEXTURE_UNIT_INDEX          11
#define SHADOW_MAP_RANDOM_OFFSET_TEXTURE_UNIT       GL_TEXTURE12
#define SHADOW_MAP_RANDOM_OFFSET_TEXTURE_UNIT_INDEX 12
#define METALLIC_TEXTURE_UNIT                       GL_TEXTURE14
#define METALLIC_TEXTURE_UNIT_INDEX                 14
#define HEIGHT_TEXTURE_UNIT                         GL_TEXTURE15
#define HEIGHT_TEXTURE_UNIT_INDEX                   15

namespace SpaceEngine
{
    

    void Mesh::clear()
    {
        if(buffers[0] != 0)
        {
            glDeleteBuffers(ARRAY_SIZE_IN_ELEMENTS(buffers), buffers);
        }

        if(VAO != 0)
        {
            glDeleteVertexArrays(1, &VAO);
            VAO = 0;
        }
    }

    Mesh* MeshManager::pTMPMesh = nullptr;
    Mesh* MeshManager::loadMesh(const std::string& fileName)
    {
        //clear();
        Mesh* pMesh = new Mesh();
        pTMPMesh = pMesh;
        //VAO
        glGenVertexArrays(1, &pTMPMesh->VAO);
        glBindVertexArray(pTMPMesh->VAO);
        glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(pTMPMesh->buffers), pTMPMesh->buffers);

        bool ret = false;
        
        Assimp::Importer importer;
        const aiScene* pScene = importer.ReadFile(fileName.c_str(), ASSIMP_LOAD_FLAGS);

        if(pScene)
        {
            /*capiamo*/
            //globalInverseTransform = aiMat4_2_Mat4(pScene->mRootNode->mTransformation);
            //globalInverseTransform = inverse(globalInverseTransform);
            if(!initFromScene(pScene, fileName))
            {
                SPACE_ENGINE_FATAL("Impossible to init the scene:{}", fileName);
                exit(-1);
            }
        }
        pTMPMesh = nullptr;
        return pMesh;
    }

    bool MeshManager::initFromScene(const aiScene* pScene, const std::string& fileName)
    {
        //number of the meshes in the scene
        pTMPMesh->meshes.resize(pScene->mNumMeshes);
        pTMPMesh->materials.resize(pScene->mNumMaterials);

        unsigned int numVertices = 0;
        unsigned int numIndices = 0;
        //calculate the number of vertices and indices in the scene
        countVerticesAndIndices(pScene, numVertices, numIndices);
        //allocate space in the host
        reserveSpace(numVertices, numIndices);

        initAllMeshes(pScene);

        if (!initMaterials(pScene, fileName)) {
            return false;
        }

        //populateBuffers();

        return glGetError() == GL_NO_ERROR;
    }
    
    void MeshManager::countVerticesAndIndices(const aiScene* pScene, unsigned int& numVertices, unsigned int& numIndices)
    {
        for (unsigned int i = 0 ; i < pTMPMesh->meshes.size() ; i++) 
        {
            pTMPMesh->meshes[i].materialIndex = pScene->mMeshes[i]->mMaterialIndex;
            pTMPMesh->meshes[i].numIndices = pScene->mMeshes[i]->mNumFaces * 3;
            pTMPMesh->meshes[i].baseVertex = numVertices;
            pTMPMesh->meshes[i].baseIndex = numIndices;

            numVertices += pScene->mMeshes[i]->mNumVertices;
            numIndices  += pTMPMesh->meshes[i].numIndices;
        }
    }

    void MeshManager::reserveSpace(unsigned int numVertices, unsigned int numIndices)
    {
        pTMPMesh->vertices.reserve(numVertices);
        pTMPMesh->indices.reserve(numIndices);
    }


    void MeshManager::initAllMeshes(const aiScene* pScene)
    {
        for (unsigned int i = 0 ; i < pTMPMesh->meshes.size() ; i++) 
        {
            const aiMesh* paiMesh = pScene->mMeshes[i];
            initSingleMesh(i, paiMesh);
        }
    }


    void MeshManager::initSingleMesh(uint32_t meshIndex, const aiMesh* paiMesh)
    {
        const aiVector3D zero3D(0.0f, 0.0f, 0.0f);

        //populate the vertex attribute vectors
        Mesh::Vertex v;

        for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
            const aiVector3D& pPos = paiMesh->mVertices[i];

            v.position = Vector3(pPos.x, pPos.y, pPos.z);

            if (paiMesh->mNormals) {
                const aiVector3D& pNormal = paiMesh->mNormals[i];
                v.normal = Vector3(pNormal.x, pNormal.y, pNormal.z);
            } else {
                aiVector3D normal(0.0f, 1.0f, 0.0f);
                v.normal = Vector3(normal.x, normal.y, normal.z);
            }

            const aiVector3D& pTexCoord = paiMesh->HasTextureCoords(0) ? paiMesh->mTextureCoords[0][i] : zero3D;
            v.texCoords = Vector2(pTexCoord.x, pTexCoord.y);

            pTMPMesh->vertices.push_back(v);
        }

        //populate the index buffer
        for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
            const aiFace& face = paiMesh->mFaces[i];
            pTMPMesh->indices.push_back(face.mIndices[0]);
            pTMPMesh->indices.push_back(face.mIndices[1]);
            pTMPMesh->indices.push_back(face.mIndices[2]);
        }
    }

    bool MeshManager::initMaterials(const aiScene* pScene, const std::string& fileName)
    {
        std::string dir = Utils::getDirFromFilename(fileName);

        SPACE_ENGINE_DEBUG("Num materials: {}", pScene->mNumMaterials);

        // Initialize the materials
        for (unsigned int i = 0 ; i < pScene->mNumMaterials ; i++) {
            const aiMaterial* pMaterial = pScene->mMaterials[i];
            std::unique_ptr<PBRMaterial> pbrMat = std::make_unique<PBRMaterial>();
            pbrMat.get()->pShader = nullptr;//TODO: make the biding 
            pTMPMesh->materials.push_back(std::move(pbrMat));
            loadTextures(dir, pMaterial, pScene, i);
            loadColors(pMaterial, i);
        }

        return true;
    }


    void MeshManager::loadTextures(const std::string& dir, const aiMaterial* pMaterial, const aiScene* pScene, int index)
    {
        loadDiffuseTexture(dir, pMaterial, pScene, index);
        loadSpecularTexture(dir, pMaterial, pScene, index);

        // PBR
        loadAlbedoTexture(dir, pMaterial, pScene, index);
        loadMetalnessTexture(dir, pMaterial, pScene, index);
        loadRoughnessTexture(dir, pMaterial, pScene, index);
    }


    void MeshManager::loadDiffuseTexture(const std::string& dir, const aiMaterial* pMaterial, const aiScene* pScene, int materialIndex)
    {
        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString path;

            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) 
            {
                const aiTexture* paiTexture = pScene->GetEmbeddedTexture(path.C_Str());

                if (paiTexture) {
                    SPACE_ENGINE_DEBUG("Embeddeded diffuse texture type '{}'\n", paiTexture->achFormatHint);
                    int buffer_size = paiTexture->mWidth;
                    bool IsSRGB = true;
                    pTMPMesh->materials[materialIndex].get()->addTexture("diffuse_tex", TextureManager::load(Utils::getFileNameFormPath(path.C_Str()), buffer_size, paiTexture->pcData, IsSRGB));
                } 
                else 
                {
                    std::string fullPath = Utils::getFullPath(dir, path);
                    bool IsSRGB = true;
                    pTMPMesh->materials[materialIndex].get()->addTexture("diffuse_tex", TextureManager::load(Utils::getFileNameFormPath(path.C_Str()), IsSRGB));
                    SPACE_ENGINE_INFO("Loaded diffuse texture '{}' at index {}", fullPath.c_str(), materialIndex);
                }
            }
                        
        }
    }

/*
    void MeshManager::loadDiffuseTextureEmbedded(const aiTexture* paiTexture, int materialIndex)
    {
        SPACE_ENGINE_DEBUG("Embeddeded diffuse texture type '{}'\n", paiTexture->achFormatHint);
        materials[materialIndex].pTextures[TEXTURE_TYPE::BASE] = new Texture(GL_TEXTURE_2D);
        int buffer_size = paiTexture->mWidth;
        bool IsSRGB = true;
        materials[materialIndex].pTextures[TEXTURE_TYPE::BASE]->load(buffer_size, paiTexture->pcData, IsSRGB);
    }


    void MeshManager::loadDiffuseTextureFromFile(const std::string& dir, const aiString& path, int materialIndex)
    {
        std::string fullPath = getFullPath(dir, path);

        materials[materialIndex].pTextures[TEXTURE_TYPE::BASE] = new Texture(GL_TEXTURE_2D, fullPath.c_str());

        bool IsSRGB = true;

        if (!materials[materialIndex].pTextures[TEXTURE_TYPE::BASE]->load(IsSRGB)) {
            SPACE_ENGINE_ERROR("Error loading diffuse texture '{}'", fullPath.c_str());
            exit(0);
        }
        else {
            SPACE_ENGINE_INFO("Loaded diffuse texture '{}' at index {}", fullPath.c_str(), materialIndex);
        }
    }
*/

    void MeshManager::loadSpecularTexture(const std::string& dir, const aiMaterial* pMaterial, const aiScene* pScene, int materialIndex)
    {
        if (pMaterial->GetTextureCount(aiTextureType_SHININESS) > 0) {
            aiString path;

            if (pMaterial->GetTexture(aiTextureType_SHININESS, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
               const aiTexture* paiTexture = pScene->GetEmbeddedTexture(path.C_Str());

                if (paiTexture) {
                    SPACE_ENGINE_DEBUG("Embeddeded specular texture type '{}'\n", paiTexture->achFormatHint);
                    int buffer_size = paiTexture->mWidth;
                    bool IsSRGB = false;
                    pTMPMesh->materials[materialIndex].get()->addTexture("specular_tex", TextureManager::load(Utils::getFileNameFormPath(path.C_Str()), buffer_size, paiTexture->pcData, IsSRGB));
                } 
                else 
                {
                    std::string fullPath = Utils::getFullPath(dir, path);
                    bool IsSRGB = false;
                    pTMPMesh->materials[materialIndex].get()->addTexture("specular_tex", TextureManager::load(Utils::getFileNameFormPath(path.C_Str()), IsSRGB));
                    SPACE_ENGINE_INFO("Loaded specular texture '{}' at index {}", fullPath.c_str(), materialIndex);
                }
            }
        }
    }

/*
    void MeshManager::loadSpecularTextureEmbedded(const aiTexture* paiTexture, int materialIndex)
    {
        SPACE_ENGINE_DEBUG("Embeddeded specular texture type '{}'", paiTexture->achFormatHint);
        materials[materialIndex].pTextures[TEXTURE_TYPE::SPECULAR] = new Texture(GL_TEXTURE_2D);
        int buffer_size = paiTexture->mWidth;
        bool IsSRGB = false;
        materials[materialIndex].pTextures[TEXTURE_TYPE::SPECULAR]->load(buffer_size, paiTexture->pcData, IsSRGB);
    }


    void MeshManager::loadSpecularTextureFromFile(const std::string& dir, const aiString& path, int materialIndex)
    {
        std::string fullPath = getFullPath(dir, path);

        materials[materialIndex].pTextures[TEXTURE_TYPE::SPECULAR] = new Texture(GL_TEXTURE_2D, fullPath.c_str());

        bool IsSRGB = false;

        if (!materials[materialIndex].pTextures[TEXTURE_TYPE::SPECULAR]->load(IsSRGB)) {
            SPACE_ENGINE_ERROR("Error loading specular texture '{}'", fullPath.c_str());
            exit(0);
        }
        else {
            SPACE_ENGINE_INFO("Loaded specular texture '{}'", fullPath.c_str());
        }
    }
*/

    void MeshManager::loadAlbedoTexture(const std::string& dir, const aiMaterial* pMaterial, const aiScene* pScene, int materialIndex)
    {
        if (pMaterial->GetTextureCount(aiTextureType_BASE_COLOR) > 0) {
            aiString path;

            if (pMaterial->GetTexture(aiTextureType_BASE_COLOR, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                const aiTexture* paiTexture = pScene->GetEmbeddedTexture(path.C_Str());

                if (paiTexture) {
                    SPACE_ENGINE_DEBUG("Embeddeded albedo texture type '{}'\n", paiTexture->achFormatHint);
                    int buffer_size = paiTexture->mWidth;
                    bool IsSRGB = true;
                    pTMPMesh->materials[materialIndex].get()->addTexture("albedo_tex", TextureManager::load(Utils::getFileNameFormPath(path.C_Str()), buffer_size, paiTexture->pcData, IsSRGB));
                } 
                else 
                {
                    std::string fullPath = Utils::getFullPath(dir, path);
                    bool IsSRGB = true;
                    pTMPMesh->materials[materialIndex].get()->addTexture("albedo_tex", TextureManager::load(Utils::getFileNameFormPath(path.C_Str()), IsSRGB));
                    SPACE_ENGINE_INFO("Loaded albedo texture '{}' at index {}", fullPath.c_str(), materialIndex);
                }
            }
        }
    }

/*
    void MeshManager::loadAlbedoTextureEmbedded(const aiTexture* paiTexture, int materialIndex)
    {
        SPACE_ENGINE_DEBUG("Embeddeded albedo texture type '{}'", paiTexture->achFormatHint);
        materials[materialIndex].PBRmaterial.pAlbedo = new Texture(GL_TEXTURE_2D);
        int buffer_size = paiTexture->mWidth;
        bool IsSRGB = true;
        materials[materialIndex].PBRmaterial.pAlbedo->load(buffer_size, paiTexture->pcData, IsSRGB);
    }


    void MeshManager::loadAlbedoTextureFromFile(const std::string& dir, const aiString& path, int materialIndex)
    {
        std::string FullPath = getFullPath(dir, path);

        materials[materialIndex].PBRmaterial.pAlbedo = new Texture(GL_TEXTURE_2D, FullPath.c_str());

        bool IsSRGB = true;

        if (!materials[materialIndex].PBRmaterial.pAlbedo->load(IsSRGB)) {
            SPACE_ENGINE_ERROR("Error loading albedo texture '{}'", FullPath.c_str());
            exit(0);
        }
        else {
            SPACE_ENGINE_INFO("Loaded albedo texture '{}'", FullPath.c_str());
        }
    }
*/

    void MeshManager::loadMetalnessTexture(const std::string& dir, const aiMaterial* pMaterial, const aiScene* pScene, int materialIndex)
    {

        int NumTextures = pMaterial->GetTextureCount(aiTextureType_METALNESS);

        if (NumTextures > 0) {
            SPACE_ENGINE_DEBUG("Num metalness textures {}", NumTextures);

            aiString path;

            if (pMaterial->GetTexture(aiTextureType_METALNESS, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                const aiTexture* paiTexture = pScene->GetEmbeddedTexture(path.C_Str());

                if (paiTexture) {
                    SPACE_ENGINE_DEBUG("Embeddeded metalness texture type '{}'\n", paiTexture->achFormatHint);
                    int buffer_size = paiTexture->mWidth;
                    bool IsSRGB = false;
                    pTMPMesh->materials[materialIndex].get()->addTexture("metalness_tex", TextureManager::load(Utils::getFileNameFormPath(path.C_Str()), buffer_size, paiTexture->pcData, IsSRGB));
                } 
                else 
                {
                    std::string fullPath = Utils::getFullPath(dir, path);
                    bool IsSRGB = false;
                    pTMPMesh->materials[materialIndex].get()->addTexture("metalness_tex", TextureManager::load(Utils::getFileNameFormPath(path.C_Str()), IsSRGB));
                    SPACE_ENGINE_INFO("Loaded metalness texture '{}' at index {}", fullPath.c_str(), materialIndex);
                }
            }
        }
    }

/*
    void MeshManager::loadMetalnessTextureEmbedded(const aiTexture* paiTexture, int materialIndex)
    {
        SPACE_ENGINE_DEBUG("Embeddeded metalness texture type '{}'", paiTexture->achFormatHint);
        materials[materialIndex].PBRmaterial.pMetallic = new Texture(GL_TEXTURE_2D);
        int buffer_size = paiTexture->mWidth;
        bool IsSRGB = false;
        materials[materialIndex].PBRmaterial.pMetallic->load(buffer_size, paiTexture->pcData, IsSRGB);
    }


    void MeshManager::loadMetalnessTextureFromFile(const std::string& dir, const aiString& path, int materialIndex)
    {
        std::string FullPath = getFullPath(dir, path);

        materials[materialIndex].PBRmaterial.pMetallic = new Texture(GL_TEXTURE_2D, FullPath.c_str());

        bool IsSRGB = false;

        if (!materials[materialIndex].PBRmaterial.pMetallic->load(IsSRGB)) {
            SPACE_ENGINE_ERROR("Error loading metalness texture '{}'", FullPath.c_str());
            exit(0);
        }
        else {
            SPACE_ENGINE_INFO("Loaded metalness texture '{}'", FullPath.c_str());
        }
    }
*/

    void MeshManager::loadRoughnessTexture(const std::string& dir, const aiMaterial* pMaterial, const aiScene* pScene, int materialIndex)
    {
        int NumTextures = pMaterial->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS);

        if (NumTextures > 0) {
            SPACE_ENGINE_DEBUG("Num roughness textures {}", NumTextures);

            aiString path;

            if (pMaterial->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                const aiTexture* paiTexture = pScene->GetEmbeddedTexture(path.C_Str());

                if (paiTexture) {
                    SPACE_ENGINE_DEBUG("Embeddeded roughness texture type '{}'\n", paiTexture->achFormatHint);
                    int buffer_size = paiTexture->mWidth;
                    bool IsSRGB = false;
                    pTMPMesh->materials[materialIndex].get()->addTexture("roughness_tex", TextureManager::load(Utils::getFileNameFormPath(path.C_Str()), buffer_size, paiTexture->pcData, IsSRGB));
                } 
                else 
                {
                    std::string fullPath = Utils::getFullPath(dir, path);
                    bool IsSRGB = false;
                    pTMPMesh->materials[materialIndex].get()->addTexture("roughness_tex", TextureManager::load(Utils::getFileNameFormPath(path.C_Str()), IsSRGB));
                    SPACE_ENGINE_INFO("Loaded roughness texture '{}' at index {}", fullPath.c_str(), materialIndex);
                }
            }
        }
    }

/*
    void MeshManager::loadRoughnessTextureEmbedded(const aiTexture* paiTexture, int materialIndex)
    {
        SPACE_ENGINE_DEBUG("Embeddeded roughness texture type '{}'", paiTexture->achFormatHint);
        materials[materialIndex].PBRmaterial.pRoughness = new Texture(GL_TEXTURE_2D);
        int buffer_size = paiTexture->mWidth;
        bool IsSRGB = false;
        materials[materialIndex].PBRmaterial.pRoughness->load(buffer_size, paiTexture->pcData, IsSRGB);
    }


    void MeshManager::loadRoughnessTextureFromFile(const std::string& dir, const aiString& path, int materialIndex)
    {
        std::string FullPath = Utils::getFullPath(dir, path);

        materials[materialIndex].PBRmaterial.pRoughness = new Texture(GL_TEXTURE_2D, FullPath.c_str());
        bool IsSRGB = false;

        if (!materials[materialIndex].PBRmaterial.pRoughness->load(IsSRGB)) {
            SPACE_ENGINE_ERROR("Error loading roughness texture '{}'", FullPath.c_str());
            exit(0);
        }
        else {
            SPACE_ENGINE_INFO("Loaded roughness texture '{}'", FullPath.c_str());
        }
    }
*/

    void MeshManager::loadColors(const aiMaterial* pMaterial, int index)
    {
        aiColor4D ambientColor(0.0f, 0.0f, 0.0f, 0.0f);
        Vector4 allOnes(1.0f);

        int shadingModel = 0;
        if (pMaterial->Get(AI_MATKEY_SHADING_MODEL, shadingModel) == AI_SUCCESS) {
            SPACE_ENGINE_DEBUG("Shading model {}", shadingModel);
        }

        if (pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor) == AI_SUCCESS) {
            SPACE_ENGINE_INFO("Loaded ambient color [{} {} {}]", ambientColor.r, ambientColor.g, ambientColor.b);
            std::get<Vector4>(pTMPMesh->materials[index].get()->props["base_color_val"]).r = ambientColor.r;
            std::get<Vector4>(pTMPMesh->materials[index].get()->props["base_color_val"]).g = ambientColor.g;
            std::get<Vector4>(pTMPMesh->materials[index].get()->props["base_color_val"]).b = ambientColor.b;
        } else {
            std::get<Vector4>(pTMPMesh->materials[index].get()->props["base_color_val"]) = allOnes;
        }

        aiColor3D diffuseColor(0.0f, 0.0f, 0.0f);

        if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS) {
            SPACE_ENGINE_INFO("Loaded diffuse color [{} {} {}]", diffuseColor.r, diffuseColor.g, diffuseColor.b);
            std::get<Vector3>(pTMPMesh->materials[index].get()->props["diffuse_color_val"]).r = diffuseColor.r;
            std::get<Vector3>(pTMPMesh->materials[index].get()->props["diffuse_color_val"]).g = diffuseColor.g;
            std::get<Vector3>(pTMPMesh->materials[index].get()->props["diffuse_color_val"]).b = diffuseColor.b;
        }

        aiColor3D specularColor(0.0f, 0.0f, 0.0f);

        if (pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == AI_SUCCESS) {
            SPACE_ENGINE_INFO("Loaded specular color [{} {} {}]\n", specularColor.r, specularColor.g, specularColor.b);
            std::get<Vector3>(pTMPMesh->materials[index].get()->props["specular_color_val"]).r = specularColor.r;
            std::get<Vector3>(pTMPMesh->materials[index].get()->props["specular_color_val"]).g = specularColor.g;
            std::get<Vector3>(pTMPMesh->materials[index].get()->props["specular_color_val"]).b = specularColor.b;
        }
    }

    void Mesh::populateBuffers()
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffers[VERTEX_BUFFER]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[INDEX_BUFFER]);

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

        size_t NumFloats = 0;

        glEnableVertexAttribArray(POSITION_LOCATION);
        glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(NumFloats * sizeof(float)));
        NumFloats += 3;

        glEnableVertexAttribArray(TEX_COORD_LOCATION);
        glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(NumFloats * sizeof(float)));
        NumFloats += 2;

        glEnableVertexAttribArray(NORMAL_LOCATION);
        glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(NumFloats * sizeof(float)));
    }

    /*
    void Mesh::render()
    {
        if (isPBR) {
            setupRenderMaterialsPBR();
        }

        glBindVertexArray(VAO);

        for (unsigned int meshIndex = 0 ; meshIndex < meshes.size() ; meshIndex++) {
            unsigned int materialIndex = meshes[meshIndex].materialIndex;
            assert(materialIndex < materials.size());

            glDrawElementsBaseVertex(GL_TRIANGLES,
                                     meshes[meshIndex].numIndices,
                                     GL_UNSIGNED_INT,
                                     (void*)(sizeof(unsigned int) * meshes[meshIndex].baseIndex),
                                     meshes[meshIndex].baseVertex);
        }

        // Make sure the VAO is not changed from the outside
        glBindVertexArray(0);
    }

    void Mesh::setupRenderMaterialsPBR()
    {
        int PBRMaterialIndex = 0;

        if (materials[PBRMaterialIndex].PBRmaterial.pAlbedo) {
            materials[PBRMaterialIndex].PBRmaterial.pAlbedo->bind(ALBEDO_TEXTURE_UNIT);
        }

        if (materials[PBRMaterialIndex].PBRmaterial.pRoughness) {
            materials[PBRMaterialIndex].PBRmaterial.pRoughness->bind(ROUGHNESS_TEXTURE_UNIT);
        }

        if (materials[PBRMaterialIndex].PBRmaterial.pMetallic) {
            materials[PBRMaterialIndex].PBRmaterial.pMetallic->bind(METALLIC_TEXTURE_UNIT);
        }

        if (materials[PBRMaterialIndex].PBRmaterial.pNormalMap) {
            materials[PBRMaterialIndex].PBRmaterial.pNormalMap->bind(NORMAL_TEXTURE_UNIT);
        }

    }

    
    void Mesh::render(unsigned int drawIndex, unsigned int primID)
    {
        glBindVertexArray(VAO);

        unsigned int materialIndex = meshes[drawIndex].materialIndex;
        assert(materialIndex < materials.size());

        if (materials[materialIndex].pTextures[TEXTURE_TYPE::BASE]) {
            materials[materialIndex].pTextures[TEXTURE_TYPE::BASE]->bind(COLOR_TEXTURE_UNIT);
        }

        if (materials[materialIndex].pTextures[TEXTURE_TYPE::SPECULAR]) {
            materials[materialIndex].pTextures[TEXTURE_TYPE::SPECULAR]->bind(SPECULAR_EXPONENT_UNIT);
        }

        glDrawElementsBaseVertex(GL_TRIANGLES,
                                 3,
                                 GL_UNSIGNED_INT,
                                 (void*)(sizeof(unsigned int) * (meshes[drawIndex].baseIndex + primID * 3)),
                                 meshes[drawIndex].baseVertex);

        // Make sure the VAO is not changed from the outside
        glBindVertexArray(0);
    }



    // Used only by instancing
    void Mesh::render(unsigned int numInstannes, const Matrix4* WVPMats, const Matrix4* worldMats)
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffers[WVP_MAT_BUFFER]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Matrix4) * numInstannes, WVPMats, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, buffers[WORLD_MAT_BUFFER]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Matrix4) * numInstannes, worldMats, GL_DYNAMIC_DRAW);

        glBindVertexArray(VAO);

        for (unsigned int i = 0 ; i < meshes.size() ; i++) {
            const unsigned int materialIndex = meshes[i].materialIndex;

            assert(materialIndex < materials.size());

            if (materials[materialIndex].pTextures[TEXTURE_TYPE::BASE]) {
                materials[materialIndex].pTextures[TEXTURE_TYPE::BASE]->bind(COLOR_TEXTURE_UNIT);
            }

            if (materials[materialIndex].pTextures[TEXTURE_TYPE::SPECULAR]) {
                materials[materialIndex].pTextures[TEXTURE_TYPE::SPECULAR]->bind(SPECULAR_EXPONENT_UNIT);
            }

            glDrawElementsInstancedBaseVertex(GL_TRIANGLES,
                                              meshes[i].numIndices,
                                              GL_UNSIGNED_INT,
                                              (void*)(sizeof(unsigned int) * meshes[i].baseIndex),
                                              numInstannes,
                                              meshes[i].baseVertex);
        }

        // Make sure the VAO is not changed from the outside
        glBindVertexArray(0);
    }
*/
/*
    const BaseMaterial& Mesh::getMaterial()
    {
        for (unsigned int i = 0 ; i < materials.size() ; i++) {
            if (materials[i].ambientColor != Vector4(0.0f)) {
                return materials[i];
            }
        }

        if (materials.size() == 0) {
            SPACE_ENGINE_ERROR("No materials");
            exit(0);
        }

        return materials[0];
    }
*/
/*
    void Mesh::getLeadingVertex(uint32_t drawIndex, uint32_t primID, Vector3& vertex)
    {
        uint32_t meshIndex = drawIndex; // Each mesh is rendered in its own draw call

        assert(meshIndex < pScene->mNumMeshes);
        const aiMesh* paiMesh = pScene->mMeshes[meshIndex];

        assert(primID < paiMesh->mNumFaces);
        const aiFace& Face = paiMesh->mFaces[primID];

        uint32_t LeadingIndex = Face.mIndices[0];

        assert(LeadingIndex < paiMesh->mNumVertices);
        const aiVector3D& Pos = paiMesh->mVertices[LeadingIndex];
        vertex.x = Pos.x;
        vertex.y = Pos.y;
        vertex.z = Pos.z;
    }
*/
}