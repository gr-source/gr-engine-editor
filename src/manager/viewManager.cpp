#include "manager/viewManager.hpp"

#include "gr-base/core/logger.hpp"

#include <imgui.h>

void ViewManager::renderer()
{
    for (auto& view : m_windows)
    {
        if (view != nullptr)
            view->Render();
    }
}

void ViewManager::clear()
{
    m_windows.clear();
}


