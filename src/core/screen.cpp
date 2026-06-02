#include "core/screen.hpp"

void Screen::SetResolution(const Vector2 &value)
{
    return GetInstance().setResolution(value);
}

const Vector2 &Screen::GetResolution()
{
    return GetInstance().getResolution();
}

// private
Screen &Screen::GetInstance()
{
    static Screen instance;
    return instance;
}

Screen::Screen() : s_Resolution(Vector2::zero)
{}

void Screen::setResolution(const Vector2 &value)
{
    s_Resolution = value;
}

const Vector2 &Screen::getResolution() const
{
    return s_Resolution;
}



