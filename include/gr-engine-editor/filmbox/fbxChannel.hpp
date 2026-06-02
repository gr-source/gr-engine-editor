#pragma once

#include <gr-math/gmath.hpp>

#include <string>
#include <vector>

template <typename T>
struct FbxKey {
    float time;

    T value;

    FbxKey(float time, T value);
};

template <typename T>
inline FbxKey<T>::FbxKey(float time, T value) : time(time), value(value) {}

class FbxChannel {
public:
    FbxChannel(const std::string &name);

    const std::string &getName() const;

    void setName(const std::string &name);

    template <typename ...Args>
    FbxKey<Vector3> &addKeyPosition(Args &&... args);

    const std::vector<FbxKey<Vector3>> getKeysPosition() const;

    template <typename ...Args>
    FbxKey<Vector3> &addKeyScale(Args &&... args);

    const std::vector<FbxKey<Vector3>> getKeysScale() const;

    template <typename ...Args>
    FbxKey<Quaternion> &addKeyRotation(Args &&... args);

    const std::vector<FbxKey<Quaternion>> getKeysRotation() const;

private:
    std::string m_name;

    std::vector<FbxKey<Vector3>> m_position;

    std::vector<FbxKey<Vector3>> m_scale;

    std::vector<FbxKey<Quaternion>> m_rotation;
};

template <typename... Args>
inline FbxKey<Vector3> &FbxChannel::addKeyPosition(Args &&...args) {
    return m_position.emplace_back(std::forward<Args>(args)...);
}

template <typename... Args>
inline FbxKey<Vector3> &FbxChannel::addKeyScale(Args &&...args) {
    return m_scale.emplace_back(std::forward<Args>(args)...);
}

template <typename... Args>
inline FbxKey<Quaternion> &FbxChannel::addKeyRotation(Args &&...args) {
    return m_rotation.emplace_back(std::forward<Args>(args)...);
}
