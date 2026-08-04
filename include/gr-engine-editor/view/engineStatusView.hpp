#pragma once

#include "gr-base/core/default.hpp"

#include "view/windowView.hpp"

class EngineStatusView : public WindowView
{
public:
private:
    void Draw() override;

    void renderer(Scene* scene);
};


