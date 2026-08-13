#include "view/editor/sceneViewport.hpp"

#include "gr-base/managers/assetsManager.hpp"
#include "gr-base/render/pipeline/forwardPipeline.hpp"
#include "settings.h"

#include "view/editor/editorContext.hpp"

#include <gr-base/managers/renderPipelineManager.hpp>
#include <gr-base/managers/renderTargetManager.hpp>
#include <gr-base/managers/sceneManager.hpp>

#include <gr-base/render/rendertarget.hpp>

#include <gr-base/core/gameObject.hpp>
#include <gr-base/core/assetDB.hpp>
#include <gr-base/core/clock.hpp>
#include <gr-base/core/scene.hpp>

#include "gr-base/script/relationship.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"
#include "view/imguiRenderer.hpp"

#define GET_FLAG_NAME(c) #c

SceneViewport::SceneViewport(EditorContext* context) : context(context)
{
    s_texturePlayGame = context->iconsUI[IconType::ICON_PLAY_GAME];

    RenderTargetDesc desc;
    desc.width = 128;
    desc.height = 128;
    desc.colorFormat = TEXTURE_FORMAT_RGBA16F;

    m_backbuffer =
        RenderTargetManager::Create(desc);
}

void SceneViewport::Draw()
{
    if (ImGui::Begin("Viewport"))
        show();

    drop_in_window();

    ImGui::End();

    create_call_popup();

    create_popup_create_scene();
}

void SceneViewport::drop_in_window()
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    if (ImGui::BeginDragDropTargetCustom(window->InnerRect, window->ID))
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetID"))
        {
            if (context->scene == nullptr)
            {
                context->scene = CreateScene("Unknow");

                SceneManager::SetActive(context->scene);
            }

            size_t count =
                payload->DataSize / sizeof(AssetID);

            AssetID* ids =
                (AssetID*)payload->Data;

            for (size_t i=0;i<count;i++)
            {
                auto info = AssetDB::GetAsset(ids[i]);
                if (info != nullptr && info->type == AssetType_Model)
                    AssetsManager::LoadGameObjectAsync(info->uri);
            }
        }
        ImGui::EndDragDropTarget();
    }
}

void SceneViewport::show()
{
    if (!ImGui::BeginTabBar("##create_render_viewport_tabbar_0"))
        return;

    for (auto &scene : SceneManager::GetScenes())
    {
        auto name = SceneManager::GetSceneName(scene);

        bool bactive = true;
        if (ImGui::BeginTabItem(name, &bactive))
        {
            if (scene != context->scene)
            {
                if (context->scene != nullptr)
                    context->scene->deselect_all();

                context->scene = scene;
            }

            ImGui::EndTabItem();
        }

        if (!bactive)
        {
            SceneManager::SetActive(nullptr);

            SceneManager::Remove(scene);
        }
    }

    if (ImGui::TabItemButton("+"))
        popupFlags |= PopupFlags_CreateScene;

    ImGui::EndTabBar();
    
    if (context->scene != nullptr)
        render_scene(context->scene);
}

void SceneViewport::render_scene(Scene *scene)
{
    GameObject obj(scene, scene->GetCurrentCamera());
    if (obj.IsAlive())
        apply_viewport(&obj);

    auto regionAvail = ImGui::GetContentRegionAvail();
    auto windowPos = ImGui::GetWindowPos();
    auto cursorPos = ImGui::GetCursorPos();

    // Play button
    if (s_texturePlayGame.get() != nullptr)
    {
        s_texturePlayGame = context->iconsUI[ICON_PLAY_GAME];
        if (Settings::bGameRunning)
        {
            if (ImGui::IsKeyReleased(ImGuiKey_Escape) && ImGui::IsWindowHovered()) {
                // OnStopButton();
            } else {
                s_texturePlayGame = context->iconsUI[ICON_STOP_GAME];
            }
        }
        
        ImGui::SetCursorPos({regionAvail.x / 2 - cursorPos.x, cursorPos.y + 0.5f});

        if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(s_texturePlayGame->id), ImVec2(16, 16)))
        {
            if (!Settings::bGameRunning)
            {
                // OnPlayButton();
            } else {
                // OnStopButton();
            }
        }
    }

    regionAvail = ImGui::GetContentRegionAvail();
    cursorPos = ImGui::GetCursorPos();

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(
            windowPos.x + cursorPos.x,
            windowPos.y + cursorPos.y,
            regionAvail.x, regionAvail.y);

    if (obj.IsAlive())
    {
        // regionAvail = ImGui::GetContentRegionAvail();
        // cursorPos = ImGui::GetCursorPos();

        Camera* camera = obj.GetComponent<Camera>();
        if (camera != nullptr)
        {
            auto pipeline =
                RenderPipelineManager::Get<ForwardPipeline>(camera->pipeline);

            auto rt = RenderTargetManager::Get(pipeline->GetOutput());
            if (rt != nullptr)
            {
                auto texture = rt->textureColor;
                if (texture.get() != nullptr)
                    ImGui::Image(reinterpret_cast<ImTextureID>(texture->id), regionAvail, {0.0f, 1.0f}, {1.0f, 0.0f});
            }

            // drop_gameobject_in_scene();

            if (scene->has_object_selected() && !Settings::bGameRunning)
                create_render_guizmo(scene, &obj);
        }
    }

    // Fps Draw
    {
        fpsTimer += Clock::deltaTime;
        fpsFrameCount++;

        if (fpsTimer >= 1.0f)
        {
            displayFps = fpsFrameCount;
            displayMS = Clock::deltaTime * 1000.0;

            fpsMax = std::max(fpsMax, displayFps);
            fpsMin = std::min(fpsMin, displayFps);

            fpsFrameCount = 0;
            fpsTimer -= 1.0f;
        }

        char fps_text[256] = {0};
        sprintf(fps_text, "FPS: %d | Max: %d | Min: %d (ms: %.5lf)", displayFps, fpsMax, fpsMin, displayMS);

        auto textSize = ImGui::CalcTextSize(fps_text);

        ImGui::SetCursorPos({ regionAvail.x - textSize.x, cursorPos.y });

        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", fps_text);
    }

    ImGui::SetCursorPos({8.0f, cursorPos.y});

    ImGui::PushStyleColor(ImGuiCol_Text, {0.0f, 0.0f, 1.0f, 1.0f});

    ImGui::BeginGroup();
    if (ImGui::RadioButton("Local", m_guizmoManipulate == ImGuizmo::LOCAL))
        m_guizmoManipulate = ImGuizmo::LOCAL;
    
    ImGui::SameLine();
    if (ImGui::RadioButton("World", m_guizmoManipulate == ImGuizmo::WORLD))
        m_guizmoManipulate = ImGuizmo::WORLD;

    ImGui::EndGroup();

    ImGui::PopStyleColor();

    if (
            scene->has_object_selected() &&
            ImGui::IsWindowFocused() &&
            context->io->KeyShift &&
            ImGui::IsKeyReleased(ImGuiKey_D)
        )
    {
        auto temp = scene->get_object_selected();
        scene->deselect_all();

        for (auto go : temp)
            scene->select(scene->create(go));
    }
}

void SceneViewport::apply_viewport(GameObject* gameObject)
{
    auto camera = gameObject->GetComponent<Camera>();

    ImVec2 regionAvail = ImGui::GetContentRegionAvail();
    if (
            camera->viewport.w != regionAvail.x ||
            camera->viewport.h != regionAvail.y
        )
    {
        auto pipeline =
            RenderPipelineManager::Get<ForwardPipeline>(camera->pipeline);

        if (pipeline == nullptr)
            return;

        auto rt = RenderTargetManager::Get(pipeline->GetOutput());
        if (rt != nullptr)
            rt->resize(regionAvail.x, regionAvail.y);

        camera->viewport.w = regionAvail.x;
        camera->viewport.h = regionAvail.y;
    }
}

void SceneViewport::create_render_guizmo(Scene *scene, const GameObject* gameObjectCamera)
{
    if (
            ImGui::IsWindowHovered() &&
            !context->io->KeyShift &&
            !context->io->KeyCtrl
        )
    {
        if (ImGui::IsKeyReleased(ImGuiKey_G))
            m_guizmoOperation = ImGuizmo::TRANSLATE;
    
        if (ImGui::IsKeyReleased(ImGuiKey_R))
            m_guizmoOperation = ImGuizmo::ROTATE;
    
        if (ImGui::IsKeyReleased(ImGuiKey_S))
            m_guizmoOperation = ImGuizmo::SCALE;
    }

    auto camera = gameObjectCamera->GetComponent<CameraData>();

    for (auto entity : scene->get_object_selected())
    {
        GameObject gameObject(scene, entity);
        if (!gameObject.IsAlive())
            continue;

        create_guizmo_manipulate(gameObject, camera->projection, camera->view);
    }
}

void SceneViewport::create_guizmo_manipulate(const GameObject& gameObject, const Matrix4x4& projection, const Matrix4x4& view)
{
    ImGuizmo::SetID(gameObject.entity);
    auto transform = gameObject.GetComponent<TransformWorld>();
    auto transformLocal = gameObject.GetComponent<TransformLocal>();

    auto manipulatedWorld = transform->world;

    bool changed =
        ImGuizmo::Manipulate(
                &view[0][0],
                &projection[0][0],
                m_guizmoOperation,
                m_guizmoManipulate,
                &manipulatedWorld[0][0]
            );
    if (!changed)
        return;

    Matrix4x4 localMatrix = manipulatedWorld;

    if (gameObject.HasComponent<Relationship>())
    {
        auto rel = gameObject.GetComponent<Relationship>();
        if (rel->parent != INVALID_ENTITY)
        {
            const GameObject parent(gameObject.scene, rel->parent);
            if (parent.HasComponent<TransformWorld>())
            {
                auto transform = parent.GetComponent<TransformWorld>();

                localMatrix =
                    Math::inverse(transform->world) * manipulatedWorld;
            }
        }
    }

    Math::decompose(
            localMatrix,
            transformLocal->position,
            transformLocal->rotation,
            transformLocal->scale
        );

    context->scene->MarkChanged<TransformWorld>(gameObject.entity);
}

void SceneViewport::create_call_popup()
{
    switch (popupFlags)
    {
    case PopupFlags_CreateScene:
        ImGui::OpenPopup(GET_FLAG_NAME(PopupFlags_CreateScene));
        break;
    default:
        break;
    }

    popupFlags = 0;
}

#include <gr-base/system/transformsystem.hpp>
#include <gr-base/system/physicsSystem.hpp>
#include <gr-base/system/camerasystem.hpp>
#include <gr-base/system/renderSystem.hpp>

#include "system/cameraEditorSystem.hpp"

void SceneViewport::create_popup_create_scene()
{
    if (!ImGui::BeginPopup(GET_FLAG_NAME(PopupFlags_CreateScene)))
        return;

    ImGui::Text("Input scene name");

    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        ImGui::SetKeyboardFocusHere();

    std::vector<char> buffer(256, '\0');
    if (ImGui::InputText("##popup_create_scene_inputtext", buffer.data(), buffer.size() - 1, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        auto scene =
            CreateScene(buffer.data());

        SceneManager::SetActive(scene);

        context->scene = scene;

        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}


Scene* SceneViewport::CreateScene(const std::string& name)
{
    Scene* scene = SceneManager::Create(name);

    scene->AddSystem<CameraEditorSystem>();
    scene->AddSystem<TransformSystem>();
    scene->AddSystem<CameraSystem>();

    scene->AddSystem<RenderSystem>();
    scene->AddSystem<PhysicsSystem>();

    Entity camera = scene->create("MainCamera");
    {
        GameObject go(scene, camera);
        go.AddComponent<CameraData>();
        go.GetOrAddComponent<Camera>();
    }

    scene->SetMainCamera(camera);

    return scene;
}

