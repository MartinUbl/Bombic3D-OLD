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
typedef std::vector<std::vector<cell>> CellVector;

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
    CellVector field;
    uint16 skybox;
    std::string mapname;
    uint32 startloc[8]; //x1,y1,x2,y2,x3,y3,x4,y4

    bool IsStartLoc(int32 x, int32 y);
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
        Map* GetMap(int32 id);

    private:
        bool LoadMap(const char* mappath, Map* dest);

        std::map<uint32, Map*> m_Maps;
};

static MapManager* sMapManager = Singleton<MapManager>::Instance();

#endif
