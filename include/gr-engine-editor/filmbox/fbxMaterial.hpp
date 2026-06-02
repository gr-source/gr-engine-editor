#pragma once

#include <gr-render/gRender.h>
#include <gr-math/gmath.hpp>

#include <string>
#include <vector>

enum TextureType {
    TextureType_Albedo
};

class FbxMaterial
{
public:
    FbxMaterial() = default;

    std::string name;

    Color albedo;

    float roughness;

    float metallic;

    float ao;

    template <typename ...Args>
    void addTexture(Args &&...args);

    const std::unordered_map<TextureType, const std::string> &getTextures() const;

private:

    std::unordered_map<TextureType, const std::string> m_textures;
};

template <typename... Args>
inline void FbxMaterial::addTexture(Args &&...args) {
    m_textures.emplace(std::forward<Args>(args)...);
}
