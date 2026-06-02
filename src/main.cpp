#include "editorApplication.hpp"

extern "C"
{
__attribute__((visibility("default")))
Application* CreateInstance()
{
    return new EditorApplication();
}
}

__attribute__((__constructor__))
void Initialize()
{}


__attribute__((destructor))
void Shutdown()
{}


