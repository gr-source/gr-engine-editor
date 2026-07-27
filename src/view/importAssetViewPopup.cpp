#include "view/importAssetViewPopup.hpp"

#include "gr-base/core/thread/taskmanager.hpp"

#include "gr-base/managers/eventManager.hpp"
#include "imgui.h"

#include "task/fbxSceneBuildTask.hpp.hpp"

ImportAssetViewPopup::ImportAssetViewPopup()
{
    EventManager::Attach<ImportAssetViewPopup_onAddFbxScene>(
            &ImportAssetViewPopup::onAddFbxScene, this
        );
}

void ImportAssetViewPopup::Draw()
{
    auto* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowSize(ImVec2(420.0f, 320.0f), ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(
        viewport->GetCenter(),
        ImGuiCond_Appearing,
        ImVec2(0.5f, 0.5f)
    );

    if (!ImGui::Begin(
            "Import FBX Assets",
            &open,
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoSavedSettings))
    {
        ImGui::End();
        return;
    }

    if (!open)
    {
        m_scenes.clear();
        ImGui::End();
        return;
    }

    ImGui::Text("Found %zu scene%s",
        m_scenes.size(),
        m_scenes.size() == 1 ? "" : "s");

    ImGui::Separator();
    ImGui::Spacing();

    if (m_scenes.empty())
    {
        open = false;
    }
    else
    {
        for (auto it=m_scenes.begin();it!=m_scenes.end();)
        {
            ImGui::PushID(&(*it));

            bool visible = true;
            if (ImGui::CollapsingHeader(it->name.c_str(), &visible))
            {
                ImGui::BulletText("Meshes: %lu", it->meshes.size());
                ImGui::BulletText("Materials: %lu", it->materials.size());
                ImGui::BulletText("Animations: %lu", it->animations.size());

                ImGui::Spacing();

                auto avail = ImGui::GetContentRegionAvail();

                if (ImGui::Button("Import", ImVec2(avail.x, 0.0f)))
                {
                }
            }

            ImGui::PopID();

            if (!visible)
                it = m_scenes.erase(it);
            else
                it++;
        }
    }

    auto avail = ImGui::GetContentRegionAvail();

    if (ImGui::Button("Import All", ImVec2(avail.x, 0.0f)))
    {
        for (auto& scene : m_scenes)
            TaskManager::Add<FbxSceneBuildTask>(scene);

        m_scenes.clear();
    }

    ImGui::End();
}

void ImportAssetViewPopup::onAddFbxScene(ImportAssetViewPopup_onAddFbxScene* event)
{
    m_scenes.push_back(event->fbxScene);
    open = true;
}

