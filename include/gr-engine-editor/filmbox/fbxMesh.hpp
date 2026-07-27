#pragma once

#include "fbxMaterial.hpp"
#include "fbxVertex.hpp"
#include "fbxBone.hpp"

#include <vector>
#include <memory>

struct FbxSubmesh
{
    uint32_t indexCount;
    uint32_t indexOffset;
    uint32_t materialID;
};

struct FbxMesh
{
    std::vector<FbxVertex> vertices;

    std::vector<uint32_t> indices;

    std::vector<FbxSubmesh> submeshes;

    std::vector<FbxBone> bones;

    std::string name;

    template <typename ...Args>
    void addBone(Args &&...args);

    inline bool hasBone() const
    {
        return !bones.empty();
    }
};

template <typename... Args>
inline void FbxMesh::addBone(Args&&... args)
{
    bones.emplace_back(std::forward<Args>(args)...);
}
