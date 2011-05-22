#include <game_inc.h>

Timer gTimer;

void Timer::Initialize()
{
    TimedEvents.clear();
}

void Timer::AddTimedEvent(uint32 time, void (*Handler)(uint32, uint32), uint32 param1, uint32 param2)
{
    TimerRecord temp;
    temp.Handler = Handler;
    temp.maxtime = time;
    temp.remaintime = time;
    temp.param1 = param1;
    temp.param2 = param2;
    TimedEvents.push_back(temp);
}

void Timer::Update(clock_t diff)
{
    if (TimedEvents.empty())
        return;

    //std::list<TimerRecord*> RemoveList;
    //RemoveList.clear();

    for (std::list<TimerRecord>::iterator itr = TimedEvents.begin(); itr != TimedEvents.end(); ++itr)
    {
        if (itr->remaintime <= diff)
        {
            itr->Handler(itr->param1, itr->param2);
            //RemoveList.push_back((itr));
            itr->remaintime = itr->maxtime;
        } else itr->remaintime -= diff;
    }

    //for (std::list<TimerRecord>::iterator itr = RemoveList.begin(); itr != RemoveList.end(); ++itr)
    //    TimedEvents.remove((*itr));
}


