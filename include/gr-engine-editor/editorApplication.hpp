#pragma once

#include "core/project.hpp"

#include "event/editorApplicationEvent.hpp"

#include "view/imguiRenderer.hpp"
#include "view/workspace/iworkspace.hpp"

#include <gr-base/core/default.hpp>

#include <gr-base/iapplication.hpp>
#include <gr-base/runtime.hpp>

#include <memory>

extern "C"
{
#include <gr-ini/config.h>
}

#include <gr-render/uniformBuffer.hpp>
#include <gr-render/color.hpp>

#include <gr-math/gmath.hpp>

struct __attribute__((aligned(16))) LightData
{
    Vector4 position;
    Vector4 direction;

    Color color;

    Vector4 params;     // x = constant
                           // y = linear
                           // z = quadratic
                           // w = tipo

    Vector4 spot;       // x = cos(innerCone)
                           // y = cos(outerCone)
                           // z,w = reservados
};

typedef struct __attribute__((aligned(16))) CameraBuffer
{
    Matrix4x4 projection;
    Matrix4x4 view;
    Vector4 position;
} CameraBuffer;

#define MAX_NUM_LIGHT 10

typedef struct LightBuffer
{
    int numLights;
    LightData lights[MAX_NUM_LIGHT];
} LightBuffer;

class PostProcessPipeline;

class EditorApplication : public IApplication
{
public:
    EditorApplication();
    ~EditorApplication() override;

    std::shared_ptr<PostProcessPipeline> pipeline;

    void initialize() override;
    
    void shutdown() override;

    void run() override;

    void dropPath(const std::vector<std::string>& entries) override;

    inline Project* getProject() const
    {
        return m_project != nullptr ? m_project.get() : nullptr;
    }

private:
    std::unique_ptr<IWorkspace> gworkspace = nullptr;

    std::shared_ptr<Project> m_project;

    ImguiRenderer imgui;

    gConfig* config;

    Runtime runtime;

    std::unique_ptr<engine::renderer::UniformBuffer> cameraUBO;
    std::unique_ptr<engine::renderer::UniformBuffer> lightUBO;

    void beginFrame();

    void endFrame();

    void update();

    void renderer();

    void drawScene(Scene* scene);

    void loadConfig();

    void saveConfig();

    void onOpenProject(EditorApplicationEvent_onOpenProject& event);

    void onCloseProject(EditorApplicationEvent_onCloseProject& event);

    template <typename T, typename... Args>
    void MakeWorkspace(Args&&... args);

    void SetWorkspace(std::unique_ptr<IWorkspace> workspace);
};


template <typename T, typename... Args>
void EditorApplication::MakeWorkspace(Args&&... args)
{
    SetWorkspace(std::make_unique<T>(std::forward<Args>(args)...));
}


