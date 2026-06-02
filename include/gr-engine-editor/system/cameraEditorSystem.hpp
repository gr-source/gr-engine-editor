#pragma once

#include "gr-base/core/system.hpp"

#include "gr-base/script/transform.hpp"
#include <gr-base/core/default.hpp>

class CameraEditorSystem : public System
{
public:
    CameraEditorSystem();

public:
    float mouseSensitivity;

    Vector2 elapsed;

    void Update(Scene* scene) override;

    void UpdateCursor(Transform* transform);

    void ProcessMouse(const Vector2 &delta, Transform *transform);
};



