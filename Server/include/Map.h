#ifndef MAP_H
#define MAP_H

#include "Singleton.h"
#include "Player.h"

#include <vector>
#include <map>

// Jedna bunka mapy
struct cell
{
    uint16 type;
    uint16 texture;
};

struct Map
{
    Map()
    {
        MapId = -1;
        field.resize(1);
        field[0].resize(1);
        mapname = "Unknown";
        skybox = 0;
    }
    int MapId;
    std::vector<std::vector<cell>> field;
    uint16 skybox;
    std::string mapname;
};

// Vychozi chunk z mapy
struct MapChunk
{
    uint32 x;
    uint32 y;
    uint16 type;
    uint16 texture;
};

static const struct { uint32 id : 4; std::string name; } PresentMaps[] = {
    {1, "001.bm"},
};

#define MAP_COUNT 1

class MapManager
{
    public:
        MapManager();
        ~MapManager();

        bool Initialize();
        const char* GetMapName(int32 map);

    private:
        bool LoadMap(const char* mappath, Map* dest);

        std::map<uint32, Map*> m_Maps;
};

static MapManager* sMapManager = Singleton<MapManager>::Instance();

#endif
