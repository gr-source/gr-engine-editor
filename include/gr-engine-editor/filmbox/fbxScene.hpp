#pragma once

#include "fbxAnimation.hpp"
#include "fbxMaterial.hpp"
#include "fbxMesh.hpp"
#include "fbxNode.hpp"

class FbxScene {
public:
    FbxScene(const std::string &name = "default");

    template <typename ...Args>
    FbxAnimation &addAnimation(Args &&...args);

    const std::vector<FbxAnimation> &getAnimations() const;

    const FbxNode& getRootNode() const;

    const FbxNode *findNode(const std::string &name) const;

    const std::string &getName() const;

    std::vector<FbxMaterial> materials;

    std::vector<FbxMesh> meshes;
    
private:
    std::string m_name;

    std::vector<FbxAnimation> m_animation;

    FbxNode m_rootNode;

    const FbxNode *readNode(const FbxNode& node, const std::string &name) const;

    friend class Filmbox;
};

template <typename... Args>
inline FbxAnimation &FbxScene::addAnimation(Args &&...args) {
    return m_animation.emplace_back(std::forward<Args>(args)...);
}


