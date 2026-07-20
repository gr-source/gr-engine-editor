#include "view/windowView.hpp"

#include "imgui.h"

void WindowView::Render()
{
    if (!open)
        return;

    Draw();
}
