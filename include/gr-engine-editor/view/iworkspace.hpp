#pragma once

#include "view/iview.hpp"

class IWorkspace : public IView
{
public:
    IWorkspace();

    virtual const char* GetWorkspaceID() const = 0;

    virtual void DrawWorkspace() = 0;

    virtual void Update() {}

    void Render() override;
};


