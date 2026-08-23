#pragma once

#include "gr-base/core/ievent.hpp"
#include <string>

struct EditorApplicationEvent_onOpenProject : IEvent
{
    std::string path;
};

struct EditorApplicationEvent_onCloseProject : IEvent
{
};

