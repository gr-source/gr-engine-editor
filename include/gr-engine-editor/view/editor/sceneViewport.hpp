#pragma once

#include "view/windowView.hpp"

#include "gr-base/asset/assetPtr.hpp"

#include <gr-render/texture.h>

#include "imgui.h"
#include "guizmo/ImGuizmo.h"

#include <gr-math/gmath.hpp>

#include <climits>
#include <memory>
#include <array>

struct EditorContext;
struct GameObject;

class RenderTarget;
class Scene;

class SceneViewport : public WindowView
{
public:
    SceneViewport(EditorContext* context);

    void Draw() override;

private:
    enum PopupFlags
    {
        PopupFlags_CreateScene = 1 << 1
    };

    // static constexpr int HistorySize = 120;

    AssetPtr<gr::texture> s_texturePlayGame;

    EditorContext* context;

    double fpsTimer =
        0.0;

    int fpsFrameCount =
        0;

    int fpsMax = INT_MIN;
    int fpsMin = INT_MAX;

    double displayMS = 0.0;
    int displayFps = 0;

    // std::array<float, HistorySize> fpsHistory{};

    // int historyOffset = 0;

    ImGuizmo::OPERATION m_guizmoOperation =
        ImGuizmo::TRANSLATE;

    ImGuizmo::MODE m_guizmoManipulate =
        ImGuizmo::WORLD;

    std::shared_ptr<RenderTarget> m_backbuffer;

    uint32_t popupFlags = 0;

    void drop_in_window();

    void show();

    void render_scene(Scene *scene);

    void apply_viewport(GameObject* gameObject);

    void create_render_guizmo(Scene *scene, const GameObject* gameObjectCamera);

    void create_guizmo_manipulate(const GameObject& gameObject, const Matrix4x4& projection, const Matrix4x4& view);

    // popup
    void create_call_popup();

    void create_popup_create_scene();

    Scene* CreateScene(const std::string& name);
};


