#pragma once

#include <gr-math/gmath.hpp>

#include <string>
#include <vector>

struct FbxNode
{
    std::vector<FbxNode> children;

    std::string name;

    Vector3 position = type_traits<Vector3>::zero;
    Quaternion rotation = type_traits<Quaternion>::identity;
    Vector3 scale = type_traits<Vector3>::one;

    uint32_t mesh = -1;

    template <typename ...Args>
    void addChild(Args &&...args);
};

template <typename... Args>
inline void FbxNode::addChild(Args &&...args)
{
    children.emplace_back(std::forward<Args>(args)...);
}
