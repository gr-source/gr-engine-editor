#include "view/workspace/mainWorkspace.hpp"

#include "event/mainWorkspaceEvent.hpp"
#include "event/editorApplicationEvent.hpp"

#include <gr-base/managers/eventManager.hpp>

#include <gr-base/file/file.hpp>

extern "C"
{
#include <gr-ini/config.h>
}

#include "view/checkupdate.hpp"

#include "imgui.h"
#include "view/projectMenu.hpp"

MainWorkspace::MainWorkspace()
{
    EventManager::Subscribe<MainWorkspace_onPostCheckUpdate>(&MainWorkspace::onPostCheckUpdate, this);
}

void MainWorkspace::DrawWorkspace()
{
    viewManager.renderer();
}

// private
void MainWorkspace::Initialize()
{
    viewManager.add<CheckUpdate>()->open = true;
    viewManager.add<ProjectMenu>()->open = true;
}

void MainWorkspace::onPostCheckUpdate(MainWorkspace_onPostCheckUpdate& event)
{
    viewManager.hide<CheckUpdate>();

    std::string projectFile;

    auto config = config_create();
    if (config != NULL)
    {
        std::string metadataFile = gfile::get_data_directory() + "/MainWorkspace.ini";
        if (config_load(config, metadataFile.c_str()) == CONFIG_TRUE)
        {
            gSection* section = config_find_section(config, "project");
            if (section != NULL)
            {
                const char* path = config_get_string(section, "path");
                if (path != NULL && gfile::exists(path))
                    projectFile = path;
            }
        }
    }

    config_release(config);

    if (!projectFile.empty())
    {
        EventManager::Emit<EditorApplicationEvent_onOpenProject>({
                .path = projectFile
            });
    }
}

