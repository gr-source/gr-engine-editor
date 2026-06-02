#pragma once

#include <unordered_map>
#include <vector>
#include <string>

class Script {
public:
    Script(const std::string &path);
    ~Script();

    bool setRegistry(const std::string &name);

    bool isValid() const;

    void replaceLibraryField(const std::string &name, void *ptr);

    bool registerFunction(const std::string &name, void **ptr);

    bool replaceLibraryFunction(const std::string &name, void *ptr);

private:
    std::unordered_map<std::string, void **> (*GetFunctionsReference)();

    std::vector<void **> m_funcMap;
    
    void *m_script;

    void *findSymbol(const std::string &name);
};
