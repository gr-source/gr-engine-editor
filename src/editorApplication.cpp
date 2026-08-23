#include "editorApplication.hpp"

#include "event/editorApplicationEvent.hpp"
#include "gr-base/core/assetDB.hpp"
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
#include <gr-base/render/pipeline/forwardPipeline.hpp>

#include <gr-base/managers/renderPipelineManager.hpp>

void EditorApplication::initialize()
{
    runtime.initialize();

    ImguiRenderer::Initialize();

    loadConfig();

    EventManager::Subscribe<EditorApplicationEvent_onCheckProject>(
            &EditorApplication::onCheckProject,
            this
        );
    EventManager::Subscribe<EditorApplicationEvent_onOpenProject>(
            &EditorApplication::onOpenProject,
            this
        );
    EventManager::Subscribe<EditorApplicationEvent_onCloseProject>(
            &EditorApplication::onCloseProject,
            this
        );

    cameraUBO.Create(0, sizeof(CameraBuffer));
    lightUBO.Create(1, sizeof(LightBuffer));

    RenderPipelineManager::Register<ForwardPipeline>(RenderPipelineForward);

    m_workspace = std::make_unique<ProjectBrowser>();
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

#include <gr-base/scopeTimer.hpp>

struct PrintFrame
{
    std::string name;
    ScopeTimer scope;

    PrintFrame(const std::string& name) : name(name)
    {
        scope.start();
    }

    ~PrintFrame()
    {
        auto ms = scope.end();

        std::cout << name << ": " << ms << " ms\n";
    }
};

void EditorApplication::run()
{
    beginFrame();

    {
        // PrintFrame p("Update");
        update();
    }

    {
        // PrintFrame p("render");
        renderer();
    }

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
    gr::gRender::SetBackgroundColor(Color::Black());
    
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
    const GameObject go(scene, scene->GetCurrentCamera());
    if (!go.IsAlive())
        return;

    auto&& [transform, camera, cameraData] =
        scene->world.GetComponents<TransformLocal, Camera, CameraData>(scene->GetCurrentCamera());

    CameraBuffer cameraBuffer;
    cameraBuffer.projection = cameraData->projection;
    cameraBuffer.view = cameraData->view;
    *(Vector3*)(&cameraBuffer.position[0]) = transform->position;

    LightBuffer lightBuffer{0};

    scene->world.Query<TransformLocal, Light>([&](
                size_t count,
                Entity*,
                TransformLocal* transforms,
                Light* lights)
    {
        for (size_t i=0;i<count;i++)
        {
            if (lightBuffer.numLights >= MAX_NUM_LIGHT)
                break;

            auto& transform = transforms[i];
            auto& light = lights[i];

            LightData& data = lightBuffer.lights[lightBuffer.numLights++];
            *(Vector3*)(&data.position[0]) = transform.position;
            *(Vector3*)(&data.direction[0]) = transform.rotation * type_traits<Vector3>::forward;
            // data.direction = transform.rotation * type_traits<Vector3>::forward;

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

    cameraUBO.Update(cameraBuffer);
    lightUBO.Update(lightBuffer);

    Frustum::CalculateFrustum(cameraData->projection * cameraData->view);

    Renderer::BeginFrame();

    if (m_workspace != nullptr)
        m_workspace->Render3D();

    CameraRenderData cam;

    auto pipeline = RenderPipelineManager::Get<IPipeline>(camera->pipeline);
    if (pipeline != nullptr)
        pipeline->Render(cam);

    // Renderer::Render();

    Renderer::EndFrame();
}

/*
void EditorApplication::drawScene(Scene* scene)
{
    LightBuffer lightBuffer{0};

    scene->world.Query<TransformLocal, Light>([&](
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

    lightUBO.Update(lightBuffer);

    Renderer::BeginFrame();

    scene->world.Query<TransformLocal, Camera, CameraData>(
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

                    Frustum::CalculateFrustum(cameraData.projection * cameraData.view);

                    auto pipeline = RenderPipelineManager::Get<IRenderPipeline>(camera.pipeline);

                    pipeline->Execute();
                }
            }
        );

    Renderer::EndFrame();
}
*/

// #include <gr-base/core/assetProvider.hpp>
#include <gr-base/core/assetResolver.hpp>

class ProjectAssetProvider : public IAssetProvider
{
    std::string m_directory;
public:
    ProjectAssetProvider(const std::string& dir) : m_directory(dir)
    {}

    std::string resolve(const std::string& path) override
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

void EditorApplication::onCheckProject(EditorApplicationEvent_onCheckProject& event)
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

    AssetResolver::Registry<ProjectAssetProvider>(Scheme_Project, m_project->path);

    m_workspace = std::make_unique<EditorWorkspace>();
}

void EditorApplication::onOpenProject(EditorApplicationEvent_onOpenProject& event)
{
    m_project = ProjectMenu::Load(event.path + "/metadata.ini");
    if (m_project == nullptr)
        return;

    AssetResolver::Registry<ProjectAssetProvider>(Scheme_Project, m_project->path);

    m_workspace = std::make_unique<EditorWorkspace>();
}

void EditorApplication::onCloseProject(EditorApplicationEvent_onCloseProject& event)
{
    m_workspace = nullptr; // = std::make_unique<ProjectBrowser>();

    m_project = nullptr;
}


