#pragma once

#include "view/imguiRenderer.hpp"

#include <gr-render/color.hpp>
#include <gr-math/gmath.hpp>

// #include <string>

class ImguiWidget
{
public:
    static const bool TextureInputUI(const ImColor& color);

    static const bool PopupChangeColor(const std::string& name, Color& color);

    static bool DragScalarEx(const char **items, int numItems, float *data, float v_speed, float p_min, float p_max);

    static bool ImageTreeNode(ImTextureID textureID, const char *name);

    static bool CloseButton();

    static bool ImageAndTextButton(ImTextureID textureID, const char *label, bool selected = false, ImVec2 size = {0.0f, 0.0f});

    static bool ButtonWithComment(const char *name, const char *comment);
};






