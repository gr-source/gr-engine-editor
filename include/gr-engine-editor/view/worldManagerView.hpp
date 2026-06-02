#pragma once

#include "view/viewlayout.hpp"

class WorldManagerView : public ViewLayout
{
public:
    inline virtual const char* getName() const override
    {
        return "World Manager";
    }

    virtual void onRenderUI() override;
};

