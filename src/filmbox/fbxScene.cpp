#include "filmbox/fbxScene.hpp"

FbxScene::FbxScene(const std::string & name) : m_name(name) {}

const std::vector<FbxAnimation> &FbxScene::getAnimations() const {
    return m_animation;
}

const FbxNode &FbxScene::getRootNode() const {
    return m_rootNode;
}

const FbxNode *FbxScene::findNode(const std::string &name) const {
    return readNode(m_rootNode, name);
}

const std::string &FbxScene::getName() const {
    return m_name;
}

const FbxNode *FbxScene::readNode(const FbxNode &node, const std::string &name) const {
    const FbxNode *found = nullptr;
    if (node.getName() == name) {
        found = &node;
        return found;
    }
    for (const auto &child : node.getChildren()) {
        found = readNode(child, name);
        if (found) {
            break;
        }
    }
    return found;
}
