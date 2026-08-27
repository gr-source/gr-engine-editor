#pragma once

#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#include <imgui_internal.h>
#include <imgui.h>
#include <string>

class ImguiRenderer
{
public:
    ImguiRenderer();

    void initialize();
    
    void shutdown();

    void pushStyle(ImGuiStyle& dst);

    void setConfigPath(const std::string& path);

    void begin();

    void end();

protected:
    ImGuiIO *m_io =
        nullptr;
};


