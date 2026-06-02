#include "plugin/pluginManager.hpp"

#include <gr-base/core/file/file.hpp>

#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <dlfcn.h>

std::unordered_map<std::string, std::unique_ptr<Plugin>> PluginManager::pluginMap; 

std::unordered_map<Plugin *, std::string> PluginManager::pluginPathMap;

std::unordered_map<std::string, Plugin *> pluginPath;

std::vector<Plugin *> PluginManager::pluginList;

void PluginManager::Load(const std::string &path)
{
    auto it = pluginMap.find(path);
    if (it != pluginMap.end())
        throw std::runtime_error("plugin has be loaded.");

    if (!gfile::exists(path))
        throw std::runtime_error("plugin do not exists.");

    auto handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        throw std::runtime_error(strerror(errno));
    }
    
    std::unique_ptr<Plugin> plugin = std::make_unique<Plugin>(handle);
    
    plugin->getPluginInfo = ((PluginInfo (*)(void)) dlsym(handle, "CreatePluginInfo"));
    if (!plugin->getPluginInfo) {
        dlclose(handle);
        throw std::runtime_error("Fail to link plugininfo.");
    }

    plugin->initialize = ((void(*)(void)) dlsym(handle, "Initialize"));
    if (!plugin->initialize) {
        dlclose(handle);
        throw std::runtime_error("Fail to link initialize.");
    }

    plugin->renderGUI = ((void(*)(void)) dlsym(handle, "Render"));
    if (!plugin->renderGUI) {
        dlclose(handle);
        throw std::runtime_error("Fail to link render.");
    }
    
    plugin->shutdown = ((void(*)(void)) dlsym(handle, "Shutdown"));
    if (!plugin->shutdown) {
        dlclose(handle);
        throw std::runtime_error("Fail to link shutdown.");
    }
    plugin->initialize();
    
    pluginMap[path] = std::move(plugin);

    pluginList.push_back(&(*pluginMap[path]));

    pluginPathMap[&(*pluginMap[path])] = path;
}

std::unordered_map<Plugin *, std::string> PluginManager::GetPluginFile() {
    return pluginPathMap;
}

std::vector<Plugin *> PluginManager::getPluginList() {
    return pluginList;
}

void PluginManager::Remove(Plugin *plugin) {
    auto it = pluginPathMap.find(plugin);
    if (it == pluginPathMap.end()) {
        return;
    }
    plugin->shutdown();

    dlclose(plugin->getPointer());
    
    pluginList.erase(std::remove(pluginList.begin(), pluginList.end(), plugin), pluginList.end());

    pluginMap.erase(it->second);

    pluginPathMap.erase(it);
}

void PluginManager::Render() {
    for (auto &plugin : PluginManager::getPluginList()) {
        plugin->renderGUI();
    }
}

void PluginManager::Shutdown() {
    for (auto &plugin : pluginList) {
        plugin->shutdown();

        dlclose(plugin->getPointer());
    }
    pluginList.clear();
}
