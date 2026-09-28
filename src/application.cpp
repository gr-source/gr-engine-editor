#include "application.hpp"

#include "event/editorApplicationEvent.hpp"
#include "gr-base/core/logger.hpp"
#include "gr-base/core/scene.hpp"
#include "gr-base/managers/eventManager.hpp"

#include "manager/projectManager.hpp"
#include "view/workspace/editorWorkspace.hpp"
#include "view/workspace/mainWorkspace.hpp"

#include "view/imguiRenderer.hpp"

#include <gr-base/file/file.hpp>

#include <gr-render/gRender.h>

EditorApplication::EditorApplication() :
    config(nullptr)
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

    imgui.initialize();

    loadConfig();

    EventManager::Subscribe<EditorApplicationEvent_onOpenProject>(
            &EditorApplication::onOpenProject,
            this
        );
    EventManager::Subscribe<EditorApplicationEvent_onCloseProject>(
            &EditorApplication::onCloseProject,
            this
        );

    cameraUBO = std::make_unique<engine::renderer::UniformBuffer>(sizeof(CameraBuffer)),
    lightUBO = std::make_unique<engine::renderer::UniformBuffer>(sizeof(LightBuffer));

    // cameraUBO.Create(0, sizeof(CameraBuffer));
    // lightUBO.Create(1, sizeof(LightBuffer));

    MakeWorkspace<MainWorkspace>();
}

void EditorApplication::shutdown()
{
    if (gworkspace != nullptr)
        gworkspace->Shutdown();

    gworkspace = nullptr;

    imgui.shutdown();

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
    if (gworkspace != nullptr)
        gworkspace->dropPath(entries);
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

    if (gworkspace != nullptr)
        gworkspace->Update();
}

#include <gr-base/core/renderer.hpp>

void EditorApplication::renderer()
{
    cameraUBO->attach(0);
    lightUBO->attach(1);

    auto scene = Scene::GetCurrent();
    if (scene != nullptr)
        drawScene(scene);

    gr::gRender::SetRenderState(gr::GR_CULL, gr::GR_BACK);
    gr::gRender::SetEnable(GR_DEPTH, false);
    gr::gRender::SetEnable(GR_CULL_FACE, true);
    gr::gRender::SetRenderState(gr::GR_BACKGROUND, gr::GR_COLOR_BUFFER);
    gr::gRender::SetBackgroundColor(Color::Black());
    
    imgui.begin();

    if (gworkspace != nullptr)
        gworkspace->Render();

    imgui.end();
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

    cameraUBO->Update(cameraBuffer);
    lightUBO->Update(lightBuffer);

    Frustum::CalculateFrustum(cameraData->projection * cameraData->view);

    Renderer::BeginCamera();

    if (gworkspace != nullptr)
        gworkspace->Render3D();

    CameraRenderData cam;
    cam.viewport = camera->viewport;

    auto pipeline = RenderPipelineManager::Get<IPipeline>(camera->pipeline);
    if (pipeline != nullptr)
    {
        /*
        ScopeTimer scope;

        scope.start();
        */

        pipeline->Render(cam);

        /*
        double ms =
            scope.end();

        Profiler::Add("[ Renderer ] Renderer", static_cast<float>(ms));
        */
    }

    Renderer::EndCamera();
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

    if (ProjectManager::GetInstance().Has())
    {
        auto project = ProjectManager::GetInstance().GetCurrent();

        auto section =
            config_find_section(config, "project");
        if (section == nullptr)
            section = config_create_section(config, "project");
        config_set_string(section, "path", (project->path + "/metadata.ini").c_str());
    }

    config_save(config, (gfile::get_config_directory() + "/config.ini").c_str());
}

void EditorApplication::onOpenProject(EditorApplicationEvent_onOpenProject& event)
{
    if (!ProjectManager::GetInstance().Open(event.path))
        return;

    if (config == NULL)
        return;

    auto project = ProjectManager::GetInstance().GetCurrent();
    if (project != nullptr)
    {
        auto section =
            config_find_section(config, "project");
        if (section == nullptr)
            section = config_create_section(config, "project");
        config_set_string(section, "path", (project->path + "/metadata.ini").c_str());
    }

    config_save(config, (gfile::get_config_directory() + "/config.ini").c_str());

    MakeWorkspace<EditorWorkspace>();
}

void EditorApplication::onCloseProject(EditorApplicationEvent_onCloseProject& event)
{
    MakeWorkspace<MainWorkspace>();

    ProjectManager::GetInstance().Close();
}

void EditorApplication::SetWorkspace(std::unique_ptr<IWorkspace> workspace)
{
    try
    {
        workspace->Initialize();
    } catch (const std::exception& e)
    {
        logger::critical("[ EditorApplication ] %s", e.what());
        return;
    }

    gworkspace = std::move(workspace);
}

