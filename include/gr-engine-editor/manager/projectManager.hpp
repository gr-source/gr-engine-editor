#pragma once

#include "core/project.hpp"

#include <memory>

class ProjectManager
{
public:
    static ProjectManager& GetInstance();

    bool Open(const std::string& path);

    void Close();

    bool Has() const;

    Project* GetCurrent() const;

private:
    std::unique_ptr<Project> m_project;
};


