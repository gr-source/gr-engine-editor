#include "system/cameraEditorSystem.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/clock.hpp"
#include "gr-base/core/input.hpp"

#include "gr-base/script/transform.hpp"

#include <algorithm>
#include <gr-base/core/scene.hpp>

CameraEditorSystem::CameraEditorSystem() : m_mouseSensitivity(0.1f), m_movementSpeed(30.0f), elapsed(Vector2::zero)
{}

void CameraEditorSystem::Update(Scene *scene)
{
    GameObject go(scene, scene->GetCamera());
    if (!go.IsAlive())
        return;

    auto transform = go.GetComponent<Transform>();

    Vector3 diretion = Vector3::zero;

    Vector3 forward = transform->rotation * Vector3::forward;
    Vector3 right = transform->rotation * Vector3::right;

    if (Input::IsKeyPress(KEY_W))
        diretion += forward;
    if (Input::IsKeyPress(KEY_S))
        diretion -= forward;

    if (Input::IsKeyPress(KEY_A))
        diretion += right;
    if (Input::IsKeyPress(KEY_D))
        diretion -= right;

    auto position = transform->position;
    if (Input::IsKeyPress(KEY_UP))
        diretion.y += 1.0f;
    if (Input::IsKeyPress(KEY_DOWN))
        diretion.y -= 1.0f;

    auto dt = Input::GetMouseScroll();
    if (Math::magnitudeSqrt(dt) > 0.0f)
        m_movementSpeed += dt.y;

    if (Math::magnitudeSqrt(diretion) > 0.0f)
        transform->position += (Math::normalize(diretion) * m_movementSpeed * (float)Clock::deltaTime);

    UpdateCursor(transform);

    scene->MarkChanged<Transform>(go.entity);
}

void CameraEditorSystem::UpdateCursor(Transform *transform)
{
    if (Input::IsMousePress(MOUSE_BUTTON_MIDDLE))
        ProcessMouse(Input::GetMouseDelta(), transform);
}

void CameraEditorSystem::ProcessMouse(const Vector2 &delta, Transform *transform)
{
    elapsed += delta * m_mouseSensitivity;

    elapsed.y = std::clamp(elapsed.y, -89.0f, 89.0f);
    elapsed.x = std::fmod(elapsed.x, 360.0f);

    auto rotation = Math::euler({Math::rad(elapsed.y), Math::rad(-elapsed.x), 0.0f});

    rotation = Math::slerp(transform->rotation, rotation, 12.0f * Clock::deltaTime);

    transform->rotation = Math::normalize(rotation);
}

