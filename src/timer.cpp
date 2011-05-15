#include <game_inc.h>

Timer gTimer;

void Timer::Initialize()
{
    if (!QueryPerformanceFrequency((LARGE_INTEGER*)&frequency))
    {
        ptimer = false;
        timer_start = timeGetTime();
        frequency = 1000;
        resolution = 1.0f/(float)frequency;
        timer_elapsed = timer_start;
    }
    else
    {
        QueryPerformanceCounter((LARGE_INTEGER*)&ptimer_start);

        ptimer = true;
        resolution = (float)(((double)1.0f)/((double)frequency));
        ptimer_elapsed = ptimer_start;
    }
}

uint64 Timer::GetTime()
{
    __int64 time;

    if (ptimer)
    {
        QueryPerformanceCounter((LARGE_INTEGER*)&time);
        return uint64(((float)(time - ptimer_start)*resolution)*1000.0f);
    }
    else
        return uint64(((float)(timeGetTime() - timer_start)*resolution)*1000.0f);
}

