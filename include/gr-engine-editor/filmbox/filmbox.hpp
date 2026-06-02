#pragma once

#include "fbxScene.hpp"

#include <assimp/cimport.h>

#include <assimp/scene.h>

#include <functional>
#include <memory>

class Filmbox
{
public:
    Filmbox();
    ~Filmbox();

    void load(const std::string &path);

    FbxScene *getScene() const;

    std::unique_ptr<FbxScene> get_scene();

private:
    std::unique_ptr<FbxScene> m_scene;

    void loadSceneToCache(const aiScene *scene, FbxScene *fbxScene);

    static void ProcessNode(aiNode *mNode, FbxNode& node, const aiScene *scene, FbxScene *fbxScene, std::unordered_map<std::string, Matrix4x4> &boneMap);

    void loadMaterial(const aiScene *scene, FbxScene *fbxScene);

    static void ProcessTexture(const aiMaterial *iMaterial, FbxMaterial& fbxMaterial);

    void loadMesh(const aiScene *scene, FbxScene *fbxScene, std::unordered_map<std::string, Matrix4x4> &boneMap, std::unordered_map<std::string, int> &boneIndex);

    void setupMesh(const aiMesh *mMesh, FbxMesh* fbxMesh, std::unordered_map<std::string, Matrix4x4> &boneMap, std::unordered_map<std::string, int> &boneIndex);

    static void LoadAnimation(const aiScene *scene, FbxScene *fbxScene, std::unordered_map<std::string, int> &boneIndex);

    static void AddKeys(aiNodeAnim *mNodeAnim, FbxChannel *channel);
};


