#pragma once

#include "plugin.hpp"

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

class PluginManager {
public:
    static void Load(const std::string &path);
    
    static std::unordered_map<Plugin *, std::string> GetPluginFile();

    static std::vector<Plugin *> getPluginList(); 

    static void Remove(Plugin *plugin);

    static void Render();
    
    static void Shutdown();

private:
    static std::unordered_map<std::string, std::unique_ptr<Plugin>> pluginMap; 

    static std::unordered_map<Plugin *, std::string> pluginPathMap;

    static std::vector<Plugin *> pluginList; 
};