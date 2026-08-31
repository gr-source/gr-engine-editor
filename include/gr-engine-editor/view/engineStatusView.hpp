#pragma once

#include "view/iwindowView.hpp"

#include "gr-base/core/default.hpp"

class EngineStatusView : public IWindowView
{
public:
private:
    void Draw() override;

    void renderer(Scene* scene);
};


