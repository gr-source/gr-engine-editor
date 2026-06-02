#pragma once

#include <gr-math/gmath.hpp>

class ShadowMap {
public:
    ShadowMap() = delete;

    ShadowMap(u32 width, u32 height);
    ~ShadowMap();

    void bind();

    void unbind();

    void bindTexture(u32 index);

private:
    u32 shadowFBO;

    u32 shadowMap;
};


