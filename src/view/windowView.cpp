#include "view/iwindowView.hpp"

void IWindowView::Render()
{
    if (!open)
        return;

    Draw();
}
