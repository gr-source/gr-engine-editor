#pragma once

#include <string>
#include <vector>

#include "fbxMesh.hpp"

class FbxNode {
public:
    FbxNode(const std::string &name = "default");

    std::string getName() const;

    void AddMesh(uint32_t id);

    std::vector<uint32_t> GetMesheIDs() const;

    bool HasMeshe() const;

    template <typename ...Args>
    FbxNode &addNode(Args &&...args);

    const std::vector<FbxNode> &getChildren() const;

    void setPosition(const Vector3 &position);

    const Vector3 &getPosition() const;

    void setScale(const Vector3 &scale);

    const Vector3 &getScale() const;

    void setRotation(const Quaternion &rotation);

    const Quaternion &getRotation() const;

private:
    std::string m_name;

    std::vector<FbxNode> m_children;

    std::vector<uint32_t> m_mesheIDs;

    Vector3 m_position;

    Vector3 m_scale;
    
    Quaternion m_rotation;
};

template <typename... Args>
inline FbxNode &FbxNode::addNode(Args &&...args) {
    return m_children.emplace_back(std::forward<Args>(args)...);
}
