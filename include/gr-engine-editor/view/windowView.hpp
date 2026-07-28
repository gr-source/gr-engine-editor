#pragma once

#include "view/iview.hpp"

class WindowView : public IView
{
public:
    bool open = false;

    virtual const char* GetTitle() const { return  "NN" ; }

    virtual void Draw() = 0;

    void Render() override;
};

