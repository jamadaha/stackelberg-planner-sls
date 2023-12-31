#ifndef UTILS_TIMER_H
#define UTILS_TIMER_H

#include <ostream>

#include "../utilities.h"

namespace utils
{
class Timer
{
    double last_start_clock;
    double collected_time;
    bool stopped;
#if OPERATING_SYSTEM == WINDOWS
    LARGE_INTEGER frequency;
    LARGE_INTEGER start_ticks;
#endif

    double current_clock() const;

public:
    Timer();
    ~Timer() = default;
    double operator()() const;
    double stop();
    void resume();
    double reset();
};

}

namespace std
{
std::ostream &operator<<(std::ostream &os, const utils::Timer &timer);
}

#endif
