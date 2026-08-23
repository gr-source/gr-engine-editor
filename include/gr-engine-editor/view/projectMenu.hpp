#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <atomic>
#include <array>
#include <memory>

#include "default.hpp"

#include <imgui.h>

#include "core/project.hpp"

#include "windowView.hpp"

using ProjectMenuPopup_ = std::uint32_t;

enum ProjectMenuPopup : ProjectMenuPopup_
{
    ProjectMenuPopup_CreateProject = 1 << 1,
};

class ProjectMenu : public WindowView
{
public:
    static std::shared_ptr<Project> Load(const std::string& path);

    ProjectMenu();
    ~ProjectMenu() override;

    void Draw() override;

private:
    std::vector<Project> m_projects;

    ImGuiViewport *m_mainViewport;

    ImGuiStyle *m_style;

    ProjectMenuPopup_ popup_flags;

    ImGuiIO *m_io;

    void create_window_project_list();

    void render_project_list();

    void popup_create_new_project();

    bool create_project(const std::string& name);

    bool create_project_metadata();

    static void CreateProjectAssets(const std::string &path);

    void read_project_directory();

    static bool ReadLastProject();
};


