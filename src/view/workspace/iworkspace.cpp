#include "view/workspace/iworkspace.hpp"

#include "imgui.h"

IWorkspace::IWorkspace()
{}

void IWorkspace::Render()
{
    /*
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    if (ImGui::Begin(GetWorkspaceID(), nullptr,
                ImGuiWindowFlags_NoDocking
                | ImGuiWindowFlags_NoTitleBar
                | ImGuiWindowFlags_NoCollapse
                | ImGuiWindowFlags_NoResize
                | ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_NoBringToFrontOnFocus
                | ImGuiWindowFlags_NoNavFocus
            ))
        */
    DrawWorkspace();

    // ImGui::End();
}


