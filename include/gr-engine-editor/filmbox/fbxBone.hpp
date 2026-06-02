#pragma once

#include <gr-math/gmath.hpp>

#include <string>

class FbxBone {
public:
    FbxBone(const std::string &name, int id);

    std::string getName() const;

    void setPosition(const Vector3 &position);

    Vector3 getPosition() const;

    void setScale(const Vector3 &scale);

    Vector3 getScale() const;

    void setRotation(const Quaternion &rotation);

    Quaternion getRotation() const;

    int getID() const;

    Matrix4x4 getLocalMatrix() const;

    int getParentIndex() const;

    void setParentIndex(int index);

private:
    std::string m_name;

    Vector3 m_position;

    Vector3 m_scale;
    
    Quaternion m_rotation;

    int m_id;

    int m_parentIndex;
};





