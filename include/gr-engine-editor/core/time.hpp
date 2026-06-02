#pragma once

#include <chrono>

class gtimer
{
public:
    gtimer(double fps);

    void set_fps(double fps);

    void begin_frame();

    void end_frame();

    double get_delta() const;

private:
    double m_targetFrameTime;
    
    double m_lastTime;

    double m_delta;
};

