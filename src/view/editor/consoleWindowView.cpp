#include "view/editor/consoleWindowView.hpp"

#include "gr-base/managers/eventManager.hpp"

#include "gr-base/core/console.hpp"

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
    if (ImGui::Button("Text"))
    {
        Console::LogDefault(
            "Texto normal "
            "{FF0000}vermelho "
            "{00FF00}verde "
            "{0000FF}azul "
            "{FFFFFF}normal novamente"
        );
    }

    std::deque<std::string> messages;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        messages = m_messages;
    }

    ImGuiListClipper clipper;
    clipper.Begin((int)messages.size());

    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
            show(messages[i].c_str());
    }

    // for (auto& message : messages)
        // show(message.c_str());

    if (bScroolConsole.load(std::memory_order_acquire))
    {
        bScroolConsole.store(false, std::memory_order_release);

        ImGui::SetScrollHereY(1.0f);
    }
}

void ConsoleWindowView::show(const char* p)
{
    ImColor color(255, 255, 255, 255);

    while (*p)
    {
        if (*p != '{')
        {
            const char* start = p;

            while (*p && *p != '{')
                ++p;

            ImGui::TextColored(color, "%.*s",
                static_cast<int>(p - start),
                start);
            if (*p)
                ImGui::SameLine(0.0f, 0.0f);
        }
        else
        {
            ++p;

            const char* start = p;

            while (*p && *p != '}')
                ++p;

            if (*p == '}')
            {
                uint32_t bgra = (uint32_t)strtol(start, nullptr, 16);
                bgra = ((bgra & 0x00FFFFFF) | 0xFF000000);

                uint8_t b = (bgra >> 0) & 0xFF;
                uint8_t g = (bgra >> 8) & 0xFF;
                uint8_t r = (bgra >> 16) & 0xFF;
                uint8_t a = (bgra >> 24) & 0xFF;

                color = ImColor(IM_COL32(r, g, b, a));

                ++p;
            }
        }
    }
}

void ConsoleWindowView::onAddMessage(Console_onAddMessage* event)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_messages.size() >= 1000)
            m_messages.pop_front();

        m_messages.push_back(event->message);
    }

    bScroolConsole.store(true, std::memory_order_release);
}


