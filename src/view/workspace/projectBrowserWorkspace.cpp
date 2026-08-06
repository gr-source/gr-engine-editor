#include "view/workspace/projectBrowserWorkspace.hpp"

#include "event/editorApplicationEvent.hpp"
#include "gr-base/managers/eventManager.hpp"

#include "event/projectBrowserEvent.hpp"

#include "view/checkupdate.hpp"

#include "imgui.h"
#include "view/projectMenu.hpp"

ProjectBrowser::ProjectBrowser()
{
    viewManager.add<CheckUpdate>()->open = true;
    viewManager.add<ProjectMenu>()->open = true;

    EventManager::Attach<ProjectBrowser_onCheckProjectExists>(&ProjectBrowser::onReadProjects, this);
}

void ProjectBrowser::DrawWorkspace()
{
    viewManager.renderer();
}

void ProjectBrowser::onReadProjects(IEvent* event)
{
    viewManager.hide<CheckUpdate>();

    EventManager::Notify<EditorApplicationEvent_onCheckProject>(nullptr);
}

