#ifndef INSTANCE_H
#define INSTANCE_H

#include "Singleton.h"
#include "Player.h"
#include "Packets/SmartPacket.h"

#include <map>

#define MAX_INSTANCES 10
#define MAX_PLAYERS_PER_INSTANCE 4

struct Instance
{
    Instance()
    {
        id = -1;
        for (int i = 0; i < MAX_PLAYERS_PER_INSTANCE; i++)
            pPlayers[i] = NULL;
    };
    int32 id;
    std::string instanceName;
    uint32 mapId;
    uint32 players;
    uint32 maxplayers;

    Player* pPlayers[MAX_PLAYERS_PER_INSTANCE];
};

class InstanceManager
{
    public:
        InstanceManager();
        ~InstanceManager();

        bool Initialize();
        bool InitDefaultInstance();
        int32 InitNewInstance();

        void RegisterPlayer(Player* pPlayer, uint32 instanceId);
        int32 GetPlayerInstanceId(Player* pPlayer);
        Instance* GetPlayerInstance(Player* pPlayer);
        void SendInstancePacket(SmartPacket* data, uint32 instanceId);

        const char* GetInstanceString();

    private:
        std::map<uint32, Instance*> m_Instances;
};

static InstanceManager* sInstanceManager = Singleton<InstanceManager>::Instance();

#endif
