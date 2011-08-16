#include <global.h>
#include "Map.h"
#include "Log.h"

MapManager::MapManager()
{
}

MapManager::~MapManager()
{
}

const char* MapManager::GetMapName(int32 map)
{
    if (map < 0)
        return "ERROR";

    return m_Maps[map]->mapname.c_str();
}

bool MapManager::Initialize()
{
    m_Maps.clear();

    char path[256];
    uint32 pos = 0;

    for (int i = 0; i < MAP_COUNT; i++)
    {
        pos = PresentMaps[i].id;
        sprintf(path,"%s%s%s",DATA_PATH,PATH_DIR,PresentMaps[i].name.c_str());
        m_Maps[pos] = new Map;
        if (!LoadMap(path,m_Maps[pos]))
        {
            sLog->StringOut("Could not load map %s", path);
            return false;
        }
        sLog->StringOut("Loaded map %u: %s",pos,m_Maps[pos]->mapname.c_str());
    }

    return true;
}

bool MapManager::LoadMap(const char* mappath, Map* dest)
{
    FILE* MapFile = fopen(mappath,"r");

    if (!MapFile)
        return false;

    uint32 namesize = 0;
    fread(&namesize,4,1,MapFile);
    char* mapname = new char[namesize+1];
    fread(mapname,1,namesize,MapFile);
    mapname[namesize] = 0;
    dest->mapname = mapname;
    fread(&dest->skybox,2,1,MapFile);

    MapChunk* pChunk = new MapChunk;

    //field 1x1, will be resized
    dest->field.resize(1);
    dest->field[0].resize(1);

    while(fread(pChunk,sizeof(MapChunk),1,MapFile) > 0)
    {
        if (pChunk->x > dest->field.size()-1)
        {
            dest->field.resize(pChunk->x+1);
            dest->field[dest->field.size()-1].resize(dest->field[0].size());
        }
        if (pChunk->y > dest->field[0].size()-1)
        {
            for (uint32 i = 0; i < dest->field.size(); i++)
                dest->field[i].resize(pChunk->y+1);
        }
        dest->field[pChunk->x][pChunk->y].type = pChunk->type;
        dest->field[pChunk->x][pChunk->y].texture = pChunk->texture;
    }

    return true;
}
