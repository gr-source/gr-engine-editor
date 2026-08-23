#pragma once

#include "view/iview.hpp"

#include <vector>
#include <string>

class IWorkspace : public IView
{
public:
    IWorkspace();

    virtual void Initialize() {}

    virtual void Shutdown() {}

    virtual void DrawWorkspace() = 0;

    virtual void Update() {}

    virtual void dropPath(const std::vector<std::string>& entries) {}

    void Render() override;

    virtual void Render3D() {}
};


