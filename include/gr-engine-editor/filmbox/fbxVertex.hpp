#pragma once

#include <gr-math/gmath.hpp>

typedef struct FbxVertex
{
    Vector3 position;
    Vector3 normal;
    Vector2 texCoord;
    int index;

    float weight[4];
    int boneIndex[4];
} FbxVertex;


