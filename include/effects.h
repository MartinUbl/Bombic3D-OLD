#ifndef _EFFECTS_H_
#define _EFFECTS_H_

#include "game_inc.h"

struct Particle
{
    //Pozice jedne castice
    float x,y,z;
    //Jeji textura
    unsigned int textureId;
    //A doba pusobeni na svete
    unsigned int actTime;
};

class ParticleEmitter
{
public:
    ParticleEmitter();
    ~ParticleEmitter();

    //Metoda obnoveni
    bool Update();

protected:
    //Pozice naseho emitteru
    float x,y,z;
    //Vertikalni a horizontalni smer a uhel
    float dirangleh, diranglev, angleh, anglev;
    //Textura co bude emittovana (nejspis bude nutne nejake pole textur pro hezci finalni vystup)
    unsigned int textureId;
    //Maximalni a minimalni dolet castic
    float minrange, maxrange;
    //Cas po jaky maji byt castice vydavany, jejich rychlost a velocity (po jakem case (milisekundy) se vyplivne dalsi castice)
    unsigned int time, speed, velocity;
    //Zdali ma pusobit gravitace
    bool gravity;

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
                    unsigned int textureId,
                    float minrange, float maxrange,
                    unsigned int time, unsigned int speed, unsigned int velocity,
                    bool gravity);

    void Update();
protected:
    vector<ParticleEmitter*> Emitters;
};

#endif

