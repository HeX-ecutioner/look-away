#include "TimerManager.h"

TimerManager::TimerManager()
{
    onBreak = false;
    lastSwitch = clock::now();
    remaining = WORK_DURATION;
}

void TimerManager::update()
{
    auto now = clock::now();
    int elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastSwitch).count();

    if (!onBreak)
    {
        remaining = WORK_DURATION - elapsed;
        if (remaining <= 0)
        {
            onBreak = true;
            lastSwitch = now;
        }
    }
    else
    {
        remaining = BREAK_DURATION - elapsed;
        if (remaining <= 0)
        {
            onBreak = false;
            lastSwitch = now;
        }
    }
}

int TimerManager::getRemaining() const
{
    return remaining > 0 ? remaining : 0;
}

bool TimerManager::isOnBreak() const
{
    return onBreak;
}

void TimerManager::forceBreak()
{
    onBreak = true;
    lastSwitch = clock::now();
}

void TimerManager::skipBreak()
{
    onBreak = false;
    lastSwitch = clock::now();
}