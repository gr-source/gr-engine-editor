#include "filmbox/filmbox.hpp"

#include "gr-base/core/console.hpp"

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <gr-math/matrix4x4.hpp>
#include <gr-math/quaternion.hpp>
#include <gr-math/vector3.hpp>

#define NORMALIZE(v) if (std::abs(v.x) > 6.0f) v.x /= 1e2f; if (std::abs(v.y) > 6.0f) v.y /= 1e2f; if (std::abs(v.z) > 6.0f) v.z /= 1e2f;

#define NORM(n) n > 6.0f ? n / 1e2f : n

Filmbox::Filmbox() : m_scene(nullptr)
{
}

Filmbox::~Filmbox()
{}

void Filmbox::load(const std::string &path)
{
    Assimp::Importer importer;

    auto scene = importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_PopulateArmatureData | aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        throw std::runtime_error("fail to load scene.");
    }

    m_scene = std::make_unique<FbxScene>(scene->mName.C_Str());

    loadSceneToCache(scene, m_scene.get());

    importer.FreeScene();
}

FbxScene *Filmbox::getScene() const
{
    return m_scene.get();
}

std::unique_ptr<FbxScene> Filmbox::get_scene()
{
    return std::move(m_scene);
}

void Filmbox::loadSceneToCache(const aiScene *scene, FbxScene *fbxScene)
{
    if (scene->HasMaterials())
    {
        Console::LogWarning("Loading material...");
        loadMaterial(scene, fbxScene);
    }

    std::unordered_map<std::string, Matrix4x4> boneMap;
    std::unordered_map<std::string, int> boneIndex;

    if (scene->HasMeshes())
    {
        Console::LogWarning("Loading Mesh...");
        loadMesh(scene, fbxScene, boneMap, boneIndex);
    }

    if (scene->HasAnimations())
    {
        Console::LogWarning("Loading Animations...");
        LoadAnimation(scene, fbxScene, boneIndex);
    }

    Console::LogWarning("Loading Node...");

    auto mRootNode = scene->mRootNode;
    auto& fRootNode = fbxScene->m_rootNode;

    for (std::uint32_t i=0;i<mRootNode->mNumChildren;++i)
    {
        ProcessNode(mRootNode->mChildren[i], fRootNode, scene, fbxScene, boneMap);
    }
}

void Filmbox::ProcessNode(aiNode *mNode, FbxNode& root, const aiScene *scene, FbxScene *fbxScene, std::unordered_map<std::string, Matrix4x4> &boneMap) {
    auto name = mNode->mName;

    auto& node = root.addNode(name.C_Str());

    aiVector3D scale;
    aiVector3D position;
    aiQuaternion rotation;
    mNode->mTransformation.Decompose(scale, rotation, position);

    node.setPosition({position.x, position.y, position.z});
    node.setScale({scale.x, scale.y, scale.z});
    node.setRotation({rotation.w, rotation.x, rotation.y, rotation.z});
    
    if (boneMap.find(name.C_Str()) != boneMap.end())
    {
        Vector3 t;
        Quaternion r;
        Vector3 s;

        boneMap[name.C_Str()].decompose(s, r, t);
        
        node.setPosition(t);
        node.setRotation(r);
        node.setScale(s);
    }

    for (auto i=0;i<mNode->mNumMeshes;++i)
        node.AddMesh(mNode->mMeshes[i]);

    for (auto i=0;i<mNode->mNumChildren;++i)
    {
        auto child = mNode->mChildren[i];

        ProcessNode(child, node, scene, fbxScene, boneMap);
    }
}

void Filmbox::loadMaterial(const aiScene* scene, FbxScene* fbxScene)
{
    auto& outMaterials = fbxScene->materials;
    outMaterials.resize(scene->mNumMaterials);

    for (uint32_t i = 0; i < scene->mNumMaterials; ++i) 
    {
        const aiMaterial* aiMat = scene->mMaterials[i];
        auto& targetMat = outMaterials[i];

        aiString matName;
        if (aiMat->Get(AI_MATKEY_NAME, matName) == AI_SUCCESS)
            targetMat.name = matName.C_Str();

        ProcessTexture(aiMat, targetMat);

        targetMat.albedo = { 1.0f, 1.0f, 1.0f, 1.0f };
        targetMat.metallic = 0.0f;
        targetMat.roughness = 0.5f;
        targetMat.ao = 1.0f;
        
        aiColor4D color;
        if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
            targetMat.albedo = { color.r, color.g, color.b, color.a };

        float factor = 0.0f;
        
        if (aiMat->Get(AI_MATKEY_METALLIC_FACTOR, factor) == AI_SUCCESS || 
            aiMat->Get("$mat.gltf.pbrMetallicRoughness.metallicFactor", 0, 0, factor) == AI_SUCCESS)
            targetMat.metallic = factor;

        if (aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, factor) == AI_SUCCESS || 
            aiMat->Get("$mat.gltf.pbrMetallicRoughness.roughnessFactor", 0, 0, factor) == AI_SUCCESS)
            targetMat.roughness = factor;

        if (aiMat->Get(AI_MATKEY_USE_AO_MAP, factor) == AI_SUCCESS)
            targetMat.ao = factor;
    }
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

void Filmbox::loadMesh(const aiScene *scene, FbxScene *fbxScene, std::unordered_map<std::string, Matrix4x4> &boneMap, std::unordered_map<std::string, int> &boneIndex)
{
    auto& meshes = fbxScene->meshes;
    meshes.resize(scene->mNumMeshes, {});

    for (uint32_t i=0;i<scene->mNumMeshes;++i)
    {
        FbxMesh& fbxMesh = meshes[i];

        setupMesh(scene->mMeshes[i], &fbxMesh, boneMap, boneIndex);
    }
}

void Filmbox::setupMesh(const aiMesh *mMesh, FbxMesh* fbxMesh, std::unordered_map<std::string, Matrix4x4> &boneMap, std::unordered_map<std::string, int> &boneIndex)
{
    fbxMesh->SetMaterialID(mMesh->mMaterialIndex);
    fbxMesh->SetName(mMesh->mName.C_Str());

    bool bHasPosition = mMesh->HasPositions();
    bool bHasNormal = mMesh->HasNormals();
    bool bHasTexCoord = mMesh->HasTextureCoords(0);

    auto& vertices = fbxMesh->vertices;
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

    auto& indices = fbxMesh->indices;
    indices.resize(mMesh->mNumFaces * 3, 0);

    for (uint32_t i = 0; i < mMesh->mNumFaces; ++i)
    {
        const aiFace& face = mMesh->mFaces[i];
        assert(face.mNumIndices <= 3);

        for (int j=0;j<face.mNumIndices;j++)
            indices[i * 3 + j] = face.mIndices[j];
    }

    if (mMesh->HasBones()) {
        for (auto i=0;i<mMesh->mNumBones;++i) {
            auto mBone = mMesh->mBones[i];

            auto name = mBone->mName;

            if (boneIndex.find(name.C_Str()) == boneIndex.end())
            {
                boneIndex.emplace(name.C_Str(), i);
            }

            aiVector3D scale;
            aiQuaternion rotation;
            aiVector3D position;
            mBone->mOffsetMatrix.Decompose(scale, rotation, position);

            auto& fBone = fbxMesh->addBone(name.C_Str(), i);
            fBone.setScale({scale.x, scale.y, scale.z});
            fBone.setPosition({position.x, position.y, position.z});
            fBone.setRotation({rotation.w, rotation.x, rotation.y, rotation.z});

            int parentIndex = 0;
            if (mBone->mNode->mParent != nullptr)
            {
                auto it = boneIndex.find(mBone->mNode->mParent->mName.C_Str());
                if (it != boneIndex.end())
                    parentIndex = it->second;
            }

            fBone.setParentIndex(parentIndex);

            if (boneMap.find(name.C_Str()) == boneMap.end())
            {
                boneMap.emplace(name.C_Str(), fBone.getLocalMatrix());
            }

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
}

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




