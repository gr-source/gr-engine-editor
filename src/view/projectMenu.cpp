#include "view/projectMenu.hpp"

#include "editorApplication.hpp"
#include "gr-base/application.hpp"
#include "gr-base/core/mainthreaddispatcher.hpp"
#include "imgui.h"

#include <gr-base/core/thread/threadpool.hpp>

#include <gr-base/core/console.hpp>
#include <gr-base/core/scene.hpp>

#include <gr-base/core/file/file.hpp>

#include <filesystem>

#include <iostream>

extern "C"
{
#include <gr-ini/config.h>
}

static const std::array<const std::string, 8> m_assetsFile {
    "animation",
    "material",
    "texture",
    "shader",
    "script",
    "mesh",
    "scene",
    "model",
};


std::shared_ptr<Project> ProjectMenu::Load(const std::string& path)
{
    auto config = config_create();
    if (config == nullptr)
        return nullptr;

    std::shared_ptr<Project> project = nullptr;
    if (config_load(config, path.c_str()) != 0)
    {
        auto section = config_find_section(config, "project");
        if (section != nullptr)
        {
            auto name = config_get_string(section, "name");
            auto path = config_get_string(section, "path");
            auto version = config_get_string(section, "version");

            project = std::make_shared<Project>();
            project->name = name;
            project->path = path;
            project->version = version;
        }
    }
    config_release(config);

    return project;
}

ProjectMenu::ProjectMenu() : m_engine(static_cast<EditorApplication*>(Application::application)), popup_flags(0), m_mainViewport(nullptr), m_style(nullptr), m_io(nullptr)
{
    m_mainViewport = ImGui::GetMainViewport();

    m_style = &ImGui::GetStyle();

    m_io = &ImGui::GetIO();

    read_project_directory();
}

ProjectMenu::~ProjectMenu()
{
    // DebugDraw::Shutdown();
}

const char* ProjectMenu::getName() const
{
    return "ProjectMenu";
}

void ProjectMenu::onRenderUI()
{
    create_window_project_list();

    switch (popup_flags)
    {
        case ProjectMenuPopup_CreateProject:
            ImGui::OpenPopup("##popup_create_new_project");
            break;
        default:
            break;
    }

    popup_create_new_project();

    popup_flags = 0;
}

// ********** private ********** //
void ProjectMenu::create_window_project_list()
{
    ImGui::SetNextWindowPos(ImVec2{0.0f, 0.0f}, ImGuiCond_Always);
    ImGui::SetNextWindowSize(m_mainViewport->Size, ImGuiCond_Always);
    if (ImGui::Begin("##create_window_project_list", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar))
        render_project_list();

    ImGui::End();
}

void ProjectMenu::render_project_list()
{
    auto avail = ImGui::GetContentRegionAvail();
    if (ImGui::Button("Create New Project", ImVec2{avail.x, 0.0f}))
        popup_flags |= ProjectMenuPopup_CreateProject;

    for (auto &project : m_projects)
    {
        ImGui::Selectable(project.name.c_str());
            // MainThreadDispatcher::Add(Priority::LOW, &Engine::init_project, m_engine, project.path + "/metadata.ini");
    }
}

void ProjectMenu::popup_create_new_project()
{
    ImVec2 window_size = {300.0f, 400.0f};

    ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);
    ImGui::SetNextWindowPos((m_mainViewport->Size / 2.0f) - (window_size / 2.0f));

    auto isOpen = ImGui::BeginPopupModal("##popup_create_new_project", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    if (!isOpen)
        return;

    ImGui::SeparatorText("Create Project");

    ImGui::NewLine();

    ImGui::Text("Name: ");
    ImGui::SameLine();
    
    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
        ImGui::SetKeyboardFocusHere(0);

    static char name[64] = {0};
    ImGui::InputText("##create_project_input_name", name, 64);

    ImGui::NewLine();

    auto avail = ImGui::GetContentRegionAvail();
    
    if (ImGui::Button("Create", ImVec2{avail.x, 0.0f}))
    {
        create_project(name);

        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

bool ProjectMenu::create_project(const std::string& name)
{
    std::string path = gfile::get_home_directory() + "/" + name;
    if (gfile::exists(path))
    {
        std::cout << "The project: " << name << " exists, in: " << path << std::endl;
        return false;
    }
    
    if (!gfile::create_directory(path))
    {
        std::cout << "Unable to create project in directory: " << path << std::endl;
        return false;
    }

    auto& project = m_projects.emplace_back();
    project.name = name;
    project.path = path;
    project.version = "0.1.0";

    if (!create_project_metadata())
    {
        gfile::remove(path);

        m_projects.pop_back();

        std::cout << "[ Project ] Fail to create project metadada." << std::endl;
        return false;
    }
    
    CreateProjectAssets(path);

    return true;
}

bool ProjectMenu::create_project_metadata()
{
    auto config = config_create();
    if (config == nullptr)
        return false;

    auto& project = m_projects.back();

    {
        auto section = config_create_section(config, "project");
        config_set_string(section, "name", project.name.c_str());
        config_set_string(section, "path", project.path.c_str());
        config_set_string(section, "version", project.version.c_str());
    }

    config_save(config, (project.path + "/metadata.ini").c_str());

    config_release(config);

    return true;
}

void ProjectMenu::CreateProjectAssets(const std::string &path)
{
    if (!gfile::exists(path + "/assets"))
        gfile::create_directory(path + "/assets");

    for (auto &dir : m_assetsFile)
    {
        auto fullPath = path + "/assets/" + dir;

        if (!gfile::exists(fullPath))
            gfile::create_directory(fullPath);
    }

    // std::string internal_asset_path = gfile::get_data_directory() + "/assets";

    // gfile::copy((internal_asset_path + "/shader").c_str(), (path + "/assets").c_str());
}

void ProjectMenu::read_project_directory()
{
    auto dir = gfile::get_home_directory() + "/gr-engine";
    if (!gfile::exists(dir))
        return;

    for (auto &entry : std::filesystem::directory_iterator(dir))
    {
        if (!entry.is_directory() || !gfile::exists(entry.path() / "metadata.ini"))
            continue;

        auto config = config_create();
        if (config == nullptr)
            continue;

        config_load(config, (entry.path() / "metadata.ini").c_str());
        {
            auto section = config_find_section(config, "project");
            if (section != nullptr)
            {
                auto name = config_get_string(section, "name");
                auto path = config_get_string(section, "path");
                auto version = config_get_string(section, "version");

                auto& project = m_projects.emplace_back();
                project.name = name;
                project.path = path;
                project.version = version;
            }
        }
        config_release(config);
    }
}

bool ProjectMenu::ReadLastProject()
{
    /*
    auto path = File::GetFile("/config.ini", true);
    if (!gfile::exists(path))
        return false;

    Config config(path);
    if (!config.is_open())
        return false;

    config.load();

    if (!config.hasKey("name") || !config.hasKey("path"))
        return false;

    path = config.getValue<std::string>("path");

    if (!gfile::exists(path))
        return false;
    */
    return false;
}

// uninstall engine
// rm -rf ~/.cache/gr-engine ~/.config/gr-engine ~/.local/share/gr-engine
