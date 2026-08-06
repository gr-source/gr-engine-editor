#pragma once

class IView
{
public:
    virtual ~IView() {}

    virtual void Render() = 0;
};









