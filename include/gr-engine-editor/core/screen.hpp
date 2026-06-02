#pragma once

#include <gr-math/gmath.hpp>

class Screen
{
public:
    static void SetResolution(const Vector2 &value);

    static const Vector2 &GetResolution();

public:
    static Screen &GetInstance();

    Vector2 s_Resolution;

    Screen();

    void setResolution(const Vector2 &value);

    const Vector2 &getResolution() const;
};

