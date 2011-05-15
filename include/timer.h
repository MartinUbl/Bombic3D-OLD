#ifndef __TIMER_H__INCL__
#define __TIMER_H__INCL__

#include <game_inc.h>

/* Trida casovace, pro rizeni hernich mechanismu a prizpusobeni
 * starsim pocitacum (celkova rychlost bude stejna, ale na starsich
 * zelezech se bude obraz "trhat" pokud nedosahnou urcite rychlosti)
 */
class Timer
{
public:
    Timer() {};
    ~Timer() {};

    void Initialize();
    uint64 GetTime();
protected:
    int64  frequency;      // frekvence
    float  resolution;     // perioda ticku
    uint64 timer_start;    // cas odstartovani casovace
    uint64 timer_elapsed;  // cas ktery uplynul od startu
    bool   ptimer;         // performance timer?
    int64  ptimer_start;   // cas odstartovani performance timeru
    int64  ptimer_elapsed; // cas ktery uplynul u performance timeru
};

static const uint32 steps[] = {1, 2, 4, 6, 8, 10};

extern Timer gTimer;

#endif

