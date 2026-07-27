#pragma once

#include "view/iview.hpp"

#include <vector>
#include <string>

class IWorkspace : public IView
{
public:
    IWorkspace();

    virtual const char* GetWorkspaceID() const = 0;

    virtual void DrawWorkspace() = 0;

    virtual void Update() {}

    virtual void dropPath(const std::vector<std::string>& entries) {}

    void Render() override;
};


