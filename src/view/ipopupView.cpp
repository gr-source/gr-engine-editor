#include "view/ipopupView.hpp"

#include "imgui.h"

void IPopupView::Render()
{
    if (!requestOpen)
        return;

    DrawPopup();
}

