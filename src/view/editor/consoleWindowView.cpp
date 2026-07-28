#include "view/editor/consoleWindowView.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/thread/threadpool.hpp"
#include "gr-base/managers/eventManager.hpp"
#include "imgui.h"

ConsoleWindowView::ConsoleWindowView()
{
    EventManager::Attach<Console_onAddMessage>(&ConsoleWindowView::onAddMessage, this);
}

void ConsoleWindowView::Draw()
{
    if (ImGui::Begin("Console"))
        render();
    ImGui::End();
}

void ConsoleWindowView::render()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto& log : m_logs)
        show(&log);

    if (refresh)
    {
        ImGui::SetScrollHereY(1.0f);
        refresh = false;
    }
}

void ConsoleWindowView::show(const Log* log)
{
    ImVec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
    switch (log->type)
    {
        case Log::LogType::INFO:
            color = {0.1f, 0.8f, 0.1f, 1.0f};
            break;
        case Log::LogType::WARNING:
            color = {0.8f, 0.8f, 0.1f, 1.0f};
            break;
        case Log::LogType::ERROR:
            color = {0.8f, 0.1f, 0.1f, 1.0f};
            break;
        default:
            break;
    }

    ImGui::TextColored(color, "%s", log->message.c_str());
}

void ConsoleWindowView::onAddMessage(Console_onAddMessage* event)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_logs.size() >= 1000)
        m_logs.pop_front();

    m_logs.push_back(event->log);

    refresh = true;
}


