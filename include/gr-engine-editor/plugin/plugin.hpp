#pragma once

#include "pluginInfo.hpp"

class Plugin {
public:
    Plugin() = delete;
    Plugin(void *pointer);

    void (*initialize)(void);

    void (*renderGUI)(void);

    void (*shutdown)(void);

    PluginInfo (*getPluginInfo)(void);

    void *getPointer() const;
    
private:
    void *pointer;
};

