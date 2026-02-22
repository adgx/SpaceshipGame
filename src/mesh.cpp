#include "mesh.h"
#include "log.h"

#include <algorithm>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "utils/utils.h"
#include "texture.h"

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a) / sizeof(a[0]))
#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenSmoothNormals | \
                           aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices)

#define POSITION_LOCATION 0
#define TEX_COORD_LOCATION 1
#define NORMAL_LOCATION 2

namespace SpaceEngine
{

        const static int N_VERTEX_QUAD = 12;
    const static int N_TEXTURE_CORD_QUAD = 8;
    const static int N_INDICES_QUAD = 6;
    // data quad vertex coords,  texture coords
    float UIQuad[] =
        {
            // vertex  texture
            0.f, 0.f, 0.f, 0.f, // bottom-left
            1.f, 1.f, 1.f, 1.f, // top-right
            0.f, 1.f, 0.f, 1.f, // top-left
            1.f, 0.f, 1.f, 0.f  // bottom-right
    };

    float plane[] =
        {
            // vertex  texture
            -1.f, -1.f, 0.f, 0.f, // bottom-left
             1.f,  1.f, 1.f, 1.f, // top-right
            -1.f,  1.f, 0.f, 1.f, // top-left
             1.f, -1.f, 1.f, 0.f  // bottom-right
    };
    uint32_t UIindices[N_INDICES_QUAD] =
        {
            0, 1, 2,
            0, 3, 1};


    //---------------------------------------------//
    //-------------------Mesh----------------------//
    //---------------------------------------------//
    void Mesh::clear()
    {
        if (buffers[0] != 0)
        {
            glDeleteBuffers(ARRAY_SIZE_IN_ELEMENTS(buffers), buffers);
        }

        if (VAO != 0)
        {
            glDeleteVertexArrays(1, &VAO);
            VAO = 0;
        }
    }

    void Mesh::bindVAO()
    {
        SPACE_ENGINE_ASSERT(VAO, "VAO is not allocated");
        glBindVertexArray(VAO);
    }

    int Mesh::getNumSubMesh()
    {
        return static_cast<int>(subMeshes.size());
    }

    int Mesh::bindMaterialToSubMeshIndex(int index, BaseMaterial *pMat)
    {
        // TODO: check if there is a submesh just bind to this material
        if (!pMat)
        {
            SPACE_ENGINE_ERROR("Material is nullptr")
            return -1;
        }

        if (index >= subMeshes.size())
        {
            SPACE_ENGINE_ERROR("Index out of rangeis nullptr")
            return -1;
        }

        materials.push_back(pMat);
        subMeshes[index].materialIndex = static_cast<uint32_t>(materials.size()-1);

        return 1;
    }

    //---------------------------------------------//
    //---------------BaseMaterial------------------//
    //---------------------------------------------//
    BaseMaterial *Mesh::getMaterialBySubMeshIndex(int index)
    {
        if (index >= subMeshes.size())
        {
            SPACE_ENGINE_ERROR("Index out of rangeis of number of meshes");
            return nullptr;
        }

        int indexMat = subMeshes[index].materialIndex;

        if (indexMat >= materials.size())
        {
            SPACE_ENGINE_ERROR("Index out of rangeis of number of materials");
            return nullptr;
        }
        return materials[subMeshes[index].materialIndex];
    }


    //------------------------------------------//
    //--------------MeshManager-----------------//
    //------------------------------------------//
    Mesh *MeshManager::pTMPMesh = nullptr;
    UIMesh *MeshManager::pUIMesh = nullptr;
    TextMesh *MeshManager::pTextMesh = nullptr;
    PlaneMesh *MeshManager::pPlaneMesh = nullptr;
    std::unordered_map<std::string, Mesh *> MeshManager::meshMap;

    UIMesh *MeshManager::getUIMesh()
    {
        if (!pUIMesh)
        {
            pUIMesh = new UIMesh();
        }

        return pUIMesh;
    }

    Mesh *MeshManager::findMesh(const std::string &name)
    {
        if (meshMap.find(name) != meshMap.end())
            return meshMap[name];

        SPACE_ENGINE_WARN("MeshManager: Mesh not found");
        return nullptr;
    }

    TextMesh *MeshManager::getTextMesh()
    {
        if (!pTextMesh)
        {
            pTextMesh = new TextMesh();
        }

        return pTextMesh;
    }

    PlaneMesh* MeshManager::getPlaneMesh()
    {
        if(!pPlaneMesh)
        {
            pPlaneMesh = new PlaneMesh();
        }
        
        return pPlaneMesh;
    }

    Mesh *MeshManager::loadMesh(const std::string &fileName)
    {
        pTMPMesh = nullptr;
        std::string name = Utils::getFileNameNoExt(fileName);

        if (pTMPMesh = findMesh(name), !pTMPMesh)
        {
            Mesh *pMesh = new Mesh();
            pTMPMesh = pMesh;
            // VAO
            glGenVertexArrays(1, &pTMPMesh->VAO);
            glBindVertexArray(pTMPMesh->VAO);
            glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(pTMPMesh->buffers), pTMPMesh->buffers);

            Assimp::Importer importer;
            std::string fullPath(MESHES_PATH + fileName);
            const aiScene *pScene = importer.ReadFile(fullPath.c_str(), ASSIMP_LOAD_FLAGS);
            pTMPMesh->name = name;
            auto glError = glGetError();

            if (pScene)
            {
                if (!initFromScene(pScene, fileName))
                {
                    SPACE_ENGINE_FATAL("Impossible to init the scene:{}", fileName);
                    exit(-1);
                }
            }
            else
            {
                SPACE_ENGINE_FATAL("Impossible to load the model");
            }

            meshMap[name] = pMesh;
        }
        return pTMPMesh;
    }

    bool MeshManager::initFromScene(const aiScene *pScene, const std::string &fileName)
    {
        // number of the  subMeshes in the scene
        pTMPMesh->subMeshes.resize(pScene->mNumMeshes);
        pTMPMesh->materials.resize(pScene->mNumMaterials);

        unsigned int numVertices = 0;
        unsigned int numIndices = 0;
        // calculate the number of vertices and indices in the scene
        countVerticesAndIndices(pScene, numVertices, numIndices);
        // allocate space in the host
        reserveSpace(numVertices, numIndices);

        initAllMeshes(pScene);

        if (!initMaterials(pScene, fileName))
        {
            return false;
        }

        pTMPMesh->populateBuffers();
        auto flag = glGetError();
        return flag == GL_NO_ERROR;
    }

    void MeshManager::countVerticesAndIndices(const aiScene *pScene, unsigned int &numVertices, unsigned int &numIndices)
    {
        for (unsigned int i = 0; i < pTMPMesh->subMeshes.size(); i++)
        {
            pTMPMesh->subMeshes[i].materialIndex = pScene->mMeshes[i]->mMaterialIndex;
            pTMPMesh->subMeshes[i].numIndices = pScene->mMeshes[i]->mNumFaces * 3;
            pTMPMesh->subMeshes[i].baseVertex = numVertices;
            pTMPMesh->subMeshes[i].baseIndex = numIndices;

            numVertices += pScene->mMeshes[i]->mNumVertices;
            numIndices += pTMPMesh->subMeshes[i].numIndices;
        }
    }

    void MeshManager::reserveSpace(unsigned int numVertices, unsigned int numIndices)
    {
        pTMPMesh->vertices.reserve(numVertices);
        pTMPMesh->indices.reserve(numIndices);
    }

    void MeshManager::initAllMeshes(const aiScene *pScene)
    {
        for (unsigned int i = 0; i < pTMPMesh->subMeshes.size(); i++)
        {
            const aiMesh *paiMesh = pScene->mMeshes[i];
            initSingleMesh(i, paiMesh);
        }
    }

    void MeshManager::initSingleMesh(uint32_t meshIndex, const aiMesh *paiMesh)
    {
        const aiVector3D zero3D(0.0f, 0.0f, 0.0f);

        // populate the vertex attribute vectors
        Mesh::Vertex v;

        for (unsigned int i = 0; i < paiMesh->mNumVertices; i++)
        {
            const aiVector3D &pPos = paiMesh->mVertices[i];

            v.position = Vector3(pPos.x, pPos.y, pPos.z);

            pTMPMesh->maxPos.x = std::max(pTMPMesh->maxPos.x, v.position.x);
            pTMPMesh->maxPos.y = std::max(pTMPMesh->maxPos.y, v.position.y);
            pTMPMesh->maxPos.z = std::max(pTMPMesh->maxPos.z, v.position.z);
            pTMPMesh->minPos.x = std::min(pTMPMesh->minPos.x, v.position.x);
            pTMPMesh->minPos.y = std::min(pTMPMesh->minPos.y, v.position.y);
            pTMPMesh->minPos.z = std::min(pTMPMesh->minPos.z, v.position.z);

            if (paiMesh->mNormals)
            {
                const aiVector3D &pNormal = paiMesh->mNormals[i];
                v.normal = Vector3(pNormal.x, pNormal.y, pNormal.z);
            }
            else
            {
                aiVector3D normal(0.0f, 1.0f, 0.0f);
                v.normal = Vector3(normal.x, normal.y, normal.z);
            }

            const aiVector3D &pTexCoord = paiMesh->HasTextureCoords(0) ? paiMesh->mTextureCoords[0][i] : zero3D;
            v.texCoords = Vector2(pTexCoord.x, pTexCoord.y);

            pTMPMesh->vertices.push_back(v);
        }

        // populate the index buffer
        for (unsigned int i = 0; i < paiMesh->mNumFaces; i++)
        {
            const aiFace &face = paiMesh->mFaces[i];
            pTMPMesh->indices.push_back(face.mIndices[0]);
            pTMPMesh->indices.push_back(face.mIndices[1]);
            pTMPMesh->indices.push_back(face.mIndices[2]);
        }
    }

    bool MeshManager::initMaterials(const aiScene *pScene, const std::string &fileName)
    {
        std::string dir = TEXTURES_PATH + pTMPMesh->name;

        SPACE_ENGINE_DEBUG("Num materials: {}", pScene->mNumMaterials);

        // Initialize the materials
        for (unsigned int i = 0; i < pScene->mNumMaterials; i++)
        {
            const aiMaterial *pMaterial = pScene->mMaterials[i];
            BaseMaterial *pbrMat = MaterialManager::createMaterial<PBRMaterial>(pTMPMesh->name + std::to_string(i));
            pbrMat->pShader = ShaderManager::findShaderProgram("pbr"); // TODO: make the biding
            if (!pbrMat->pShader)
            {
                SPACE_ENGINE_FATAL("Shader pbr not compiled")
                exit(-1);
            }

            pTMPMesh->materials[i] = std::move(pbrMat);
            loadTextures(dir, pMaterial, pScene, i);
            loadColors(pMaterial, i);
        }

        return true;
    }

    void MeshManager::loadTextures(const std::string &dir, const aiMaterial *pMaterial, const aiScene *pScene, int index)
    {
        auto glError = glGetError();
        loadDiffuseTexture(dir, pMaterial, pScene, index); // diffuse and albedo are the same
        glError = glGetError();
        // loadSpecularTexture(dir, pMaterial, pScene, index);
        loadNormalsTexture(dir, pMaterial, pScene, index);
        glError = glGetError();

        // PBR
        // loadAlbedoTexture(dir, pMaterial, pScene, index);
        loadMetalnessTexture(dir, pMaterial, pScene, index);
        glError = glGetError();
        loadRoughnessTexture(dir, pMaterial, pScene, index);
        glError = glGetError();
        loadAmbientOcclusionTexture(dir, pMaterial, pScene, index);
        glError = glGetError();
    }

    void MeshManager::loadDiffuseTexture(const std::string &dir, const aiMaterial *pMaterial, const aiScene *pScene, int materialIndex)
    {
        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString path;

            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
            {
                const aiTexture *paiTexture = pScene->GetEmbeddedTexture(path.C_Str());

                if (paiTexture)
                {
                    SPACE_ENGINE_DEBUG("Embeddeded diffuse texture type '{}'\n", paiTexture->achFormatHint);
                    int buffer_size = paiTexture->mWidth;
                    bool IsSRGB = true;
                    pTMPMesh->materials[materialIndex]->addTexture("albedo_tex", TextureManager::load(Utils::getFileNameFormPath(path.C_Str()), buffer_size, paiTexture->pcData, IsSRGB));
                }
                else
                {
                    std::string fullPath = Utils::getFullPath(dir, path);
                    bool IsSRGB = true;
                    pTMPMesh->materials[materialIndex]->addTexture("albedo_tex", TextureManager::load(fullPath, IsSRGB));
                    SPACE_ENGINE_INFO("Loaded diffuse texture '{}' at indexMaterial {}", fullPath.c_str(), materialIndex);
                }
            }
        }
    }

    void MeshManager::loadSpecularTexture(const std::string &dir, const aiMaterial *pMaterial, const aiScene *pScene, int materialIndex)
    {
        if (pMaterial->GetTextureCount(aiTextureType_SHININESS) > 0)
        {
            aiString path;

            if (pMaterial->GetTexture(aiTextureType_SHININESS, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
            {
                const aiTexture *paiTexture = pScene->GetEmbeddedTexture(path.C_Str());

                if (paiTexture)
                {
                    SPACE_ENGINE_DEBUG("Embeddeded specular texture type '{}'\n", paiTexture->achFormatHint);
                    int buffer_size = paiTexture->mWidth;
                    bool IsSRGB = false;
                    pTMPMesh->materials[materialIndex]->addTexture("specular_tex", TextureManager::load(Utils::getFileNameFormPath(path.C_Str()), buffer_size, paiTexture->pcData, IsSRGB));
                }
                else
                {
                    std::string fullPath = Utils::getFullPath(dir, path);
                    bool IsSRGB = false;
                    pTMPMesh->materials[materialIndex]->addTexture("specular_tex", TextureManager::load(fullPath, IsSRGB));
                    SPACE_ENGINE_INFO("Loaded specular texture '{}' at index {}", fullPath.c_str(), materialIndex);
                }
            }
        }
    }

    void MeshManager::loadAlbedoTexture(const std::string &dir, const aiMaterial *pMaterial, const aiScene *pScene, int materialIndex)
    {
        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString path;

            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
            {
                const aiTexture *paiTexture = pScene->GetEmbeddedTexture(path.C_Str());

                if (paiTexture)
                {
                    SPACE_ENGINE_DEBUG("Embeddeded albedo texture type '{}'\n", paiTexture->achFormatHint);
                    int buffer_size = paiTexture->mWidth;
                    bool IsSRGB = true;
                    pTMPMesh->materials[materialIndex]->addTexture("albedo_tex", TextureManager::load(Utils::getFileNameFormPath(path.C_Str()), buffer_size, paiTexture->pcData, IsSRGB));
                }
                else
                {
                    std::string fullPath = Utils::getFullPath(dir, path);
                    bool IsSRGB = true;
                    pTMPMesh->materials[materialIndex]->addTexture("albedo_tex", TextureManager::load(fullPath, IsSRGB));
                    SPACE_ENGINE_INFO("Loaded albedo texture '{}' at index {}", fullPath.c_str(), materialIndex);
                }
            }
        }
    }

    void MeshManager::loadMetalnessTexture(const std::string &dir, const aiMaterial *pMaterial, const aiScene *pScene, int materialIndex)
    {

        int NumTextures = pMaterial->GetTextureCount(aiTextureType_METALNESS);

        if (NumTextures > 0)
        {
            SPACE_ENGINE_DEBUG("Num metalness textures {}", NumTextures);

            aiString path;

            if (pMaterial->GetTexture(aiTextureType_METALNESS, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
            {
                const aiTexture *paiTexture = pScene->GetEmbeddedTexture(path.C_Str());

                if (paiTexture)
                {
                    SPACE_ENGINE_DEBUG("Embeddeded metalness texture type '{}'\n", paiTexture->achFormatHint);
                    int buffer_size = paiTexture->mWidth;
                    bool IsSRGB = false;
                    pTMPMesh->materials[materialIndex]->addTexture("metalness_tex", TextureManager::load(Utils::getFileNameFormPath(path.C_Str()), buffer_size, paiTexture->pcData, IsSRGB));
                }
                else
                {
                    std::string fullPath = Utils::getFullPath(dir, path);
                    bool IsSRGB = false;
                    pTMPMesh->materials[materialIndex]->addTexture("metalness_tex", TextureManager::load(fullPath, IsSRGB));
                    SPACE_ENGINE_INFO("Loaded metalness texture '{}' at index {}", fullPath.c_str(), materialIndex);
                }
            }
        }
    }

    void MeshManager::loadRoughnessTexture(const std::string &dir, const aiMaterial *pMaterial, const aiScene *pScene, int materialIndex)
    {
        int NumTextures = pMaterial->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS);

        if (NumTextures > 0)
        {
            SPACE_ENGINE_DEBUG("Num roughness textures {}", NumTextures);

            aiString path;

            if (pMaterial->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
            {
                const aiTexture *paiTexture = pScene->GetEmbeddedTexture(path.C_Str());

                if (paiTexture)
                {
                    SPACE_ENGINE_DEBUG("Embeddeded roughness texture type '{}'\n", paiTexture->achFormatHint);
                    int buffer_size = paiTexture->mWidth;
                    bool IsSRGB = false;
                    pTMPMesh->materials[materialIndex]->addTexture("roughness_tex", TextureManager::load(Utils::getFileNameFormPath(path.C_Str()), buffer_size, paiTexture->pcData, IsSRGB));
                }
                else
                {
                    std::string fullPath = Utils::getFullPath(dir, path);
                    bool IsSRGB = false;
                    pTMPMesh->materials[materialIndex]->addTexture("roughness_tex", TextureManager::load(fullPath, IsSRGB));
                    SPACE_ENGINE_INFO("Loaded roughness texture '{}' at index {}", fullPath.c_str(), materialIndex);
                }
            }
        }
    }

    // aiTextureType_NORMALS
    void MeshManager::loadNormalsTexture(const std::string &dir, const aiMaterial *pMaterial, const aiScene *pScene, int materialIndex)
    {
        int NumTextures = pMaterial->GetTextureCount(aiTextureType_NORMALS);

        if (NumTextures > 0)
        {
            SPACE_ENGINE_DEBUG("Num Normals textures {}", NumTextures);

            aiString path;

            if (pMaterial->GetTexture(aiTextureType_NORMALS, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
            {
                const aiTexture *paiTexture = pScene->GetEmbeddedTexture(path.C_Str());

                if (paiTexture)
                {
                    SPACE_ENGINE_DEBUG("Embeddeded normals texture type '{}'\n", paiTexture->achFormatHint);
                    int buffer_size = paiTexture->mWidth;
                    bool IsSRGB = false;
                    pTMPMesh->materials[materialIndex]->addTexture("normal_map_tex", TextureManager::load(Utils::getFileNameFormPath(path.C_Str()), buffer_size, paiTexture->pcData, IsSRGB));
                }
                else
                {
                    std::string fullPath = Utils::getFullPath(dir, path);
                    bool IsSRGB = false;
                    pTMPMesh->materials[materialIndex]->addTexture("normal_map_tex", TextureManager::load(fullPath, IsSRGB));
                    SPACE_ENGINE_INFO("Loaded normals texture '{}' at index {}", fullPath.c_str(), materialIndex);
                }
            }
        }
    }

    void MeshManager::loadAmbientOcclusionTexture(const std::string &dir, const aiMaterial *pMaterial, const aiScene *pScene, int materialIndex)
    {
        int NumTextures = pMaterial->GetTextureCount(aiTextureType_AMBIENT);

        if (NumTextures > 0)
        {
            SPACE_ENGINE_DEBUG("Num Ambient Occlusion textures {}", NumTextures);

            aiString path;

            if (pMaterial->GetTexture(aiTextureType_AMBIENT, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
            {
                const aiTexture *paiTexture = pScene->GetEmbeddedTexture(path.C_Str());

                if (paiTexture)
                {
                    SPACE_ENGINE_DEBUG("Embeddeded Ambient Occlusion texture type '{}'\n", paiTexture->achFormatHint);
                    int buffer_size = paiTexture->mWidth;
                    bool IsSRGB = false;
                    pTMPMesh->materials[materialIndex]->addTexture("ambient_occlusion_tex", TextureManager::load(Utils::getFileNameFormPath(path.C_Str()), buffer_size, paiTexture->pcData, IsSRGB));
                }
                else
                {
                    std::string fullPath = Utils::getFullPath(dir, path);
                    bool IsSRGB = false;
                    pTMPMesh->materials[materialIndex]->addTexture("ambient_occlusion_tex", TextureManager::load(fullPath, IsSRGB));
                    SPACE_ENGINE_INFO("Loaded Ambient Occlusion texture '{}' at index {}", fullPath.c_str(), materialIndex);
                }
            }
        }
    }

    void MeshManager::loadColors(const aiMaterial *pMaterial, int index)
    {
        aiColor4D ambientColor(0.0f, 0.0f, 0.0f, 0.0f);
        Vector4 allOnes(1.0f);

        int shadingModel = 0;
        if (pMaterial->Get(AI_MATKEY_SHADING_MODEL, shadingModel) == AI_SUCCESS)
        {
            SPACE_ENGINE_DEBUG("Shading model {}", shadingModel);
        }

        if (pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor) == AI_SUCCESS)
        {
            SPACE_ENGINE_INFO("Loaded ambient color [{} {} {}]", ambientColor.r, ambientColor.g, ambientColor.b);
            std::get<float>(pTMPMesh->materials[index]->props["ambient_occlusion_val"]) = ambientColor.r;
        }
        else
        {
            std::get<float>(pTMPMesh->materials[index]->props["base_color_val"]) = 1.f;
        }

        aiColor3D albedoColor(1.0f, 1.0f, 1.0f);

        if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, albedoColor) == AI_SUCCESS)
        {
            SPACE_ENGINE_INFO("Loaded diffuse color [{} {} {}]", albedoColor.r, albedoColor.g, albedoColor.b);
            std::get<Vector4>(pTMPMesh->materials[index]->props["albedo_color_val"]).r = albedoColor.r;
            std::get<Vector4>(pTMPMesh->materials[index]->props["albedo_color_val"]).g = albedoColor.g;
            std::get<Vector4>(pTMPMesh->materials[index]->props["albedo_color_val"]).b = albedoColor.b;
        }
        // todo: metalness and roughness
        // aiColor3D specularColor(0.0f, 0.0f, 0.0f);
        //
        // if (pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == AI_SUCCESS) {
        //    SPACE_ENGINE_INFO("Loaded specular color [{} {} {}]\n", specularColor.r, specularColor.g, specularColor.b);
        //    std::get<Vector3>(pTMPMesh->materials[index]->props["specular_color_val"]).r = specularColor.r;
        //    std::get<Vector3>(pTMPMesh->materials[index]->props["specular_color_val"]).g = specularColor.g;
        //    std::get<Vector3>(pTMPMesh->materials[index]->props["specular_color_val"]).b = specularColor.b;
        //}
    }

    //------------------------------------------//
    //-------------------Mesh-------------------//
    //------------------------------------------//

    void Mesh::populateBuffers()
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffers[VERTEX_BUFFER]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[INDEX_BUFFER]);

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

        size_t NumFloats = 0;

        glEnableVertexAttribArray(POSITION_LOCATION);
        glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)(NumFloats * sizeof(float)));
        NumFloats += 3;

        glEnableVertexAttribArray(TEX_COORD_LOCATION);
        glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)(NumFloats * sizeof(float)));
        NumFloats += 2;

        glEnableVertexAttribArray(NORMAL_LOCATION);
        glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)(NumFloats * sizeof(float)));
    }

    void Mesh::drawSubMesh(unsigned int idSubMesh)
    {
        glDrawElementsBaseVertex(GL_TRIANGLES,
                                 subMeshes[idSubMesh].numIndices,
                                 GL_UNSIGNED_INT,
                                 (void *)(sizeof(unsigned int) * (subMeshes[idSubMesh].baseIndex)),
                                 subMeshes[idSubMesh].baseVertex);

        // Make sure the VAO is not changed from the outside
        glBindVertexArray(0);
    }

    //------------------------------------------//
    //-----------------UIMesh-------------------//
    //------------------------------------------//

    UIMesh::UIMesh()
    {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        glGenBuffers(2, buffers);
        populateBuffers();
    }

    void UIMesh::populateBuffers()
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);         // ok
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]); // ok

        glBufferData(GL_ARRAY_BUFFER, sizeof(UIQuad), UIQuad, GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(UIindices), UIindices, GL_STATIC_DRAW);

        size_t NumFloats = 0;

        glVertexAttribPointer(POSITION_LOCATION, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void *)(NumFloats * sizeof(float)));
        glEnableVertexAttribArray(POSITION_LOCATION);
        NumFloats += 2;

        glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void *)(NumFloats * sizeof(float)));
        glEnableVertexAttribArray(TEX_COORD_LOCATION);
    }

    void UIMesh::draw()
    {
        glDrawElements(GL_TRIANGLES,
                       N_INDICES_QUAD,
                       GL_UNSIGNED_INT,
                       0);
        GL_CHECK_ERRORS();
        // Make sure the VAO is not changed from the outside
        glBindVertexArray(0);
    }

    void UIMesh::bindVAO()
    {
        glBindVertexArray(VAO);
    }

    //------------------------------------------//
    //--------------UIMeshRenderer--------------//
    //------------------------------------------//

    UIMeshRenderer::UIMeshRenderer()
    {
        pMesh = MeshManager::getUIMesh();
    }

    int UIMeshRenderer::bindMaterial(UIMaterial *pMat)
    {
        if (!pMat)
        {
            SPACE_ENGINE_ERROR("Material is nullptr")
            return -1;
        }

        this->pMat = pMat;

        return 1;
    }

    UIMaterial *UIMeshRenderer::getMaterial()
    {
        return pMat;
    }

    inline void UIMeshRenderer::bindVAO()
    {
        pMesh->bindVAO();
    }

    UIMesh *UIMeshRenderer::getUIMesh()
    {
        return pMesh;
    }

    //---------------------------------------------//
    //-------------------TextMesh------------------//
    //---------------------------------------------//
    TextMesh::TextMesh()
    {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        glGenBuffers(1, &buffer);
        populateBuffers();
    }

    void TextMesh::populateBuffers()
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void TextMesh::draw()
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void TextMesh::bindVAO()
    {
        glBindVertexArray(VAO);
    }

    void TextMesh::subData(const std::array<std::array<float, 4>, 6>& vertices)
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*4*6, vertices.data());
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        GL_CHECK_ERRORS();
    }

    //---------------------------------------------//
    //--------------TextMeshRenderer---------------//
    //---------------------------------------------//
    TextMeshRenderer::TextMeshRenderer()
    {
        pMesh = MeshManager::getTextMesh();
    }


    int TextMeshRenderer::bindMaterial(TextMaterial *pMat)
    {
        if (!pMat)
        {
            SPACE_ENGINE_ERROR("Material is nullptr")
            return -1;
        }

        this->pMat = pMat;

        return 1;
    }

    TextMaterial *TextMeshRenderer::getMaterial()
    {
        return pMat;
    }
    
    TextMesh *TextMeshRenderer::getTextMesh()
    {
        return pMesh;
    }

    //----------------------------------------------//
    //------------------PlaneMesh-------------------//
    //----------------------------------------------//
    PlaneMesh::PlaneMesh()
    {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        glGenBuffers(2, buffers);
        populateBuffers();
    }
    void PlaneMesh::draw()
    {
        glDrawElements(GL_TRIANGLES,
                       N_INDICES_QUAD,
                       GL_UNSIGNED_INT,
                       0);
        GL_CHECK_ERRORS();
        // Make sure the VAO is not changed from the outside
        glBindVertexArray(0);
    }
    void PlaneMesh::bindVAO()
    {
        glBindVertexArray(VAO);
    }
    void PlaneMesh::populateBuffers()
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);

        glBufferData(GL_ARRAY_BUFFER, sizeof(plane), plane, GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(UIindices), UIindices, GL_STATIC_DRAW);

        size_t NumFloats = 0;

        glVertexAttribPointer(POSITION_LOCATION, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void *)(NumFloats * sizeof(float)));
        glEnableVertexAttribArray(POSITION_LOCATION);
        NumFloats += 2;

        glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void *)(NumFloats * sizeof(float)));
        glEnableVertexAttribArray(TEX_COORD_LOCATION);
    }
}