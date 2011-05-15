#include <effects.h>

bool ParticleEmitter::Update()
{
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

void EmitterMgr::AddEmitter(float x, float y, float z, float dirangleh, float diranglev, float angleh, float anglev, unsigned int textureId, float minrange, float maxrange, unsigned int time, unsigned int speed, unsigned int velocity, bool gravity)
{
    //TODO: DO
}

void EmitterMgr::Update()
{
    if(Emitters.empty())
        return;

    for(vector<ParticleEmitter*>::const_iterator itr = Emitters.begin(); itr != Emitters.end(); ++itr)
    {
        if(!(*itr)->Update())
        {
            //Pridat emitter do "remove listu", aby se nenarusila iterace jeho mazanim
        }
    }

    //projit remove list a vymazat ukoncene emittery
}

