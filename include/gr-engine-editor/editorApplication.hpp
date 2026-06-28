#pragma once

#include "core/project.hpp"

#include <gr-base/application.hpp>
#include <gr-base/runtime.hpp>

#include <memory>

extern "C"
{
#include <gr-ini/config.h>
}

class EditorApplication : public Application
{
public:
    EditorApplication();
    ~EditorApplication() override;

    void initialize() override;
    
    void shutdown() override;

    void run() override;

    void dropPath(const char* path) override;

    inline Project* getProject() const
    {
        return m_project != nullptr ? m_project.get() : nullptr;
    }

private:
    std::shared_ptr<Project> m_project;

    gConfig* config;

    Runtime runtime;

    void update();

    void renderer();

    void checkProject();

    void loadConfig();

    void saveConfig();
};


