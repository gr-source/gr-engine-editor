#pragma once

#include "event/projectBrowserEvent.hpp"
#include "manager/viewManager.hpp"

#include "view/workspace/iworkspace.hpp"

class ProjectBrowser : public IWorkspace
{
public:
    ProjectBrowser();

    const char* GetWorkspaceID() const override
    {
        return "Project Browser";
    }

    void DrawWorkspace() override;

private:
    ViewManager viewManager;

    void onReadProjects(ProjectBrowser_onCheckProjectExists& event);
};

