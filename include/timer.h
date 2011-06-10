#ifndef __TIMER_H__INCL__
#define __TIMER_H__INCL__

#include <game_inc.h>

struct TimerRecord
{
    uint32 maxtime; //milisekundy
    uint32 remaintime; //milisekundy
    uint32 param1;
    uint32 param2;
    uint32 param3;
    void (*Handler)(uint32 param1, uint32 param2, uint32 param3);
};

class Timer
{
public:
    Timer() {};
    ~Timer() {};

    void Initialize();
    void Update(clock_t diff);

    void AddTimedEvent(uint32 time, void (*Handler)(uint32, uint32, uint32),uint32 param1, uint32 param2, uint32 param3);

    void SetDiff(clock_t diff) { m_diff = diff; };
    clock_t GetDiff() { return m_diff; };
protected:
    clock_t m_diff;
    std::list<TimerRecord> TimedEvents;
};

extern Timer gTimer;

#endif

