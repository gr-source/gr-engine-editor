#include "plugin/plugin.hpp"

Plugin::Plugin(void *pointer) : pointer(pointer) { }

void *Plugin::getPointer() const {
    return pointer;
}
