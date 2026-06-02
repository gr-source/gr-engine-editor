#pragma once

#include <atomic>

#include <gr-math/gmath.hpp>

class Settings {
public:
    static const std::atomic<bool> &IsEngineRunning();

    static void StartPlayEngine();

    static void StopPlayEngine();

    static std::atomic<bool> bGameRunning;

    static Vector2 GameGrapicQiality;

    static Vector2 IconSize;

private:
    static std::atomic<bool> m_bEngineRunning;
};

