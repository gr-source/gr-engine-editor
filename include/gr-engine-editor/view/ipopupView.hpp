#pragma once

#include "view/iview.hpp"

class IPopupView : public IView
{
public:
    bool open = false;

    void Render() override;

    virtual void Draw() = 0;
};


