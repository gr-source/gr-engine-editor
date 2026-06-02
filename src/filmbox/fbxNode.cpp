#include "filmbox/fbxNode.hpp"

FbxNode::FbxNode(const std::string &name) : m_name(name)
{}

std::string FbxNode::getName() const {
    return m_name;
}

void FbxNode::AddMesh(uint32_t id)
{
    m_mesheIDs.push_back(id);
}

std::vector<uint32_t> FbxNode::GetMesheIDs() const
{
    return m_mesheIDs;
}

bool FbxNode::HasMeshe() const
{
    return !m_mesheIDs.empty();
}

const std::vector<FbxNode> &FbxNode::getChildren() const {
    return m_children;
}

void FbxNode::setPosition(const Vector3 &position) {
    m_position = position;
}

const Vector3 &FbxNode::getPosition() const {
    return m_position;
}

void FbxNode::setScale(const Vector3 &scale) {
    m_scale = scale;
}

const Vector3 &FbxNode::getScale() const {
    return m_scale;
}

void FbxNode::setRotation(const Quaternion &rotation) {
    m_rotation = rotation;
}

const Quaternion &FbxNode::getRotation() const {
    return m_rotation;
}
