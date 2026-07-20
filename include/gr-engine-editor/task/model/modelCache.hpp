#pragma once

#include "gr-base/asset/materialasset.hpp"
#include "gr-base/asset/meshasset.hpp"

#include <gr-math/types.hpp>

#include <unordered_map>
#include <string>
#include <vector>

struct ModelNode
{
    std::string name;

    Vector3 position;
    Quaternion rotation;
    Vector3 scale;

    std::vector<ModelNode> children;
};

struct ModelPack
{
    std::unordered_map<std::string, MaterialAsset> materialsAsset;
    std::unordered_map<std::string, MeshAsset> meshesAsset;

    ModelNode root;
};

