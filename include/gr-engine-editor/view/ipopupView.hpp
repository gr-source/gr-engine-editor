#pragma once

#include "view/iview.hpp"

class IPopupView : public IView
{
public:
    bool requestOpen = false;

    virtual const char* GetPopupID() const = 0;

    virtual void DrawPopup() = 0;

    void Render() override;
};


