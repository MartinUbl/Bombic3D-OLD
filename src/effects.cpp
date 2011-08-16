#include <effects.h>
#include <algorithm>

EmitterMgr gEmitterMgr;
TextureAnimationMgr gTextureAnimationMgr;

ParticleEmitter::ParticleEmitter()
{
}

ParticleEmitter::~ParticleEmitter()
{
    //Pokud existuji nejake castice, vymazat je
    if (!Particles.empty())
        ClearParticles();
}

void ParticleEmitter::ClearParticles()
{
    //Vymazani vsech castic
    Particle* temp = NULL;
    for (std::list<Particle*>::iterator itr = Particles.begin(); itr != Particles.end();)
    {
        temp = *itr;

        //Vymazat i zaznam v displaylistu
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

    uint32 newparticlescount = 10;

    //Zde update vsech castic a emit novych
    while(newparticlescount > 0)
    {
        if (Particles.size() < velocity-1)
        {
            if (nextParticleCountdown < diff)
            {
                //vytvorit novou castici
                Particle* pNew = new Particle;
                //a priradit ji veskere hodnoty

                //velice neuniverzalni a ne prilis profesionalni postup zde
                pNew->sx = x;
                pNew->sz = z;
                if (dirangleh < PI/4 || dirangleh > 7*PI/4 || (dirangleh > 3*PI/4 && dirangleh < 5*PI/4) )
                    pNew->sz = z+frand(square1,square3);
                else
                    pNew->sx = x+frand(square1,square3);
                pNew->sy = y+frand(square2,square4);
                //az sem

                pNew->modelId = modelId;
                pNew->actTime = 0;
                pNew->destrange = frand(minrange,maxrange);
                pNew->hangle = frand(dirangleh-angleh/2,dirangleh+angleh/2);
                pNew->vangle = frand(diranglev-anglev/2,diranglev+anglev/2);
                pNew->modelSize = modelSize+frand(min(0,sizevar),max(0,sizevar));
                float rotate = 0.0f;
                if (flags & EMIT_FLAG_RANDOM_ROTATE)
                    rotate = frand(0,PI);

                //pridat ji do display listu a ulozit pointer
                pNew->pRec = gDisplay.DrawModel(x,y,z,modelId, ANIM_IDLE,false,pNew->modelSize,rotate);

                Particles.push_back(pNew);
                nextParticleCountdown = density;
            } else nextParticleCountdown -= diff;
        }
        --newparticlescount;
    }

    Particle* temp;
    // Projit particles a rozpohybovat je
    for (std::list<Particle*>::iterator itr = Particles.begin(); itr != Particles.end(); ++itr)
    {
        temp = *itr;

        //pricist cas ktery ubehl [ms]
        temp->actTime += diff;

        if (!temp->pRec)
            continue;

        //speed [pocet tisicin rozmeru za 1 sekundu]
        temp->pRec->x = temp->sx+(float(temp->actTime)/1000)*(float(speed)/1000)*cos(temp->hangle);
        temp->pRec->y = temp->sy+(float(temp->actTime)/1000)*(float(speed)/1000)*sin(temp->vangle);
        temp->pRec->z = temp->sz+(float(temp->actTime)/1000)*(float(speed)/1000)*sin(temp->hangle);

        //pokud castice urazila drahu jakou ma urazit, vymazeme ji
        if (pythagoras_c(pythagoras_c(temp->pRec->x-temp->sx,temp->pRec->z-temp->sz),temp->pRec->y-temp->sy) > temp->destrange)
        {
            temp->pRec->remove = true;
            itr = Particles.erase(itr);
            if (itr == Particles.end())
                break;
            if (itr != Particles.begin())
                --itr;
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
                    float square1, float square2, float square3, float square4,
                    unsigned int modelId, float modelSize, float sizevar,
                    float minrange, float maxrange,
                    unsigned int time, unsigned int speed, unsigned int velocity, unsigned int density,
                    bool gravity, unsigned int flags)
{
    //Vytvori novy emitter, priradi mu dane hodnoty a prida ho do listu emitteru
    ParticleEmitter* pTemp = new ParticleEmitter();
    pTemp->angleh = angleh;
    pTemp->anglev = anglev;
    pTemp->dirangleh = dirangleh;
    pTemp->diranglev = diranglev;
    pTemp->square1 = square1;
    pTemp->square2 = square2;
    pTemp->square3 = square3;
    pTemp->square4 = square4;
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

    //Projde vsechny emittery, zavola jejich update funkci, a pokud vrati false, vymazat z listu emitteru
    for(list<ParticleEmitter*>::const_iterator itr = Emitters.begin(); itr != Emitters.end(); ++itr)
    {
        if(!(*itr)->Update(diff))
        {
            //Nejdrive vymazat vsechny castice
            (*itr)->ClearParticles();
            //A az pak vymazat emitter z listu
            itr = Emitters.erase(itr);
            if (itr == Emitters.end())
                break;
            if (itr != Emitters.begin())
                --itr;
            continue;
        }
    }
}

//////////////////////////////////
// Animace                      //
//////////////////////////////////

TextureAnimationMgr::TextureAnimationMgr()
{
}

TextureAnimationMgr::~TextureAnimationMgr()
{
    //TODO: cleanup, uvolneni pameti
}

void TextureAnimationMgr::AddAnimatedTexture(uint32 *Textures, uint16 texturesCount, uint32 time)
{
    if (texturesCount == 0 || !Textures || time == 0)
        return;

    TextureAnimData* pTemp = new TextureAnimData;
    pTemp->Textures = Textures;
    pTemp->totalTextures = texturesCount;
    pTemp->time = time;

    AnimatedData[Textures[0]] = pTemp;
}

void TextureAnimationMgr::AddAnimatedBillboard(BillboardDisplayListRecord *src)
{
    if (!src || !IsAnimatedTexture(src->TextureID))
        return;

    BillboardTextureAnimRecord* pTemp = new BillboardTextureAnimRecord;
    pTemp->actual_time = 0;
    pTemp->actual_texture_pos = 0;
    pTemp->source = src;
    pTemp->data = AnimatedData[src->TextureID];

    BillboardAnimList.push_back(pTemp);
}

void TextureAnimationMgr::Update(const clock_t diff)
{
    if (!BillboardAnimList.empty())
    {
        for (std::list<BillboardTextureAnimRecord*>::iterator itr = BillboardAnimList.begin(); itr != BillboardAnimList.end(); ++itr)
        {
            (*itr)->actual_time += (uint32)diff;
            // Pokud jsme dospeli do casu kdy je treba zmenit texturu, do toho
            if ((*itr)->actual_time >= (*itr)->data->time)
            {
                (*itr)->actual_time = 0;
                (*itr)->actual_texture_pos += 1;
                if ((*itr)->actual_texture_pos >= (*itr)->data->totalTextures)
                    (*itr)->actual_texture_pos = 0;

                (*itr)->source->TextureID = (*itr)->data->Textures[(*itr)->actual_texture_pos];
            }
        }
    }

    // Prostor pro dalsi druhy animace (v budoucnu i floor textury)
}

void Display::AnimWorker()
{
    uint32 AnimFirst, AnimLast;
    ModelDisplayListRecord* temp;

    while(1)
    {
        for(std::vector<ModelDisplayListRecord*>::iterator itr = gDisplayStore.ModelDisplayList.begin(); itr != gDisplayStore.ModelDisplayList.end(); ++itr)
        {
            temp = *itr;

            //Posunout frame animace modelu pri kazdem pokusu o vykresleni
           if(temp->Animation != ANIM_NONE && !temp->remove)
            {
                AnimFirst = gDataStore.ModelData[temp->ModelID].AnimData[temp->Animation].first;
                AnimLast = gDataStore.ModelData[temp->ModelID].AnimData[temp->Animation].second;
                temp->AnimProgress += 1;
                if(temp->AnimProgress > gDisplayStore.Models[temp->ModelID].numberOfFrames || temp->AnimProgress > AnimLast)
                {
                    if (temp->Animation == ANIM_DISAPPEAR)
                    {
                        temp->remove = true;
                        temp->AnimProgress -= 1;
                    }
                    else
                        temp->AnimProgress = AnimFirst;
                }
            }

            if(temp->AnimProgress == 0)
                temp->AnimProgress = 1;
        }

        boost::this_thread::sleep(boost::posix_time::milliseconds(1));
    }
}

void runAnimWorker()
{
    gDisplay.AnimWorker();
}

