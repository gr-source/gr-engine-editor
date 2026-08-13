#include "editorApplication.hpp"

#include "event/editorApplicationEvent.hpp"
#include "gr-base/core/scene.hpp"
#include "gr-base/managers/eventManager.hpp"

#include "view/workspace/projectBrowserWorkspace.hpp"
#include "view/workspace/editorWorkspace.hpp"

#include "view/imguiRenderer.hpp"

#include "view/projectMenu.hpp"

#include <gr-base/file/file.hpp>

#include <gr-render/gRender.h>

EditorApplication::EditorApplication() : m_project(nullptr), config(nullptr), m_workspace(nullptr)
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

    loadConfig();

    EventManager::Attach<EditorApplicationEvent_onCheckProject>(
            &EditorApplication::onCheckProject,
            this
        );
    EventManager::Attach<EditorApplicationEvent_onOpenProject>(
            &EditorApplication::onOpenProject,
            this
        );
    EventManager::Attach<EditorApplicationEvent_onCloseProject>(
            &EditorApplication::onCloseProject,
            this
        );

    m_workspace = std::make_unique<ProjectBrowser>();

    cameraUBO.Create(0, sizeof(CameraBuffer));
    lightUBO.Create(1, sizeof(LightBuffer));
}

void EditorApplication::shutdown()
{
    m_workspace = nullptr;

    ImguiRenderer::Shutdown();

    cameraUBO.Destroy();
    lightUBO.Destroy();

    runtime.shutdown();

    saveConfig();
}

#include <gr-base/managers/sceneManager.hpp>

void EditorApplication::run()
{
    beginFrame();

    update();

    renderer();

    endFrame();
}

void EditorApplication::dropPath(const std::vector<std::string>& entries)
{
    if (m_workspace != nullptr)
        m_workspace->dropPath(entries);
}

#include <gr-base/profiler.hpp>

// private
void EditorApplication::beginFrame()
{
    // std::cout << "[ ===== BeginFrame ===== ]\n";

    Profiler::BeginFrame();

    auto scene = Scene::GetCurrent();
    if (scene != nullptr)
        scene->BeginFrame();
}

void EditorApplication::endFrame()
{
    // std::cout << "[ ===== EndFrame ===== ]\n";

    auto scene = Scene::GetCurrent();
    if (scene != nullptr)
        scene->EndFrame();

    Profiler::EndFrame();
}

void EditorApplication::update()
{
    runtime.update();

    if (m_workspace != nullptr)
        m_workspace->Update();
}

#include <gr-base/core/renderer.hpp>

void EditorApplication::renderer()
{
    auto scene = Scene::GetCurrent();
    if (scene != nullptr)
        drawScene(scene);

    gr::gRender::SetRenderState(gr::GR_CULL, gr::GR_BACK);
    gr::gRender::SetEnable(GR_DEPTH, false);
    gr::gRender::SetEnable(GR_CULL_FACE, true);
    gr::gRender::SetRenderState(gr::GR_BACKGROUND, gr::GR_COLOR_BUFFER);
    gr::gRender::SetBackgroundColor(Color{0.5f, 0.5f, 0.5f, 1.0f}.data);
    
    ImguiRenderer::Begin();

    if (m_workspace != nullptr)
        m_workspace->Render();

    ImguiRenderer::End();
}

#include <gr-base/core/culling/frustum.hpp>

#include <gr-base/ecs/world.hpp>

#include <gr-base/script/transform.hpp>
#include <gr-base/script/camera.hpp>
#include <gr-base/script/light.hpp>

void EditorApplication::drawScene(Scene* scene)
{
    LightBuffer lightBuffer{0};

    scene->world->Query<TransformLocal, Light>([&](
                size_t count,
                Entity*,
                TransformLocal* transforms,
                Light* lights)
    {
        for (size_t i=0;i<count;i++)
        {
            if ((lightBuffer.numLights + 1) >= MAX_NUM_LIGHT)
                break;

            auto& transform = transforms[i];
            auto& light = lights[i];

            LightData& data = lightBuffer.lights[lightBuffer.numLights++];
            *(Vector3*)(&data.position[0]) = transform.position;
            *(Vector3*)(&data.direction[0]) = transform.rotation * type_traits<Vector3>::forward;

            data.color = light.color;
            data.params = {
                light.attenuationConstant,
                light.attenuationLinear,
                light.attenuationQuadratic,
                static_cast<float>(light.type)
            };

            data.spot = {
                cosf(Math::rad(light.angleInnerCone)),
                cosf(Math::rad(light.angleOuterCone))
            };
        }
    });

    Renderer::BeginFrame();

    scene->world->Query<TransformLocal, Camera, CameraData>(
            [&](
                size_t count,
                Entity*,
                TransformLocal* transformLocals,
                Camera* cameras,
                CameraData* cameraDatas)
            {
                for (size_t i=0;i<count;i++)
                {
                    auto& transformLocal = transformLocals[i];
                    auto& cameraData = cameraDatas[i];
                    auto& camera = cameras[i];

                    CameraBuffer cameraBuffer;
                    cameraBuffer.projection =
                        cameraData.projection;
                    cameraBuffer.view =
                        cameraData.view;
                    *(Vector3*)(&cameraBuffer.position[0]) =
                        transformLocal.position;


                    cameraUBO.Update(cameraBuffer);
                    lightUBO.Update(lightBuffer);

                    Renderer::SetRenderTarget(
                        camera.rendertarget
                    );

                    Renderer::SetViewport(camera.viewport);

                    Frustum::CalculateFrustum(cameraData.projection * cameraData.view);

                    Renderer::Render();
                }
            }
        );

    Renderer::EndFrame();
}

// #include <gr-base/core/assetProvider.hpp>
#include <gr-base/core/assetResolver.hpp>

class ProjectAssetProvider : public IAssetProvider
{
    std::string m_directory;
public:
    ProjectAssetProvider(const std::string& dir) : m_directory(dir)
    {}

    virtual std::string resolve(const std::string& path) override
    {
        return m_directory + "/assets/" + path;
    }
};

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
        auto section =
            config_find_section(config, "project");
        if (section == nullptr)
            section = config_create_section(config, "project");
        config_set_string(section, "path", (m_project->path + "/metadata.ini").c_str());
    }

    config_save(config, (gfile::get_config_directory() + "/config.ini").c_str());
}

void EditorApplication::onCheckProject(IEvent* event)
{
    gSection* section = config_find_section(config, "project");
    if (section != NULL)
    {
        const char* path = config_get_string(section, "path");
        if (path != NULL && gfile::exists(path))
            m_project = ProjectMenu::Load(path);
    }

    if (m_project == nullptr)
        return;

    AssetResolver::Registry<ProjectAssetProvider>(Scheme::Project, m_project->path);

    m_workspace = std::make_unique<EditorWorkspace>();
}

void EditorApplication::onOpenProject(EditorApplicationEvent_onOpenProject* event)
{
    std::cout << "Open: " << event->dir << std::endl;
}

void EditorApplication::onCloseProject(IEvent* event)
{
    m_workspace = nullptr; // = std::make_unique<ProjectBrowser>();

    m_project = nullptr;
}


