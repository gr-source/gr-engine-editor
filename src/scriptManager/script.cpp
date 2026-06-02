#include "scriptManager/script.hpp"

#include <gr-base/core/file/file.hpp>
#include <gr-base/core/console.hpp>

#include <dlfcn.h>

Script::Script(const std::string &path) : m_script(nullptr), GetFunctionsReference(nullptr)
{
    if (!gfile::exists(path))
    {
        Console::LogError("Falied to open script file do not found: " + path);
        return;
    }

    m_script = dlopen(path.c_str(), RTLD_LAZY);
    if (!m_script) {
        Console::LogError("Falied to load script: " + std::string(dlerror()));
        return;
    }

    Console::LogInfo("Script laoded sucess full.");
}

Script::~Script() {
    if (!m_script) {
        return;
    }
    for (auto &ptr : m_funcMap) {
        *ptr = nullptr;
    }
    m_funcMap.clear();

    if (dlclose(m_script) != 0) {
        Console::LogError("Falied to close script: " + std::string(dlerror()));
    }
    m_script = nullptr;
}

bool Script::setRegistry(const std::string &name) {
    return registerFunction(name, reinterpret_cast<void **>(&GetFunctionsReference));
}

bool Script::isValid() const {
    return m_script != nullptr;
}

void Script::replaceLibraryField(const std::string &name, void *ptr) {
    auto symbol = findSymbol(name);
    if (!symbol) {
        return;
    }
    symbol = ptr;
}

bool Script::registerFunction(const std::string &name, void **ptr) {
    auto symbol = findSymbol(name);
    if (!symbol) {
        return false;
    }
    *m_funcMap.emplace_back(ptr) = symbol;
    return true;
}

void *Script::findSymbol(const std::string &name) {
    auto ptr = dlsym(m_script, name.c_str());
    if (!ptr) {
        return nullptr;
    }
    return ptr;
}

bool Script::replaceLibraryFunction(const std::string &name, void *ptr) {
    if (GetFunctionsReference().find(name) == GetFunctionsReference().end()) {
        return false;
    }
    *GetFunctionsReference()[name] = ptr;
    return true;
}
