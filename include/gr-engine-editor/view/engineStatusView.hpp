#pragma once

#include "view/viewlayout.hpp"

class EngineStatusView : public ViewLayout
{
public:
    inline const char* getName() const override
    {
        return "Status";
    }

private:
    void onRenderUI() override;
};


