#include "filmbox/filmbox.hpp"

#include "gr-base/core/console.hpp"

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <gr-math/matrix4x4.hpp>
#include <gr-math/quaternion.hpp>
#include <gr-math/vector3.hpp>

Filmbox::Filmbox()
{}

Filmbox::~Filmbox()
{}

bool Filmbox::load(const std::string &path, std::string &error, FbxScene* fbxScene)
{
    Assimp::Importer importer;

    bool result = false;

    error = "This fbxScene is nullptr";

    if (fbxScene != nullptr)
    {
        auto scene = importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_PopulateArmatureData | aiProcess_JoinIdenticalVertices);

        if (
                scene != nullptr &&
                (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != AI_SCENE_FLAGS_INCOMPLETE &&
                scene->mRootNode != nullptr
            )
        {
            *fbxScene =
                loadSceneToCache(scene);

            result = true;
        }
    }

    importer.FreeScene();

    return result;
}

FbxScene Filmbox::loadSceneToCache(const aiScene *scene)
{
    std::vector<FbxMaterial> materials;
    if (scene->HasMaterials())
    {
        Console::LogWarning("Loading material...");
        materials = loadMaterials(scene);
    }

    // std::vector<FbxMesh> meshes;
    // if (scene->HasMeshes())
    // {
        // Console::LogWarning("Loading Mesh...");
        // meshes = loadMeshes(scene);
    // }

    /*
    if (scene->HasAnimations())
    {
        Console::LogWarning("Loading Animations...");
        LoadAnimation(scene, fbxScene, boneIndex);
    }
    */

    Console::LogWarning("Loading Node...");

    auto mRootNode = scene->mRootNode;

    FbxNode root;
    root.name = mRootNode->mName.C_Str();

    for (std::uint32_t i=0;i<mRootNode->mNumChildren;++i)
        root.addChild(readNode(mRootNode->mChildren[i], scene));

    FbxScene fbxScene;
    fbxScene.materials = materials;
    fbxScene.meshes = m_meshes;
    fbxScene.root = root;
    fbxScene.name = scene->mName.C_Str();

    return fbxScene;
}

FbxNode Filmbox::readNode(aiNode *mNode, const aiScene *scene)
{
    auto name = mNode->mName;

    FbxNode node;
    node.name = name.C_Str();

    aiVector3D scale;
    aiVector3D position;
    aiQuaternion rotation;
    mNode->mTransformation.Decompose(scale, rotation, position);

    node.position =
        { position.x, position.y, position.z };
    node.rotation =
        { rotation.w, rotation.x, rotation.y, rotation.z };
    node.scale =
        { scale.x, scale.y, scale.z };
    
    if (m_boneMap.find(name.C_Str()) != m_boneMap.end())
    {
        Math::mat4_decompose(
                m_boneMap[name.C_Str()],
                node.scale,
                node.rotation,
                node.position
            );
    }

    node.mesh = loadMeshByNode(mNode, scene);

    for (auto i=0;i<mNode->mNumChildren;++i)
        node.addChild(readNode(mNode->mChildren[i], scene));

    return node;
}

uint32_t Filmbox::loadMeshByNode(aiNode* mNode, const aiScene* scene)
{
    std::vector<FbxVertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<FbxSubmesh> submeshes;

    for (uint32_t meshIndex = 0; meshIndex < mNode->mNumMeshes; ++meshIndex)
    {
        aiMesh* mesh = scene->mMeshes[mNode->mMeshes[meshIndex]];

        bool hasPosition = mesh->HasPositions();
        bool hasNormal   = mesh->HasNormals();
        bool hasTexCoord = mesh->HasTextureCoords(0);

        uint32_t vertexOffset = static_cast<uint32_t>(vertices.size());
        uint32_t indexOffset  = static_cast<uint32_t>(indices.size());

        // Vértices
        for (uint32_t v = 0; v < mesh->mNumVertices; ++v)
        {
            auto& vertex = vertices.emplace_back();

            if (hasPosition)
            {
                const auto& p = mesh->mVertices[v];
                vertex.position = { p.x, p.y, p.z };
            }

            if (hasNormal)
            {
                const auto& n = mesh->mNormals[v];
                vertex.normal = { n.x, n.y, n.z };
            }

            if (hasTexCoord)
            {
                const auto& uv = mesh->mTextureCoords[0][v];
                vertex.texCoord = { uv.x, uv.y };
            }
        }

        // Índices
        uint32_t submeshIndexCount = 0;

        for (uint32_t f = 0; f < mesh->mNumFaces; ++f)
        {
            const aiFace& face = mesh->mFaces[f];
            assert(face.mNumIndices <= 3);

            for (uint32_t k = 0; k < face.mNumIndices; ++k)
            {
                indices.push_back(face.mIndices[k] + vertexOffset);
                ++submeshIndexCount;
            }
        }

        auto& submesh = submeshes.emplace_back();
        submesh.indexOffset = indexOffset;
        submesh.indexCount  = submeshIndexCount;
        submesh.materialID  = mesh->mMaterialIndex;
    }

    FbxMesh mesh;
    mesh.name = mNode->mName.C_Str();
    mesh.vertices = std::move(vertices);
    mesh.indices = std::move(indices);
    mesh.submeshes = std::move(submeshes);

    m_meshes.push_back(std::move(mesh));

    return static_cast<uint32_t>(m_meshes.size() - 1);
}

std::vector<FbxMaterial> Filmbox::loadMaterials(const aiScene* scene)
{
    std::vector<FbxMaterial> materials(scene->mNumMaterials);
    for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
        materials[i] = readMaterial(scene->mMaterials[i]);
    return materials;
}

FbxMaterial Filmbox::readMaterial(const aiMaterial* material)
{
    FbxMaterial fbxMaterial{};

    aiString name;
    if (material->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
        fbxMaterial.name = name.C_Str();

    aiColor4D color;
    if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
        fbxMaterial.albedo = { color.r, color.g, color.b, color.a };

    float metallic = 0.0f;
    if (material->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS)
        fbxMaterial.metallic = metallic;

    float roughness = 0.5f;
    if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS)
        fbxMaterial.roughness = roughness;

    /*
    material->Get(
        "$mat.gltf.pbrMetallicRoughness.metallicFactor",
        0,
        0,
        fbxMaterial.metallic);

    material->Get(
        "$mat.gltf.pbrMetallicRoughness.roughnessFactor",
        0,
        0,
        fbxMaterial.roughness);
    */

    float ao = 1.0f;
    if (material->Get(AI_MATKEY_USE_AO_MAP, ao) == AI_SUCCESS)
        fbxMaterial.ao = ao;

    return fbxMaterial;
}

void Filmbox::ProcessTexture(const aiMaterial *iMaterial, FbxMaterial &fbxMaterial) {
    auto count = iMaterial->GetTextureCount(aiTextureType_DIFFUSE);
    for (auto i=0;i<count;++i) {
        aiString path;
        if (iMaterial->GetTexture(aiTextureType_DIFFUSE, i, &path) != AI_SUCCESS) {
            continue;
        }

        fbxMaterial.addTexture(TextureType_Albedo, path.C_Str());
        break;
    }
}

/*
std::vector<FbxMesh> Filmbox::loadMeshes(const aiScene *scene)
{
    std::vector<FbxMesh> meshes(scene->mNumMeshes);
    for (uint32_t i=0;i<scene->mNumMeshes;++i)
        meshes[i] = readMesh(scene->mMeshes[i]);
    return meshes;
}

FbxMesh Filmbox::readMesh(const aiMesh *mMesh)
{
    FbxMesh fbxMesh;
    fbxMesh.materialID = mMesh->mMaterialIndex;
    fbxMesh.name = std::string(mMesh->mName.C_Str()) + "_" + std::to_string(mMesh->mMaterialIndex);

    bool bHasPosition = mMesh->HasPositions();
    bool bHasNormal = mMesh->HasNormals();
    bool bHasTexCoord = mMesh->HasTextureCoords(0);

    auto& vertices = fbxMesh.vertices;
    vertices.resize(mMesh->mNumVertices, {});

    for (uint32_t i=0;i<mMesh->mNumVertices;++i)
    {
        auto& vertex = vertices[i];

        if (bHasPosition)
        {
            auto &mVertice = mMesh->mVertices[i];

            vertex.position = {mVertice.x, mVertice.y, mVertice.z};
        }

        if (bHasNormal)
        {
            auto &mNormal = mMesh->mNormals[i];

            vertex.normal = {mNormal.x, mNormal.y, mNormal.z};
        }

        if (bHasTexCoord)
        {
            auto &mTexCoord = mMesh->mTextureCoords[0][i];

            vertex.texCoord = {mTexCoord.x, mTexCoord.y};
        }
    }

    auto& indices = fbxMesh.indices;
    indices.resize(mMesh->mNumFaces * 3, 0);

    for (uint32_t i = 0; i < mMesh->mNumFaces; ++i)
    {
        const aiFace& face = mMesh->mFaces[i];
        assert(face.mNumIndices <= 3);

        for (int j=0;j<face.mNumIndices;j++)
            indices[i * 3 + j] = face.mIndices[j];
    }

    if (mMesh->HasBones())
    {
        for (auto i=0;i<mMesh->mNumBones;++i)
        {
            auto mBone = mMesh->mBones[i];

            auto name = mBone->mName;

            if (m_boneIndexMap.find(name.C_Str()) == m_boneIndexMap.end())
                m_boneIndexMap.emplace(name.C_Str(), i);

            aiVector3D scale;
            aiQuaternion rotation;
            aiVector3D position;
            mBone->mOffsetMatrix.Decompose(scale, rotation, position);

            int parentIndex = 0;
            if (mBone->mNode->mParent != nullptr)
            {
                auto it = m_boneIndexMap.find(mBone->mNode->mParent->mName.C_Str());
                if (it != m_boneIndexMap.end())
                    parentIndex = it->second;
            }

            FbxBone bone;
            bone.id = i;
            bone.scale = { scale.x, scale.y, scale.z };
            bone.position = { position.x, position.y, position.z };
            bone.rotation = { rotation.w, rotation.x, rotation.y, rotation.z };
            bone.parentID = parentIndex;

            fbxMesh.addBone(bone);

            if (m_boneMap.find(name.C_Str()) == m_boneMap.end())
                m_boneMap.emplace(name.C_Str(), bone.getLocalMatrix());

            for (auto j=0;j<mBone->mNumWeights;++j) {
                auto weight = mBone->mWeights[j];

                auto& vertex = vertices[weight.mVertexId];
                for (auto k=0;k<4;++k) {
                    if (vertex.boneIndex[k] != -1 || weight.mWeight <= 0.0f)
                        continue;

                    vertex.weight[k] = static_cast<float>(weight.mWeight);
                    vertex.boneIndex[k] = i;
                    break;
                }
            }
        }
    }

    return fbxMesh;
}
*/

/*
void Filmbox::LoadAnimation(const aiScene *scene, FbxScene *fbxScene, std::unordered_map<std::string, int> &boneIndex)
{
    for (auto i=0;i<scene->mNumAnimations;i++) {
        auto mAnimation = scene->mAnimations[i];

        auto name = mAnimation->mName;
        auto duration = mAnimation->mDuration;
        auto tick = mAnimation->mTicksPerSecond;

        auto &animation = fbxScene->addAnimation(name.C_Str(), duration, tick);
        animation.setChannel(std::vector<FbxChannel>(boneIndex.size(), {"None"}));

        for (auto j=0;j<mAnimation->mNumChannels;j++) {
            auto mChannel = mAnimation->mChannels[j];

            auto cName = mChannel->mNodeName;

            auto it = boneIndex.find(cName.C_Str());
            if (it == boneIndex.end())
            {
                continue;
            }

            auto channel = animation.getChannel(it->second);
            channel->setName(cName.C_Str());

            AddKeys(mChannel, channel);
        }
    }
}

void Filmbox::AddKeys(aiNodeAnim *mNodeAnim, FbxChannel *channel) {
    for (auto i=0;i<mNodeAnim->mNumPositionKeys;++i) {
        auto key = mNodeAnim->mPositionKeys[i];
        channel->addKeyPosition(static_cast<float>(key.mTime), Vector3{key.mValue.x, key.mValue.y, key.mValue.z});
    }

    for (auto i=0;i<mNodeAnim->mNumScalingKeys;++i) {
        auto key = mNodeAnim->mScalingKeys[i];
        channel->addKeyScale(static_cast<float>(key.mTime), Vector3{key.mValue.x, key.mValue.y, key.mValue.z});
    }

    for (auto i=0;i<mNodeAnim->mNumRotationKeys;++i) {
        auto key = mNodeAnim->mRotationKeys[i];
        channel->addKeyRotation(static_cast<float>(key.mTime), Quaternion{key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z});
    }
}
*/



