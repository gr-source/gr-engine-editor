#include "view/editor/consoleWindowView.hpp"

#include "gr-base/managers/eventManager.hpp"

#include "gr-base/core/console.hpp"

#include "imgui.h"

ConsoleWindowView::ConsoleWindowView()
{
    EventManager::Subscribe<Console_onAddMessage>(&ConsoleWindowView::onAddMessage, this);
}

void ConsoleWindowView::Draw()
{
    if (!ImGui::Begin("Console"))
        return ImGui::End();

    draw_menu();

    if (ImGui::BeginChild("scrolling", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
    {
        draw_child();
    }

    ImGui::EndChild();

    ImGui::End();
}

void ConsoleWindowView::draw_menu()
{
    if (ImGui::Button("Text"))
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        gTextBuffer.append(
            "Texto normal "
            "{FF0000}vermelho "
            "{00FF00}verde "
            "{0000FF}azul "
            "{FFFFFF}normal novamente"
            "\n"
        );
    }
}

void ConsoleWindowView::draw_child()
{
    TextBuffer textBuffer;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        textBuffer = gTextBuffer;
    }

    ImGuiListClipper clipper;
    clipper.Begin((int)textBuffer.lineOffsets.size());

    const auto& LineOffsets = textBuffer.lineOffsets;
    const char* buf = textBuffer.begin();
    const char* buf_end = textBuffer.end();

    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        {
            const char* line_start = buf + LineOffsets[i];
            const char* line_end = (i + 1 < LineOffsets.size()) ? (buf + LineOffsets[i + 1] - 1) : buf_end;
            show(line_start, line_end);
        }
    }
    clipper.End();

    if (gAutoScrool && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);
}

void ConsoleWindowView::show(const char* line_start, const char* line_end)
{
    if (line_start == line_end)
        return;
    
    ImColor color = { 255, 255, 255, 255 };

    while (*line_start && line_start != line_end)
    {
        if (*line_start != '{')
        {
            const char* start = line_start;

            while (*line_start && line_start != line_end && *line_start != '{')
                ++line_start;

            int length = (int)(line_start - start);

            ImGui::TextColored(color, "%.*s",
                length,
                start);
            if (*line_start && line_start != line_end)
                ImGui::SameLine(0.0f, 0.0f);
        }
        else
        {
            ++line_start;

            const char* start = line_start;

            while (*line_start && line_start != line_end && *line_start != '}')
                ++line_start;

            if (*line_start == '}')
            {
                uint32_t bgra = (uint32_t)strtol(start, nullptr, 16);
                bgra = ((bgra & 0x00FFFFFF) | 0xFF000000);

                uint8_t b = (bgra >> 0) & 0xFF;
                uint8_t g = (bgra >> 8) & 0xFF;
                uint8_t r = (bgra >> 16) & 0xFF;
                uint8_t a = (bgra >> 24) & 0xFF;

                color = ImColor(IM_COL32(r, g, b, a));

                ++line_start;
            }
        }
    }
}

void ConsoleWindowView::onAddMessage(Console_onAddMessage& event)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    gTextBuffer.append("%s\n", event.message.c_str());
}


