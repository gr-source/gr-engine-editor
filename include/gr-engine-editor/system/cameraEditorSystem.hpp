#pragma once

#include <gr-base/core/system.hpp>

#include <gr-base/script/transform.hpp>

#include <gr-base/ecs/default.hpp>

class CameraEditorSystem : public ISystem
{
public:
    CameraEditorSystem();

public:
    float m_mouseSensitivity;

    float m_movementSpeed;

    Vector2 elapsed;

    void Update() override;

    void UpdateCursor(Transform* transform);

    void ProcessMouse(const Vector2 &delta, Transform *transform);
};

template <>
inline constexpr TypeCategory engine::type_traits::category<CameraEditorSystem> =
    TypeCategory::System;


