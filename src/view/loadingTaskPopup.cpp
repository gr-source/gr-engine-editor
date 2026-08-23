#include "view/loadingTaskPopup.hpp"

#include "imgui.h"

LoadingTaskPopup* LoadingTaskPopup::s_LoadingTaskPopup = nullptr;

Handle LoadingTaskPopup::Create(const std::string& title, const std::string& message)
{
    return s_LoadingTaskPopup->create(title, message);
}

void LoadingTaskPopup::Destroy(Handle handle)
{
    s_LoadingTaskPopup->destroy(handle);
}

LoadingTaskPopup::LoadingTaskPopup()
{
    s_LoadingTaskPopup = this;
}

LoadingTaskPopup::~LoadingTaskPopup()
{
    s_LoadingTaskPopup = nullptr;
}

// private
Handle LoadingTaskPopup::create(const std::string& title, const std::string& message)
{
    Handle h = m_tasks.push({
            .name = title,
            .message = message
        });

    open = true;

    return h;
}

void LoadingTaskPopup::destroy(Handle handle)
{
    m_tasks.destroy(handle);

    if (!m_tasks.count())
        open = false;
}

// private
void LoadingTaskPopup::Draw()
{
    auto* vp = ImGui::GetMainViewport();

    ImVec2 size(200.0f, 100.0f);
    constexpr float margin = 64.0f;

    ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(
        ImVec2(
            vp->WorkPos.x + vp->WorkSize.x - size.x - margin,
            vp->WorkPos.y + vp->WorkSize.y - size.y - margin
        ),
        ImGuiCond_Appearing
    );

    ImGui::Begin(
        "LoadingTaskPopup::Render",
        nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoSavedSettings
    );

    for (size_t i = 0; i < m_tasks.count(); ++i)
    {
        const auto& info = m_tasks.data()[i];

        ImGui::SeparatorText(info.name.c_str());
        ImGui::Text("%s", info.message.c_str());
    }

    ImGui::End();
}
