#pragma once

#include "view/iwindowView.hpp"

#include "gr-base/core/system.hpp"

#include "gr-base/ecs/typeinfo.hpp"

class SceneInspectorView : public IWindowView
{
public:
    void Draw() override;

private:
    void renderer(Scene* scene);

    void ShowSystemInfo(Scene* scene, const SystemInfo* info, ISystem* system);

    void AddSystemPopup(Scene* scene);
};


