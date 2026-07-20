#pragma once

class IView
{
public:
    virtual ~IView() {}

    virtual void Render() = 0;
};

/*
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

    virtual void dropPath(const std::vector<std::string>& entries) {}
};
*/








