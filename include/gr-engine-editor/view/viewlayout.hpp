#pragma once

#include <cstdint>

typedef uint64_t ViewHash;

class IView
{
public:
    virtual ~IView() {}

    virtual void Renderer() {}
};

class ViewLayout
{
public:
    virtual ~ViewLayout() {}

    virtual const char* getName() const { return "Unknow"; }

    virtual void onFocus() {}

    virtual void onBlur() {}

    virtual void onUpdate() {}

    virtual void onRender3D() {}

    virtual void onRenderUI() {}

    virtual void drop(void *data) {}
};









