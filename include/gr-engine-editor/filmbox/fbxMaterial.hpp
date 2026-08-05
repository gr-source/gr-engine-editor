#pragma once

#include <gr-render/gRender.h>
#include <gr-math/gmath.hpp>

#include <string>
#include <vector>

typedef uint32_t FbxTextureID;

enum TextureType
{
    TextureType_Albedo = 0
};

struct FbxMaterial
{
    std::string name;

    Color albedo = COLOR_BLUE;

    float roughness = 0.5f;

    float metallic = 0.0f;

    float ao = 1.0f;

    std::unordered_map<TextureType, FbxTextureID> textures;
};

