#ifndef _EFFECTS_H_
#define _EFFECTS_H_

#include "game_inc.h"

struct Particle
{
    //Startovni pozice
    float sx,sy,sz;
    //Model castice
    unsigned int modelId;
    float modelSize;
    //A doba pusobeni na svete
    unsigned int actTime;
    //dolet, nahodne vybrany
    float destrange;
    //uhly ve kterych poleti
    float hangle;
    float vangle;

    //Zaznam v display listu (souradnice, pohyb)
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
    //ctverec emittu
    float square1, square2, square3, square4;

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

    //Prida emitter se zadanymi parametry
    void AddEmitter(float x, float y, float z,
                    float dirangleh, float diranglev, float angleh, float anglev,
                    float square1, float square2, float square3, float square4,
                    unsigned int modelId, float modelSize, float sizevar,
                    float minrange, float maxrange,
                    unsigned int time, unsigned int speed, unsigned int velocity, unsigned int density,
                    bool gravity, unsigned int flags);
    //Prida emitter predpripraveny uzivatelem
    void AddEmitter(ParticleEmitter* pEmitter);

    void Update(const clock_t diff);
protected:
    list<ParticleEmitter*> Emitters;
};

extern EmitterMgr gEmitterMgr;

//Struktura statickeho zaznamu pro animaci textury
struct TextureAnimData
{
    uint32 *Textures;
    uint16 totalTextures;
    uint32 time;
};

//Struktura dynamickeho zaznamu animace textury (billboard)
struct BillboardTextureAnimRecord
{
    TextureAnimData* data;
    BillboardDisplayListRecord* source;
    uint32 actual_time;
    uint16 actual_texture_pos;
};

//Trida starajici se o prubeh animaci a uchovavani statickych dat animaci
class TextureAnimationMgr
{
public:
    TextureAnimationMgr();
    ~TextureAnimationMgr();

    void AddAnimatedTexture(uint32 *Textures, uint16 texturesCount, uint32 time);

    bool IsAnimatedTexture(uint32 textureID) { return AnimatedData.find(textureID) != AnimatedData.end(); }
    void AddAnimatedBillboard(BillboardDisplayListRecord* src);

    void Update(const clock_t diff);
protected:
    map<uint32, TextureAnimData*> AnimatedData;
    list<BillboardTextureAnimRecord*> BillboardAnimList;
};

extern TextureAnimationMgr gTextureAnimationMgr;

#endif

