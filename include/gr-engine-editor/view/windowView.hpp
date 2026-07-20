#pragma once

#include "view/iview.hpp"

class WindowView : public IView
{
public:
    bool open = false;

    virtual const char* GetTitle() const = 0;

    virtual void Draw() = 0;

    void Render() override;
};

