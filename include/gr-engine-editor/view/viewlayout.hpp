#pragma once

#include <cstdint>
#include <string>

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
    bool visible;

    inline ViewLayout() : visible(false)
    {}

    virtual ~ViewLayout() {}

    virtual const char* getName() const { return "Unknow"; }

    virtual void onFocus() {}

    virtual void onBlur() {}

    virtual void onUpdate() {}

    virtual void onRender3D() {}

    virtual void onRenderUI() {}

    virtual void dropPath(const std::string& path) {}
};









