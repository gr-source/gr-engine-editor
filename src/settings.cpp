#include "settings.h"

std::atomic<bool> Settings::m_bEngineRunning = true;

std::atomic<bool> Settings::bGameRunning = false;

Vector2 Settings::GameGrapicQiality = {1920.0f, 1080.0f};

Vector2 Settings::IconSize = {32.0f, 32.0f};

const std::atomic<bool> &Settings::IsEngineRunning() {
    return m_bEngineRunning;
}

void Settings::StartPlayEngine() {
    m_bEngineRunning = true;
}

void Settings::StopPlayEngine() {
    m_bEngineRunning = false;
}

