#include "manager/projectManager.hpp"

extern "C"
{
#include <gr-ini/config.h>
}

// public
ProjectManager& ProjectManager::GetInstance()
{
    static ProjectManager gProjectManager;
    return gProjectManager;
}

bool ProjectManager::Open(const std::string& path)
{
    auto config = config_create();
    if (config == nullptr)
        return false;

    m_project = nullptr;

    if (config_load(config, path.c_str()) == CONFIG_TRUE)
    {
        auto section = config_find_section(config, "project");
        if (section != nullptr)
        {
            auto name = config_get_string(section, "name");
            auto path = config_get_string(section, "path");
            auto version = config_get_string(section, "version");

            m_project = std::make_unique<Project>();
            m_project->name = name;
            m_project->path = path;
            m_project->version = version;
        }
    }
    config_release(config);

    return Has();
}

void ProjectManager::Close()
{
    m_project = nullptr;
}

bool ProjectManager::Has() const
{
    return m_project != nullptr;
}

Project* ProjectManager::GetCurrent() const
{
    return Has() ? m_project.get() : nullptr;
}


