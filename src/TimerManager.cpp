#include "TimerManager.h"

TimerManager::TimerManager() : onBreak(false), remaining(WORK_DURATION), breakCount(0)
{
    lastSwitch = clock::now();
}

void TimerManager::update()
{
    auto now = clock::now();
    int elapsed = (int)std::chrono::duration_cast<std::chrono::seconds>(now - lastSwitch).count();

    if (!onBreak)
    {
        remaining = WORK_DURATION - elapsed;
        if (remaining <= 0)
        {
            onBreak = true;
            breakCount++;
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

int TimerManager::getBreakCount() const
{
    return breakCount;
}

void TimerManager::forceBreak()
{
    if (!onBreak)
        breakCount++;
    onBreak = true;
    lastSwitch = clock::now();
}

void TimerManager::skipBreak()
{
    onBreak = false;
    lastSwitch = clock::now();
}