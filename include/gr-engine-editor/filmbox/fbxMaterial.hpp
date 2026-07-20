#pragma once

#include <gr-render/gRender.h>
#include <gr-math/gmath.hpp>

#include <string>
#include <vector>

enum TextureType {
    TextureType_Albedo
};

struct FbxMaterial
{
    std::string name;

    Color albedo = COLOR_BLUE;

    float roughness = 0.0f;

    float metallic = 0.0f;

    float ao = 0.0f;

    std::unordered_map<TextureType, const std::string> textures;

    template <typename ...Args>
    void addTexture(Args &&...args);
};

template <typename... Args>
inline void FbxMaterial::addTexture(Args &&...args)
{
    textures.emplace(std::forward<Args>(args)...);
}
