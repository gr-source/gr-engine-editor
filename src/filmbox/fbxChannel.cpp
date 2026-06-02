#include "filmbox/fbxChannel.hpp"

FbxChannel::FbxChannel(const std::string &name) : m_name(name) {}

const std::string &FbxChannel::getName() const {
    return m_name;
}

void FbxChannel::setName(const std::string &name)
{
    m_name = name;
}

const std::vector<FbxKey<Vector3>> FbxChannel::getKeysPosition() const {
    return m_position;
}

const std::vector<FbxKey<Vector3>> FbxChannel::getKeysScale() const {
    return m_scale;
}

const std::vector<FbxKey<Quaternion>> FbxChannel::getKeysRotation() const {
    return m_rotation;
}
