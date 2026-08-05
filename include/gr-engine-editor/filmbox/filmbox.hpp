#pragma once

#include "fbxScene.hpp"

#include <assimp/cimport.h>

#include <assimp/scene.h>

#include <functional>
#include <memory>
#include <unordered_map>

class Filmbox
{
public:
    Filmbox();
    ~Filmbox();

    bool load(const std::string &path, std::string& error, FbxScene* fbxScene);

private:
    std::unordered_map<std::string, FbxMesh> m_meshmap;

    std::unordered_map<std::string, Matrix4x4> m_boneMap;

    std::unordered_map<std::string, int> m_boneIndexMap;

    std::vector<std::string> m_textures;
    std::vector<FbxMesh> m_meshes;

    FbxScene loadSceneToCache(const aiScene *scene);

    FbxNode readNode(aiNode *mNode, const aiScene *scene);

    uint32_t loadMeshByNode(aiNode *mNode, const aiScene *scene);

    std::vector<std::string> loadTextures(const aiScene *scene);

    std::vector<FbxMaterial> loadMaterials(const aiScene *scene);

    FbxMaterial readMaterial(const aiMaterial* material);

    // std::vector<FbxMesh> loadMeshes(const aiScene *scene);

    // FbxMesh readMesh(const aiMesh *mMesh);

    // static void LoadAnimation(const aiScene *scene, FbxScene *fbxScene, std::unordered_map<std::string, int> &boneIndex);

    // static void AddKeys(aiNodeAnim *mNodeAnim, FbxChannel *channel);
};


