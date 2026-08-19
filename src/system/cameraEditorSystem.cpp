#include "system/cameraEditorSystem.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/clock.hpp"
#include "gr-base/core/input.hpp"

#include "gr-base/script/transform.hpp"

#include <algorithm>
#include <gr-base/core/scene.hpp>

CameraEditorSystem::CameraEditorSystem()
{}

void CameraEditorSystem::Update()
{
    GameObject go(scene, scene->GetMainCamera());
    if (!go.IsAlive())
        return;

    auto& transform = go.GetComponent<TransformLocal>();

    Vector3 diretion = type_traits<Vector3>::zero;

    Vector3 forward = Math::normalize(transform.rotation * type_traits<Vector3>::forward);
    Vector3 right = Math::normalize(transform.rotation * type_traits<Vector3>::right);
    Vector3 up = Math::normalize(transform.rotation * type_traits<Vector3>::up);

    if (Input::IsKeyPress(KEY_W))
        diretion += forward;
    if (Input::IsKeyPress(KEY_S))
        diretion -= forward;

    if (Input::IsKeyPress(KEY_D))
        diretion += right;
    if (Input::IsKeyPress(KEY_A))
        diretion -= right;

    if (Input::IsKeyPress(KEY_E))
        diretion += up;
    if (Input::IsKeyPress(KEY_Q))
        diretion -= up;

    auto dt = Input::GetMouseScroll();
    if (Math::lengthSq(dt) > 0.0f)
    {
        m_movementSpeed += dt.y;
        if (m_movementSpeed < 0.0f)
            m_movementSpeed = 1.0f;
    }

    if (Math::lengthSq(diretion) > 0.0f)
    {
        diretion = Math::normalize(diretion);

        transform.position +=
            diretion * m_movementSpeed * (float)Clock::deltaTime;

        // scene->MarkChanged<TransformWorld>(go.entity);
    }

    if (Input::IsMousePress(MOUSE_BUTTON_MIDDLE))
    {
        ProcessMouse(Input::GetMouseDelta(), &transform);

        // scene->MarkChanged<TransformWorld>(go.entity);
    }
}

void CameraEditorSystem::ProcessMouse(
    const Vector2& delta,
    TransformLocal* transform)
{
    float yawDelta =
        -delta.x * sensitivity;

    float pitchDelta =
        -delta.y * sensitivity;

    Quaternion yawRotation =
        Math::angleAxis(
            Math::rad(yawDelta),
            type_traits<Vector3>::up
        );


    Quaternion pitchRotation =
        Math::angleAxis(
            Math::rad(pitchDelta),
            type_traits<Vector3>::right
        );


    transform->rotation =
        Math::normalize(
            yawRotation *
            transform->rotation *
            pitchRotation
        );
}


