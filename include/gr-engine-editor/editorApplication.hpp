#pragma once

#include "core/project.hpp"

#include "event/editorApplicationEvent.hpp"

#include "view/workspace/iworkspace.hpp"

#include <gr-base/core/default.hpp>

#include <gr-base/application.hpp>
#include <gr-base/runtime.hpp>

#include <memory>

extern "C"
{
#include <gr-ini/config.h>
}

#include <gr-render/uniformBuffer.hpp>

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

class EditorApplication : public Application
{
public:
    EditorApplication();
    ~EditorApplication() override;

    std::unique_ptr<IWorkspace> m_workspace;

    void initialize() override;
    
    void shutdown() override;

    void run() override;

    void dropPath(const std::vector<std::string>& entries) override;

    inline Project* getProject() const
    {
        return m_project != nullptr ? m_project.get() : nullptr;
    }

private:
    std::shared_ptr<Project> m_project;

    gConfig* config;

    Runtime runtime;

    engine::renderer::UniformBuffer cameraUBO;
    engine::renderer::UniformBuffer lightUBO;

    void beginFrame();

    void endFrame();

    void update();

    void renderer();

    void drawScene(Scene* scene);

    void loadConfig();

    void saveConfig();

    void onCheckProject(IEvent* event);

    void onOpenProject(EditorApplicationEvent_onOpenProject* event);

    void onCloseProject(IEvent* event);
};


