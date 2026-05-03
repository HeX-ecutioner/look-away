#pragma once
#include <chrono>

class TimerManager
{
public:
    TimerManager();

    void update();
    int getRemaining() const;
    bool isOnBreak() const;

    void forceBreak();
    void skipBreak();

private:
    using clock = std::chrono::steady_clock;

    bool onBreak;
    int remaining;

    #ifdef DEBUG_TIMER
        const int WORK_DURATION = 10;
        const int BREAK_DURATION = 5;
    #else
        const int WORK_DURATION = 20 * 60;
        const int BREAK_DURATION = 20;
    #endif

    clock::time_point lastSwitch;
};