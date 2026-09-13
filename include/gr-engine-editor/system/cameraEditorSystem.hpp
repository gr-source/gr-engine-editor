#pragma once

#include <gr-base/core/system.hpp>

#include <gr-base/script/transform.hpp>

#include <gr-base/ecs/default.hpp>

class CameraEditorSystem : public ISystemImpl<Read<>, Write<TransformLocal>>
{
public:
    CameraEditorSystem();

public:
    float sensitivity = 0.1f;

    float m_movementSpeed = 30.0f;

    float m_velocity = 0.0f;

    float yaw = 0.0f;
    float pitch = 0.0f;

    void Update() override;

    void updateMouse(const Vector2 &delta, TransformLocal *transform);
};

namespace engine
{
template <>
struct category_traits<CameraEditorSystem>
{
    static constexpr TypeCategory value =
        TypeCategory::System;
};
}


