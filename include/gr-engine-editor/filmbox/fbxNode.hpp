#pragma once

#include <gr-math/gmath.hpp>

#include <string>
#include <vector>

struct FbxNode
{
    std::string name;

    Vector3 position = type_traits<Vector3>::zero;
    Quaternion rotation = type_traits<Quaternion>::identity;
    Vector3 scale = type_traits<Vector3>::one;

    std::vector<FbxNode> children;
    std::vector<uint32_t> meshes;

    template <typename ...Args>
    void addChild(Args &&...args);
};

template <typename... Args>
inline void FbxNode::addChild(Args &&...args)
{
    children.emplace_back(std::forward<Args>(args)...);
}
