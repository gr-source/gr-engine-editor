#include "core/sandbox.hpp"

#include "gr-base/core/logger.hpp"
#include "gr-base/ecs/componentRegistry.hpp"
#include "gr-base/ecs/systemRegistry.hpp"
#include "gr-base/module/moduleloader.hpp"

#include <gr-base/file/file.hpp>

#include <dlfcn.h>
#include <gr-base/ecs/typeinfo.hpp>

#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

#include <iostream>
#include <vector>

Sandbox::Sandbox(const std::string& path) : m_path(path), m_pid(-1)
{}

Sandbox::~Sandbox()
{
    ClosePipes();
}

bool Sandbox::CreatePipes()
{
    if (pipe(m_logPipe) == -1)
        return false;

    if (pipe(m_dataPipe) == -1)
    {
        close(m_logPipe[0]);
        close(m_logPipe[1]);

        return false;
    }

    return true;
}

void Sandbox::ClosePipes()
{
    for (int& fd : m_logPipe)
    {
        if (fd != -1)
        {
            close(fd);
            fd = -1;
        }
    }

    for (int& fd : m_dataPipe)
    {
        if (fd != -1)
        {
            close(fd);
            fd = -1;
        }
    }
}

bool Sandbox::Run()
{
    if (!CreatePipes())
        return false;

    if (!LaunchChild())
        return false;

    ClosePipes();

    return true;
}

bool Sandbox::LaunchChild()
{
    m_pid = fork();

    if (m_pid < 0)
        return false;

    if (m_pid == 0)
    {
        ChildMain();
        _exit(0);
    }

    return WaitProcess();
}

void Sandbox::ChildMain()
{
    close(m_logPipe[0]);
    close(m_dataPipe[0]);

    dup2(m_logPipe[1], STDOUT_FILENO);
    dup2(m_logPipe[1], STDERR_FILENO);

    // ConfigureLimits();

    LoadPlugin(m_dataPipe[1]);

    close(m_logPipe[1]);
    close(m_dataPipe[1]);
}

bool Sandbox::WaitProcess()
{
    close(m_logPipe[1]);
    close(m_dataPipe[1]);
    
    size_t count = 0;
    read(m_dataPipe[0], &count, sizeof(size_t));
    systems.resize(count, {});

    read(m_dataPipe[0], systems.data(), count * sizeof(SystemInfo));

    read(m_dataPipe[0], &count, sizeof(size_t));
    components.resize(count, {});

    read(m_dataPipe[0], components.data(), count * sizeof(ComponentInfo));

    close(m_logPipe[0]);
    close(m_dataPipe[0]);

    int status;
    waitpid(m_pid, &status, 0);

    if (WIFSIGNALED(status))
    {
        int sig = WTERMSIG(status);

        std::cout << "[ Sandbox ] Falha ao testar o modulo\n";
        std::cout << "[ Sandbox ] Sinal recebido: " << sig << "\n";

        switch (sig)
        {
            case SIGSEGV:
                std::cout << "Erro: Segmentation Fault (acesso invalido a memoria)\n";
                break;

            case SIGFPE:
                std::cout << "Erro: Floating Point Exception (ex: divisao por zero)\n";
                break;

            case SIGABRT:
                std::cout << "Erro: Abort (assert ou std::terminate)\n";
                break;

            case SIGBUS:
                std::cout << "Erro: Bus error (memoria desalinhada ou invalida)\n";
                break;

            case SIGILL:
                std::cout << "Erro: Illegal instruction\n";
                break;

            default:
                std::cout << "Erro: outro sinal desconhecido\n";
                break;
        }

        return false;
    } else
    {
        int code = WEXITSTATUS(status);

        printf("[ Sandbox ] Modulo carregado e testado com sucesso.\n");
        printf("[ Sandbox ] Codigo de saida: '%d'.\n", code);
    }

    return true;
}

void Sandbox::LoadPlugin(int fd)
{
    ModuleLoader::Unload("script");

    ModuleLoader::Shutdown();
    ComponentRegistry::Shutdown();
    SystemRegistry::Shutdown();

    logger::Shutdown();

    // aqui você carrega o módulo
    
    logger_config config;
    config.path = gfile::get_config_directory() + "/logs/sanbox";

    logger::Initialize(&config);

    logger::info("[ Sandbox ] Running...");

    ComponentRegistry::Initialize();
    SystemRegistry::Initialize();
    // ModuleLoader::Initialize();

    void* handle = dlopen(m_path.c_str(), RTLD_LAZY);
    if (handle == nullptr)
    {
        logger::error("[ Sandbox ] Falha ao carregar o modulo.");
    } else
    {
        const auto systems = SystemRegistry::GetSystems();
        auto systemCount = SystemRegistry::GetCount();

        write(fd, &systemCount, sizeof(size_t));
        write(fd, systems, systemCount * sizeof(SystemInfo));

        const auto componentes = ComponentRegistry::GetComponents();
        auto componentCount = ComponentRegistry::GetComponentCount();

        write(fd, &componentCount, sizeof(size_t));
        write(fd, componentes, componentCount * sizeof(ComponentInfo));

        dlclose(handle);
    }

    ModuleLoader::Shutdown();
    ComponentRegistry::Shutdown();
    SystemRegistry::Shutdown();

    logger::info("[ Sandbox ] Satoped.");

    logger::Shutdown();
}


