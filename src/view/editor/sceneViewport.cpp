#include "view/editor/sceneViewport.hpp"

#include "gr-base/managers/sceneManager.hpp"

#include "gr-base/core/scene.hpp"

#include "imgui.h"

void SceneViewport::Draw()
{
    if (ImGui::Begin("Viewport"))
        show();
    ImGui::End();
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
            context->scene = nullptr;
            
            SceneManager::SetActive(nullptr);

            SceneManager::Remove(scene);
        }
    }

    if (ImGui::TabItemButton("+"))
    {}
        // popupFlags |= EditorPopupFlags_CreateScene;

    ImGui::EndTabBar();
    
    if (context->scene != nullptr)
        render_scene(context->scene);
}

void SceneViewport::render_scene(Scene *scene)
{
    /*
    UpdateResizeSceneFramebuffer();

    auto regionAvail = ImGui::GetContentRegionAvail();
    auto windowPos = ImGui::GetWindowPos();
    auto cursorPos = ImGui::GetCursorPos();

    // Play button
    if (s_texturePlayGame.get() != nullptr)
    {
        s_texturePlayGame = iconsUI[ICON_PLAY_GAME];
        if (Settings::bGameRunning) {
            if (ImGui::IsKeyReleased(ImGuiKey_Escape) && ImGui::IsWindowHovered()) {
                OnStopButton();
            } else {
                s_texturePlayGame = iconsUI[ICON_STOP_GAME];
            }
        }
        
        ImGui::SetCursorPos({regionAvail.x / 2 - cursorPos.x, cursorPos.y + 0.5f});

        if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(s_texturePlayGame->id), ImVec2(16, 16)))
        {
            if (!Settings::bGameRunning)
            {
                OnPlayButton();
            } else {
                OnStopButton();
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

    GameObject obj(m_scene, m_scene->GetCurrentCamera());
    if (obj.IsAlive())
    {
        // regionAvail = ImGui::GetContentRegionAvail();
        // cursorPos = ImGui::GetCursorPos();

        Camera* camera = obj.GetComponent<Camera>();
        if (camera != nullptr)
        {
            RenderTarget* rendertarget = camera->rendertarget;

            ImGui::Image(reinterpret_cast<ImTextureID>(rendertarget->GetTextureID()), regionAvail, {0.0f, 1.0f}, {1.0f, 0.0f});

            drop_gameobject_in_scene();

            if (scene->has_object_selected() && !Settings::bGameRunning)
                create_render_guizmo(scene, &obj);
        }
    }

    // Fps Draw
    {
        fpsTimer += static_cast<float>(Clock::deltaTime);
        fpsFrameCount++;

        if (fpsTimer >= 1.0f)
        {
            fps = fpsFrameCount;
            fpsMax = std::max(fpsMax, fps);
            fpsMin = std::min(fpsMin, fps);

            fpsFrameCount = 0;
            fpsTimer = 0.0f;
        }

        char fps_text[256] = {0};
        sprintf(fps_text, "FPS: %d\tMax: %d\tMin: %d", fps, fpsMax, fpsMin);

        auto textSize = ImGui::CalcTextSize(fps_text);

        ImGui::SetCursorPos(ImVec2(regionAvail.x - textSize.x, cursorPos.y + textSize.y));

        ImGui::TextColored(FPSColor, "%s", fps_text);
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

    if (m_scene->has_object_selected() && ImGui::IsWindowFocused() && mImGuiIO->KeyShift && ImGui::IsKeyReleased(ImGuiKey_D))
    {
        auto temp = m_scene->get_object_selected();
        m_scene->deselect_all();

        for (auto go : temp)
            m_scene->select(m_scene->create(go));
    }

    if (ImGui::IsWindowFocused() && mImGuiIO->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_S))
    {
        // std::string path = m_project->path + "/assets/scene";

        // for (auto &scene : SceneManager::GetScenes())
            // TaskManager::Add<SceneSaveTask>(path + "/" + SceneManager::GetSceneName(scene) + ".scn", scene);
    }
    */
}

