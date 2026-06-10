#pragma once

#include "gr-base/ecs/typeinfo.hpp"

#include <vector>

class Sandbox
{
public:
    std::vector<ComponentInfo> components;
    std::vector<SystemInfo> systems;

    Sandbox();
    ~Sandbox();

    bool CreatePipes();

    void ClosePipes();

    bool Run();

private:
    int m_dataPipe[2] = { -1, -1 };

    int m_logPipe[2] { -1, -1 };

    int m_pid;

    bool LaunchChild();

    void ChildMain();

    bool WaitProcess();

    void LoadPlugin(int fd);
};

