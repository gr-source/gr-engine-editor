#include "view/sceneInspectorView.hpp"

#include "gr-base/core/scene.hpp"
#include "gr-base/ecs/systemRegistry.hpp"
#include "imgui.h"

void SceneInspectorView::Draw()
{
    Scene* scene = Scene::GetCurrent();

    if (ImGui::Begin("Scene Inspector") && scene != nullptr)
        renderer(scene);

    ImGui::End();
}

void SceneInspectorView::renderer(Scene* scene)
{
    auto avail = ImGui::GetContentRegionAvail();

    ImGui::SeparatorText("Systems Added");

    for (auto systemID : scene->GetSystemIDs())
    {
        auto info = SystemRegistry::GetSystemByID(systemID);
        assert(info != nullptr);

        auto system = scene->GetSystemByID(systemID);

        bool valid = system != nullptr;
        if (ImGui::CollapsingHeader(info->name, &valid))
            ShowSystemInfo(scene, info, system);

        if (!valid)
            scene->DelSystemByID(systemID);
    }

    if (ImGui::Button("AddSystem", {avail.x, 0.0f}))
        ImGui::OpenPopup("AddSystemPopup");

    AddSystemPopup(scene);
}
// private

void SceneInspectorView::ShowSystemInfo(Scene* scene, const SystemInfo* info, ISystem* system)
{
    system->OnGUI(scene);
}

void SceneInspectorView::AddSystemPopup(Scene* scene)
{
    if (ImGui::BeginPopup("AddSystemPopup"))
    {
        ImGui::SeparatorText("System available");

        auto infos = SystemRegistry::GetSystems();
        for (size_t i=0;i<SystemRegistry::GetCount();i++)
        {
            auto& info = infos[i];

            if (scene->GetSystemByID(info.index))
                continue;

            if (ImGui::MenuItem(info.name))
                scene->AddSystemByID(info.index);
        }

        ImGui::EndPopup();
    }
}


