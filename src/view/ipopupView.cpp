#include "view/ipopupView.hpp"

void IPopupView::Render()
{
    if (!open)
        return;

    Draw();
}

