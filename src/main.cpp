#include "editorApplication.hpp"

extern "C"
{
__attribute__((visibility("default")))
Application* CreateInstance()
{
    return new EditorApplication();
}
}


