#include "view/checkupdate.hpp"

#include "event/projectBrowserEvent.hpp"
#include "gr-base/core/thread/taskmanager.hpp"

#include "gr-base/managers/eventManager.hpp"
#include "task/checkUpdateTask.hpp"

#include "imgui.h"

CheckUpdate::CheckUpdate()
{
    TaskManager::Add<CheckUpdateTask>(this);
}

void CheckUpdate::addFile(const std::string& file)
{
    m_files.push_back(file);
}

void CheckUpdate::taskFinished()
{
    EventManager::Notify<ProjectBrowser_onCheckProjectExists>(nullptr);
}

// private
void CheckUpdate::DrawPopup()
{
    auto vp = ImGui::GetMainViewport();

    ImVec2 size(500, 300);

    ImGui::SetNextWindowSize(size, ImGuiCond_Always);
    ImGui::SetNextWindowPos(
        ImVec2(
            vp->Pos.x + (vp->Size.x - size.x) * 0.5f,
            vp->Pos.y + (vp->Size.y - size.y) * 0.5f
        ),
        ImGuiCond_Always
    );
    ImGui::Begin("Check Update", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    auto avail = ImGui::GetContentRegionAvail();

    const char* text = "Checking resources...";

    // ImGui::SetCursorPos(avail - (ImGui::CalcTextSize(text) / 2.0f));

    ImGui::SeparatorText("Checking resources...");

    for (auto& file : m_files)
    {
        ImGui::Text("File: %s", file.c_str());
    }

    ImGui::End();
}

