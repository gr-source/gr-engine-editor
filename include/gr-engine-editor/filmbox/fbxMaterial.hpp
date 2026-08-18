#pragma once

// #include <gr-render/gRender.h>
#include <gr-render/color.hpp>

#include <gr-math/gmath.hpp>

#include <unordered_map>
#include <string>

typedef uint32_t FbxTextureID;

enum TextureType
{
    TextureType_Albedo = 0
};

struct FbxMaterial
{
    std::string name;

    Color albedo = Color::Blue();

    float roughness = 0.5f;

    float metallic = 0.0f;

    float ao = 1.0f;

    std::unordered_map<TextureType, FbxTextureID> textures;
};

