#pragma once

#include "view/iview.hpp"

class IWindowView : public IView
{
public:
    bool open = false;

    virtual void Draw() = 0;

    void Render() override;
};

