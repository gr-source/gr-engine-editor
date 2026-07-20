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

    void load(const std::string &path);

    FbxScene *getScene() const;

    std::unique_ptr<FbxScene> get_scene();

private:
    std::unordered_map<std::string, FbxMesh> m_meshmap;

    std::unordered_map<std::string, Matrix4x4> m_boneMap;

    std::unordered_map<std::string, int> m_boneIndexMap;

    std::unique_ptr<FbxScene> m_scene;

    void loadSceneToCache(const aiScene *scene);

    FbxNode readNode(aiNode *mNode, const aiScene *scene);

    std::vector<FbxMaterial> loadMaterials(const aiScene *scene);

    FbxMaterial readMaterial(const aiMaterial* material);

    static void ProcessTexture(const aiMaterial *iMaterial, FbxMaterial& fbxMaterial);

    std::vector<FbxMesh> loadMeshes(const aiScene *scene);

    FbxMesh readMesh(const aiMesh *mMesh);

    // static void LoadAnimation(const aiScene *scene, FbxScene *fbxScene, std::unordered_map<std::string, int> &boneIndex);

    // static void AddKeys(aiNodeAnim *mNodeAnim, FbxChannel *channel);
};


