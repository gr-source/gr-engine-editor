#include "view/sceneInspectorView.hpp"

#include "gr-base/core/scene.hpp"
#include "gr-base/ecs/systemRegistry.hpp"
#include "imgui.h"

void SceneInspectorView::onRenderUI()
{
    Scene* scene = Scene::GetCurrent();
    if (scene == nullptr)
        return;

    auto avail = ImGui::GetContentRegionAvail();

    ImGui::SeparatorText("Systems Added");

    for (auto systemID : scene->GetSystemIDs())
    {
        auto info = SystemRegistry::GetSystemByID(systemID);
        assert(info != nullptr);

        auto system = scene->GetSystem(systemID);

        bool valid = system != nullptr;
        if (ImGui::CollapsingHeader(info->name, &valid))
            ShowSystemInfo(info, system);

        if (!valid)
            scene->DelSystemByID(systemID);
    }

    if (ImGui::Button("AddSystem", {avail.x, 0.0f}))
        ImGui::OpenPopup("AddSystemPopup");

    AddSystemPopup(scene);
}
// private

void SceneInspectorView::ShowSystemInfo(const SystemInfo* info, ISystem* system)
{
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

            if (scene->GetSystem(info.index))
                continue;

            if (ImGui::MenuItem(info.name))
                scene->AddSystemByID(info.index);
        }

        ImGui::EndPopup();
    }
}


