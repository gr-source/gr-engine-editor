#include "scopeTimer.hpp"

#include <iostream>
#include <iomanip>

ScopeTimer::ScopeTimer(const char* name) : m_name(name)
{
    m_start = std::chrono::high_resolution_clock::now();
}

ScopeTimer::~ScopeTimer()
{
    auto end =
            std::chrono::high_resolution_clock::now();

        auto duration =
            std::chrono::duration<double, std::milli>(
                end - m_start);

        std::cout
            << m_name
            << ": "
            << std::fixed
            << std::setprecision(3)
            << duration.count()
            << " ms\n";
}



