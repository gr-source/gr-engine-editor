#pragma once

#include "event/mainWorkspaceEvent.hpp"

#include "manager/viewManager.hpp"

#include "view/workspace/iworkspace.hpp"

class MainWorkspace : public IWorkspace
{
public:
    MainWorkspace();

    void DrawWorkspace() override;

private:
    ViewManager viewManager;

    void Initialize() override;

    void onPostCheckUpdate(MainWorkspace_onPostCheckUpdate& event);
};

