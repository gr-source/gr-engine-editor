#pragma once

#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#include <imgui_internal.h>
#include <imgui.h>

class ImGuiRender {
public:
    static void Initialize();
    
    static void Shutdown();

    static void PushStyle(ImGuiStyle& dst);

    static void SetFont(const char* path);

    static void SetConfigPath(const char* path);

    static void Begin();

    static void End();

    static ImGuiIO *GetIO();

protected:
    static ImGuiIO *m_io;
};


