#include "editorApplication.hpp"

#include "manager/viewManager.hpp"

#include "view/imguiRenderer.hpp"
#include "view/projectMenu.hpp"
#include "view/checkupdate.hpp"
#include "view/editor.hpp"

#include <gr-base/core/file/file.hpp>

#include <gr-render/gRender.h>

EditorApplication::EditorApplication() : m_project(nullptr), config(nullptr)
{}

EditorApplication::~EditorApplication()
{
    if (config != nullptr)
        config_release(config);
}

void EditorApplication::initialize()
{
    runtime.initialize();

    ImguiRenderer::Initialize();

    ViewManager::Initialize();

    loadConfig();

    ViewManager::AddView<CheckUpdate>(&EditorApplication::checkProject, this);
}

void EditorApplication::shutdown()
{
    ViewManager::Shutdown();

    ImguiRenderer::Shutdown();

    runtime.shutdown();

    saveConfig();
}

void EditorApplication::run()
{
    runtime.process();

    update();

    renderer();
}

void EditorApplication::dropPath(const char* path)
{
    auto view = ViewManager::GetCurrentView();
    if (view != nullptr)
        view->dropPath(path);
}

// private
void EditorApplication::update()
{
    ViewManager::Update();
}

void EditorApplication::renderer()
{
    ViewManager::Renderer3D();

    gr::gRender::SetBackgroundColor(COLOR_BLUE);
    gr::gRender::SetRenderState(gr::GR_BACKGROUND, gr::GR_COLOR_BUFFER);
    
    ImguiRenderer::Begin();

    ViewManager::RendererUI();

    ImguiRenderer::End();
}

void EditorApplication::checkProject()
{
    ViewManager::RemoveAllView();


    gSection* section = config_find_section(config, "project");
    if (section != NULL)
    {
        const char* path = config_get_string(section, "path");
        if (path != NULL && gfile::exists(path))
            m_project = ProjectMenu::Load(path);
    }

    if (m_project == nullptr)
        return ViewManager::AddView<ProjectMenu>();

    ViewManager::AddView<Editor>();
    // ViewManager::AddView<ShaderEditor>();
}

void EditorApplication::loadConfig()
{
    config = config_create();
    if (config == NULL)
        return;

    std::string path = gfile::get_config_directory() + "/config.ini";
    if (!gfile::exists(path))
        return;

    config_load(config, path.c_str());
}

void EditorApplication::saveConfig()
{
    if (config == NULL)
        return;

    if (m_project != nullptr)
    {
        auto section = config_create_section(config, "project");
        config_set_string(section, "path", (m_project->path + "/metadata.ini").c_str());
    }

    config_save(config, (gfile::get_config_directory() + "/config.ini").c_str());
}


