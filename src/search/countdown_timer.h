#ifndef COUNTDOWN_TIMER_H
#define COUNTDOWN_TIMER_H

#include "utils/timer.h"

#include <ostream>

class CountdownTimer {
    utils::Timer timer;
    double max_time;
public:
    explicit CountdownTimer(double max_time);
    ~CountdownTimer();
    bool is_expired() const;
    friend std::ostream &operator<<(std::ostream &os, const CountdownTimer &cd_timer);
};

std::ostream &operator<<(std::ostream &os, const CountdownTimer &cd_timer);

#endif
