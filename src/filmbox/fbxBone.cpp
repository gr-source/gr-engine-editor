#include "filmbox/fbxBone.hpp"

#include <gr-math/gmath.hpp>
#include <gr-math/matrix4x4.hpp>

Matrix4x4 FbxBone::getLocalMatrix() const
{
    return Math::CreateTRS(position, rotation, scale);
}

