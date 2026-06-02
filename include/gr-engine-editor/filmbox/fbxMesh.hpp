#pragma once

#include "fbxMaterial.hpp"
#include "fbxVertex.hpp"
#include "fbxBone.hpp"

#include <vector>
#include <memory>

class FbxMesh
{
public:
    FbxMesh() = default;

    inline void SetName(const std::string& name)
    {
        m_name = name;
    }

    inline const std::string &GetName() const
    {
        return m_name;
    }
    
    inline uint32_t GetMaterialID() const
    {
        return m_materialID;
    }

    inline void SetMaterialID(uint32_t id)
    {
        m_materialID = id;
    }

    template <typename ...Args>
    FbxBone &addBone(Args &&...args);

    FbxBone *findBone(const std::string &name);

    std::vector<FbxBone> getBones() const;

    bool hasBone() const;

    std::vector<FbxVertex> vertices;

    std::vector<uint32_t> indices;
    
private:
    uint32_t m_materialID;

    std::string m_name;

    std::vector<FbxBone> m_bones;
};


template <typename... Args>
inline FbxBone &FbxMesh::addBone(Args &&...args) {
    return m_bones.emplace_back(std::forward<Args>(args)...);
}
