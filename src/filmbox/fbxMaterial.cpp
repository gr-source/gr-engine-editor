#include "filmbox/fbxMaterial.hpp"

const std::unordered_map<TextureType, const std::string> &FbxMaterial::getTextures() const {
    return m_textures;
}
