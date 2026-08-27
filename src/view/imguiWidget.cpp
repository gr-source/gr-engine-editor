#include "view/imguiWidget.hpp"

const bool ImguiWidget::TextureInputUI(const ImColor& color) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 size = ImGui::GetItemRectSize();

    // Define as coordenadas do retângulo
    ImVec2 rectMin = ImGui::GetCursorScreenPos();
    ImVec2 rectMax = ImVec2(rectMin.x + size.x + 100, rectMin.y + size.y);

    // Adiciona o retângulo à lista de desenho
    drawList->AddRectFilled(rectMin, rectMax, color);

    return ImGui::InvisibleButton(std::string("#TextureInputUI" + std::to_string(color.Value.x)).c_str(), ImVec2(rectMax.x - rectMin.x, size.y));
}

const bool ImguiWidget::PopupChangeColor(const std::string& name, Color& color) {
    if (ImGui::BeginPopup(std::string("#PopupChangeColor" + name).c_str())) {
        auto isOpen = ImGui::ColorPicker4(name.c_str(), color.data);
        ImGui::EndPopup();
        return isOpen;
    }
    return false;
}

bool ImguiWidget::DragScalarEx(const char** items, int numItems, float* data, float v_speed, float p_min, float p_max) {
    bool value_changed = false;

    ImGui::BeginGroup();
    ImGui::PushID(&data);

    ImGui::PushMultiItemsWidths(numItems, ImGui::CalcItemWidth());
    for (int i = 0; i < numItems; i++)
    {
        ImGui::Text("%s", items[i]);
        ImGui::SameLine();
        
        value_changed |= ImGui::DragFloat("##drag", &data[i], v_speed, p_min, p_max);
        ImGui::PopItemWidth();

        if (i < numItems - 1)
            ImGui::SameLine();
    }

    ImGui::PopID();
    ImGui::EndGroup();
    return value_changed;
}

bool ImguiWidget::ImageTreeNode(ImTextureID textureID, const char *name) {
    const ImGuiStyle& style = ImGui::GetStyle();
    ImGuiStorage* storage = ImGui::GetStateStorage();

    ImGui::PushID(name);

    // ImU32 id = ImGui::GetID(name);

    bool opened = false;//storage->GetInt(id, 0);
    float x = ImGui::GetCursorPosX();

    // ImGui::ArrowButton

    // float cross_extent = ImGui::GetFontSize() * 0.5f * 0.7071f - 1.0f;

    ImGui::BeginGroup();
    opened = ImGui::ArrowButton(name, ImGuiDir_Right);
    // opened = ImGui::ImageButton(textureID, {16, 16});

    ImGui::SameLine();

    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();
    if (hovered || active)
        ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImColor(ImGui::GetStyle().Colors[active ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered]));

    ImGui::Text("%s", name);
    ImGui::EndGroup();
    if (opened)
        ImGui::TreePush(name);
    ImGui::PopID();
    return opened;
}

bool ImguiWidget::CloseButton() {
    return false;
}

#include <iostream>
bool ImguiWidget::ImageAndTextButton(ImTextureID textureID, const char *label, bool selected, ImVec2 size)
{
    size.x = size.x <= 0.0f ? 32.0f : size.x;
    size.y = size.y <= 0.0f ? 32.0f : size.y;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    auto textSize = ImGui::CalcTextSize(label);

    auto &g = ImGui::GetStyle();

    const ImVec2 padding = g.FramePadding;
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2.0f);

    bool press = ImGui::InvisibleButton(label, (size + padding * 2.0f) + textSize);

    window->DrawList->AddImage(textureID, bb.Min + padding, bb.Max - padding, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), selected ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 255, 255, 255));

    ImVec2 button_pos{bb.Min.x, bb.Max.y};

    window->DrawList->AddText(button_pos, IM_COL32(255, 0, 255, 255), label);

    return press;
}

bool ImguiWidget::ButtonWithComment(const char *name, const char *comment)
{
    auto avail = ImGui::GetContentRegionAvail();

    bool press = ImGui::Selectable(name, false, 0);

    ImGui::SameLine();

    float textWidth = ImGui::CalcTextSize(comment).x + ImGui::CalcTextSize(name).x;

    float padding = 5.0f;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avail.x - textWidth - padding);

    ImGui::TextUnformatted(comment);

    return press;
}




