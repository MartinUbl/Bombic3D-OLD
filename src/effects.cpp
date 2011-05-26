#include <effects.h>

EmitterMgr gEmitterMgr;

ParticleEmitter::ParticleEmitter()
{
}

ParticleEmitter::~ParticleEmitter()
{
    //odstranit particles
}

bool ParticleEmitter::Update(const clock_t diff)
{
    //Nejdrive pricist k celkovemu casu ten ubehly (milisekundy)
    totaltime += diff;

    //Pak porovnat celkovy cas, zdali uz ubehnul maximalni cas
    if(totaltime >= time)
        return false; //--> odstranit emitter z listu pro update
    //TODO: dodelat "dojizdeni" emitteru - cas po ktery uz nebudou vydavany castice, ale
    //stare castice se budou stale pohybovat, az zmizi. Po tu dobu vracet stale true!

    //Zde update vsech castic a emit novych
    //...

    return true; //false pokud emitter dosahne maximalniho casu pusobeni nebo je jinak odstranen
}

EmitterMgr::EmitterMgr()
{
    //TODO: vymazani listu, atd..
}

EmitterMgr::~EmitterMgr()
{
    //TODO: uvolneni pameti (vector emitteru)
}

void EmitterMgr::AddEmitter(float x, float y, float z,
                    float dirangleh, float diranglev, float angleh, float anglev,
                    unsigned int modelId,
                    float minrange, float maxrange,
                    unsigned int time, unsigned int speed, unsigned int velocity,
                    bool gravity)
{
    ParticleEmitter* pTemp = new ParticleEmitter();
    pTemp->angleh = angleh;
    pTemp->anglev = anglev;
    pTemp->dirangleh = dirangleh;
    pTemp->diranglev = diranglev;
    pTemp->gravity = gravity;
    pTemp->maxrange = maxrange;
    pTemp->minrange = minrange;
    pTemp->modelId = modelId;
    pTemp->nextParticleCountdown = 0;
    pTemp->speed = speed;
    pTemp->time = time;
    pTemp->velocity = velocity;
    pTemp->x = x;
    pTemp->y = y;
    pTemp->z = z;

    Emitters.push_back(pTemp);
}

void EmitterMgr::AddEmitter(ParticleEmitter* pEmitter)
{
    Emitters.push_back(pEmitter);
}

void EmitterMgr::Update(const clock_t diff)
{
    if(Emitters.empty())
        return;

    static list<ParticleEmitter*> RemoveList;
    RemoveList.clear();

    for(list<ParticleEmitter*>::const_iterator itr = Emitters.begin(); itr != Emitters.end(); ++itr)
    {
        if(!(*itr)->Update(diff))
        {
            //Pridat emitter do "remove listu", aby se nenarusila iterace jeho mazanim
            RemoveList.push_back(*itr);
        }
    }

    //Projit remove list a vymazat zaznamy
    for(list<ParticleEmitter*>::const_iterator itr = RemoveList.begin(); itr != RemoveList.end(); ++itr)
        Emitters.remove(*itr);
}

