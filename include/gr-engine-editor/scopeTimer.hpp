#pragma once

#include <chrono>

class ScopeTimer
{
public:
    ScopeTimer(const char* name);
    ~ScopeTimer();
    
private:
    std::chrono::high_resolution_clock::time_point m_start;

    const char* m_name;
};




