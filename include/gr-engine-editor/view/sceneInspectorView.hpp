#pragma once

#include "gr-base/core/system.hpp"

#include "gr-base/ecs/typeinfo.hpp"

#include "view/viewlayout.hpp"

class SceneInspectorView : public ViewLayout
{
public:
    virtual const char* getName() const override
    {
        return "Scene Inspector";
    }

    virtual void onRenderUI() override;

private:
    void ShowSystemInfo(const SystemInfo* info, ISystem* system);

    void AddSystemPopup(Scene* scene);
};


