#pragma once

#include "gr-base/core/default.hpp"

#include "view/windowView.hpp"

class EngineStatusView : public WindowView
{
public:
private:
    inline const char* GetTitle() const override
    {
        return "Status";
    }

    void Draw() override;

    void renderer(Scene* scene);
};


