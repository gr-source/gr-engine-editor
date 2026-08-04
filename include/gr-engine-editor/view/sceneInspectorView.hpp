#pragma once

#include "gr-base/core/system.hpp"

#include "gr-base/ecs/typeinfo.hpp"

#include "view/windowView.hpp"

class SceneInspectorView : public WindowView
{
public:
    void Draw() override;

private:
    void renderer(Scene* scene);

    void ShowSystemInfo(Scene* scene, const SystemInfo* info, ISystem* system);

    void AddSystemPopup(Scene* scene);
};


