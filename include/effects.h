#ifndef _EFFECTS_H_
#define _EFFECTS_H_

#include "game_inc.h"

static float frand() { return rand() / (float) RAND_MAX; }
static float frand(float f) { return frand() * f; }
static float frand(float f, float g) { return frand(g - f) + f; }
static float pythagoras_c(float a, float b) { return sqrt(a*a+b*b); }

struct Particle
{
    //Pozice jedne castice
    float x,y,z;
    //Jeji model
    unsigned int modelId;
    float modelSize;
    //A doba pusobeni na svete
    unsigned int actTime;
    //dolet, nahodne vybrany
    float destrange;
    //uhly ve kterych poleti
    float hangle;
    float vangle;

    ModelDisplayListRecord* pRec;
};

enum EmitterFlags
{
    EMIT_FLAG_RANDOM_ROTATE = 0x1,
};

class ParticleEmitter
{
public:
    ParticleEmitter();
    ~ParticleEmitter();

    //Metoda obnoveni
    bool Update(const clock_t diff);
    //Vymazani castic
    void ClearParticles();

    //Staticke promenne
    //Pozice naseho emitteru
    float x,y,z;
    //Vertikalni a horizontalni smer a uhel
    float dirangleh, diranglev, angleh, anglev;
    //Model co bude emittovan (nejspis bude nutne nejake pole modelu pro hezci finalni vystup)
    unsigned int modelId;
    float modelSize, sizevar;
    //Maximalni a minimalni dolet castic
    float minrange, maxrange;
    //Cas po jaky maji byt castice vydavany, jejich rychlost a velocity (po jakem case (milisekundy) se vyplivne dalsi castice)
    unsigned int time, speed, velocity, density;
    //Zdali ma pusobit gravitace
    bool gravity;
    //A nejakou promennou pro flagy
    unsigned int flags;

    //Pohyblive promenne
    //celkovy ubehly cas (milisekundy, nejspis)
    unsigned int totaltime;

    //Pracovni promenne
    unsigned int actTime;
    unsigned int nextParticleCountdown;
    list<Particle*> Particles;
};

class EmitterMgr
{
public:
    EmitterMgr();
    ~EmitterMgr();

    void AddEmitter(float x, float y, float z,
                    float dirangleh, float diranglev, float angleh, float anglev,
                    unsigned int modelId, float modelSize, float sizevar,
                    float minrange, float maxrange,
                    unsigned int time, unsigned int speed, unsigned int velocity, unsigned int density,
                    bool gravity, unsigned int flags);
    void AddEmitter(ParticleEmitter* pEmitter);

    void Update(const clock_t diff);
protected:
    list<ParticleEmitter*> Emitters;
};

extern EmitterMgr gEmitterMgr;

#endif

