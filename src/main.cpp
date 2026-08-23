#include "editorApplication.hpp"

extern "C"
{
__attribute__((visibility("default")))
IApplication* CreateInstance()
{
    return new EditorApplication();
}
}


