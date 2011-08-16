#ifndef INSTANCE_H
#define INSTANCE_H

#include "Singleton.h"

#include <map>

#define MAX_INSTANCES 10

struct Instance
{
    std::string instanceName;
    uint32 mapId;
    uint32 players;
    uint32 maxplayers;
};

class InstanceManager
{
    public:
        InstanceManager();
        ~InstanceManager();

        bool Initialize();
        bool InitDefaultInstance();
        int32 InitNewInstance();

        const char* GetInstanceString();

    private:
        std::map<uint32, Instance*> m_Instances;
};

static InstanceManager* sInstanceManager = Singleton<InstanceManager>::Instance();

#endif
