#pragma once

#include "fbxAnimation.hpp"
#include "fbxMaterial.hpp"
#include "fbxMesh.hpp"
#include "fbxNode.hpp"

struct FbxScene
{
    std::vector<FbxAnimation> animations;

    std::vector<std::string> textures;

    std::vector<FbxMaterial> materials;

    std::vector<FbxMesh> meshes;

    std::string name;

    FbxNode root;
};



