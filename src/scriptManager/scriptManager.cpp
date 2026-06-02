#include "scriptManager/scriptManager.hpp"

#include <cstdio>

#include <gr-base/core/console.hpp>
#include <gr-base/core/scene.hpp>

#include <gr-base/core/file/file.hpp>

#include <dlfcn.h>
#include <stdexcept>

void ScriptManager::Load(const std::string &path)
{
    return GetInstance().load(path);
}

void ScriptManager::Unload()
{
    return GetInstance().unload();
}

#include <string.h>
bool ScriptManager::CheckScriptHasInMemory(const char* name)
{
    FILE* fp = fopen("/proc/self/maps", "r");
    if (fp == NULL)
    {
        perror(__FUNCTION__);
        return false;
    }

    bool found = false;
    
    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        if (strstr(line, name) == NULL)
            continue;

        found = true;
        break;
    }
    fclose(fp);
    return found;
}

// private
ScriptManager::ScriptManager() : m_script(nullptr)
{}

ScriptManager::~ScriptManager()
{}

#include <sys/wait.h>
#include <unistd.h>
void ScriptManager::load(const std::string &path)
{
    if (!gfile::exists(path))
    {
        return Console::LogError("[ ScriptManager ] File do not found: %s", path.c_str());
    }
    /*
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        return Console::LogError("[ ScriptManager ] Fail to create call!");
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        return Console::LogError("[ ScriptManager ] Fail to create virtual box");
    } else if (pid == 0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        Script script(path);
        if (!script.isValid())
        {
            std::cerr << "Erro dlopen: " << dlerror() << "\n";
            _exit(1);
        } else
        {
            test();

            _exit(0);
        }
    } else {
        close(pipefd[1]);
        
        std::vector<std::string> data;
        char temp[256];
        ssize_t count;
        while ((count = read(pipefd[0], temp, sizeof(temp)-1)) > 0) {
            temp[count] = '\0';

            data.emplace_back(temp);
        }
        close(pipefd[0]);

        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            if (code != 0)
            {
                Console::LogError("[ ScriptManager ] Fail to load!!!");
                for (auto &line : data)
                    Console::LogError("[ ScriptManager ] %s", line.c_str());

                return;
            }
        } else
        {
            Console::LogError("[ ScriptManager ] Virtual box segmentafult detected!");

            for (auto &line : data)
                Console::LogError("[ ScriptManager ] %s", line.c_str());
            return;
        }
    }
    */

    m_script = std::make_unique<Script>(path);

    if (!m_script->isValid()) {
        throw std::runtime_error("Script do not is valid: " + path);
    }
}

void ScriptManager::unload()
{
    m_script = nullptr;
}

void ScriptManager::test()
{
    /*
    auto &componentRegistry = ComponentRegistry::getInstance();
    for (auto &&[name, it] :componentRegistry.GetAllChunkComponent())
    {
        auto &&[base, b] = it;
        // auto component = base->allocateComponent(nullptr, nullptr);

        // base->deallocate(component);
    }
    */
}

// protected
ScriptManager &ScriptManager::GetInstance()
{
    static ScriptManager instance;
    return instance;
}

