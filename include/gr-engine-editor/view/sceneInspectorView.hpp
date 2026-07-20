#pragma once

#include "gr-base/core/system.hpp"

#include "gr-base/ecs/typeinfo.hpp"

#include "view/windowView.hpp"

class SceneInspectorView : public WindowView
{
public:
    const char* GetTitle() const override
    {
        return "Scene Inspector";
    }

    void Draw() override;

private:
    void renderer(Scene* scene);

    void ShowSystemInfo(const SystemInfo* info, ISystem* system);

    void AddSystemPopup(Scene* scene);
};


