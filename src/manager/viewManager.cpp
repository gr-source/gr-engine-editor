#include "manager/viewManager.hpp"

#include "gr-base/core/logger.hpp"

#include <imgui.h>

ViewManager::ViewManager()
{}

ViewManager::~ViewManager()
{}

void ViewManager::renderer()
{
    for (auto& view : m_windows)
    {
        if (view != nullptr)
            view->Render();
    }

    for (auto& view : m_popups)
    {
        if (view != nullptr)
            view->Render();
    }
}

void ViewManager::clear()
{
    m_popups.clear();
    m_windows.clear();
}


