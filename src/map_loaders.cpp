#include <game_inc.h>

bool Display::UnloadMap()
{
    //Uz byla uvolnena
    if(actmap.MapId < 0)
        return false;

    for (int i = 0; i < actmap.field.size(); i++)
    {
        actmap.field[i].clear();
    }
    actmap.field.clear();

    //Uvolneni textur z pameti
    gDisplayStore.FloorTextures.clear();
    gDisplayStore.NeededFloorTextures.clear();

    for(ModelDataStore::iterator itr = gDataStore.ModelData.begin(); itr != gDataStore.ModelData.end(); ++itr)
        (*itr).second.loaded = false;

    actmap.MapId = -1;

    return true;
}

// Vychozi chunk z mapy
struct Chunk
{
    uint32 x;
    uint32 y;
    uint16 type;
    uint16 texture;
};

//Nacte mapu ze souboru
bool Display::LoadMap(const char* MapFilename)
{
    FILE* map = fopen(MapFilename,"r");

    if(!map)
        return false;

    gDisplayStore.NeededFloorTextures.clear();
    gDisplayStore.NeededModels.clear();
    gDisplayStore.NeededModelTextures.clear();

    //Nez bude implementovano pozicovani modelu podle model mapy
    //bude nutne nacist vsechny modely do pameti, aby byly k dispozici
    //stale
    for(ModelDataStore::const_iterator itr = gDataStore.ModelData.begin(); itr != gDataStore.ModelData.end(); ++itr)
        gDisplayStore.NeededModels.push_back((*itr).first);
    //Pote se odstrani tyto radky, a do listu potrebnych modelu se budou tlacit jen
    //modely, co jsou potrebne

    uint32 namesize = 0;
    fread(&namesize,4,1,map);
    char* mapname = new char[namesize+1];
    fread(mapname,1,namesize,map);
    mapname[namesize] = 0;
    actmap.mapname = mapname;
    fread(&actmap.skybox,2,1,map);

    Chunk* pChunk = new Chunk;

    //field 1x1, will be resized
    actmap.field.resize(1);
    actmap.field[0].resize(1);

    while(fread(pChunk,sizeof(Chunk),1,map) > 0)
    {
        if (pChunk->x > actmap.field.size()-1)
        {
            actmap.field.resize(pChunk->x+1);
            actmap.field[actmap.field.size()-1].resize(actmap.field[0].size());
        }
        if (pChunk->y > actmap.field[0].size()-1)
        {
            for (uint32 i = 0; i < actmap.field.size(); i++)
                actmap.field[i].resize(pChunk->y+1);
        }
        actmap.field[pChunk->x][pChunk->y].type = pChunk->type;
        actmap.field[pChunk->x][pChunk->y].texture = pChunk->texture;

        if (!gDisplayStore.IsAlreadyNeededTexture(pChunk->texture))
            gDisplayStore.NeededFloorTextures.push_back(pChunk->texture);
    }

    return true;
}
