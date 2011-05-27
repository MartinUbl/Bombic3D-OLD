#include <effects.h>
#include <algorithm>

EmitterMgr gEmitterMgr;

ParticleEmitter::ParticleEmitter()
{
}

ParticleEmitter::~ParticleEmitter()
{
    //odstranit particles
}

void ParticleEmitter::ClearParticles()
{
    Particle* temp = NULL;
    for (std::list<Particle*>::iterator itr = Particles.begin(); itr != Particles.end();)
    {
        temp = *itr;

        if (temp->pRec)
            temp->pRec->remove = true;

        itr = Particles.erase(itr);
        if (itr == Particles.end())
            break;
        continue;
    }
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
    if (Particles.size() < velocity-1)
    {
        if (nextParticleCountdown < diff)
        {
            //vytvorit novou castici
            Particle* pNew = new Particle;
            pNew->x = x;
            pNew->y = y;
            pNew->z = z;
            pNew->modelId = modelId;
            pNew->actTime = 0;
            pNew->destrange = frand(minrange,maxrange);
            pNew->hangle = frand(dirangleh-angleh/2,dirangleh+angleh/2);
            pNew->vangle = frand(diranglev-anglev/2,diranglev+anglev/2);
            pNew->modelSize = modelSize+frand(min(0,sizevar),max(0,sizevar));
            float rotate = 0.0f;
            if (flags & EMIT_FLAG_RANDOM_ROTATE)
                rotate = frand(0,PI);
            pNew->pRec = gDisplay.DrawModel(x,y,z,modelId, ANIM_IDLE,false,pNew->modelSize,rotate);
            Particles.push_back(pNew);
            nextParticleCountdown = density;
        } else nextParticleCountdown -= diff;
    }

    Particle* temp;
    // Projit particles a rozpohybovat je
    for (std::list<Particle*>::iterator itr = Particles.begin(); itr != Particles.end(); ++itr)
    {
        temp = *itr;

        temp->actTime += diff;

        if (!temp->pRec)
            continue;

        temp->pRec->x = x+(float(temp->actTime)/1000)*(float(speed)/1000)*cos(temp->hangle); //pocet tisicin rozmeru za 1 sekundu
        temp->pRec->y = y+(float(temp->actTime)/1000)*(float(speed)/1000)*sin(temp->vangle);
        temp->pRec->z = z+(float(temp->actTime)/1000)*(float(speed)/1000)*sin(temp->hangle);

        if (pythagoras_c(pythagoras_c(temp->pRec->x,temp->pRec->z),temp->pRec->y) > temp->destrange)
        {
            temp->pRec->remove = true;
            itr = Particles.erase(itr);
            if (itr == Particles.end())
                break;
            continue;
        }
    }

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
                    unsigned int modelId, float modelSize, float sizevar,
                    float minrange, float maxrange,
                    unsigned int time, unsigned int speed, unsigned int velocity, unsigned int density,
                    bool gravity, unsigned int flags)
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
    pTemp->modelSize = modelSize;
    pTemp->sizevar = sizevar;
    pTemp->nextParticleCountdown = 0;
    pTemp->speed = speed;
    pTemp->time = time;
    pTemp->velocity = velocity;
    pTemp->x = x;
    pTemp->y = y;
    pTemp->z = z;
    pTemp->totaltime = 0;
    pTemp->density = density;
    pTemp->flags = flags;

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

    for(list<ParticleEmitter*>::const_iterator itr = Emitters.begin(); itr != Emitters.end(); ++itr)
    {
        if(!(*itr)->Update(diff))
        {
            (*itr)->ClearParticles();
            itr = Emitters.erase(itr);
            if (itr == Emitters.end())
                break;
            --itr;
            continue;
        }
    }
}

