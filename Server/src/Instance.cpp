#include <global.h>
#include "Instance.h"
#include "Map.h"

#include <sstream>

InstanceManager::InstanceManager()
{
}

InstanceManager::~InstanceManager()
{
}

bool InstanceManager::Initialize()
{
    m_Instances.clear();

    return true;
}

bool InstanceManager::InitDefaultInstance()
{
    if (m_Instances.find(0) == m_Instances.end())
    {
        Instance* pNew = new Instance;
        if (!pNew)
            return false;

        pNew->instanceName = "Default Instance";
        pNew->mapId = 1;
        pNew->maxplayers = 4;
        pNew->players = 0;

        m_Instances[0] = pNew;

        return true;
    }

    return false;
}

int32 InstanceManager::InitNewInstance()
{
    char instname[32];

    for (uint32 i = 0; i < MAX_INSTANCES; i++)
    {
        if (m_Instances.find(i) == m_Instances.end())
        {
            Instance* pNew = new Instance;
            if (!pNew)
                return -1;

            sprintf(instname, "Instance %i", i);

            pNew->instanceName = instname;
            pNew->mapId = 1;
            pNew->maxplayers = 4;
            pNew->players = 0;

            m_Instances[i] = pNew;

            return i;
        }
    }

    return -1;
}

const char* InstanceManager::GetInstanceString()
{
    bool first = true;
    std::stringstream str;

    for (std::map<uint32, Instance*>::const_iterator itr = m_Instances.begin(); itr != m_Instances.end(); ++itr)
    {
        if (!first)
             str << "|";
        else
            first = true;

        str << itr->first << "|";
        str << itr->second->instanceName.c_str() << "|";
        str << itr->second->players << "/" << itr->second->maxplayers << "|";
        str << sMapManager->GetMapName(itr->second->mapId);
    }

    str << '\0';

    const std::string retstr(str.str());

    char* pRet = new char[retstr.size()+1];
    strcpy(pRet, retstr.c_str());
    pRet[retstr.size()] = '\0';

    return pRet;
}
