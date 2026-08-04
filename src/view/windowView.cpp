#include "view/windowView.hpp"

void WindowView::Render()
{
    if (!open)
        return;

    Draw();
}
