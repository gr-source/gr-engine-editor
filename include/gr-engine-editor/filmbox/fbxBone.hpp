#pragma once

#include <gr-math/gmath.hpp>

#include <string>

struct FbxBone
{
    std::string name;

    Vector3 position;

    Vector3 scale;
    
    Quaternion rotation;

    int parentID;

    int id;

    Matrix4x4 getLocalMatrix() const;
};





