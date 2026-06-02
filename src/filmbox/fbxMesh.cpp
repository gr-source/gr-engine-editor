#include "filmbox/fbxMesh.hpp"

#include <algorithm>

FbxBone *FbxMesh::findBone(const std::string &name) {
    auto it = std::find_if(m_bones.begin(), m_bones.end(), [name](const auto &bone) {
        return bone.getName() == name;
    });
    return it != m_bones.end() ? &(*it) : nullptr;
}

std::vector<FbxBone> FbxMesh::getBones() const {
    return m_bones;
}

bool FbxMesh::hasBone() const {
    return !m_bones.empty();
}
