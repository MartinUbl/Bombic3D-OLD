/*                                         *
*            Bomberman Server              *
*                                          *
*    Developed by: Cmaranec (Kennny)       *
*                                          *
*    Copyright (c) 2011                    *
*                                          */

#include <global.h>
#include "NetworkHandlers.h"
#include "Log.h"
#include "Storage.h"
#include "Instance.h"

void Session::BroadcastPacket(SmartPacket* data)
{
    if (clientList.empty())
        return;

    for (std::list<Player*>::const_iterator itr = clientList.begin(); itr != clientList.end(); ++itr)
        SendPacket((*itr)->m_socket, data);
}

void Session::SendPacket(Client* pClient, SmartPacket* data)
{
    if (!pClient)
    {
        sLog->ErrorOut("Invalid (NULL) Client pointer as argument in Session::SendPacket");
        return;
    }

    SendPacket(pClient->m_socket, data);
}

void Session::SendPacket(SOCK socket, SmartPacket *data)
{
    //Total storage size + 4 bytes for opcode + 4 bytes for size
    size_t psize = data->GetSize() + 4 + 4;

    char* buff = new char[psize];
    unsigned int opcode = data->GetOpcode();
    memcpy(&buff[0],&opcode,sizeof(unsigned int));

    unsigned int size = (unsigned int)data->GetSize();
    memcpy(&buff[4],&size,sizeof(unsigned int));

    for(size_t i = 0; i < size; i++)
    {
        *data >> buff[8+i];
    }

    send(socket, buff, psize, 0);
}

SmartPacket* Session::BuildPacket(const char *buffer, uint32 size)
{
    if (size < 8)
    {
        sLog->ErrorOut("Received packet without smart header. Data: %s", buffer);
        return NULL;
    }

    unsigned int opcode, psize;

    //at first, parse opcode ID
    memcpy(&opcode,&buffer[0],sizeof(unsigned int));

    SmartPacket* packet = new SmartPacket(opcode);

    //next parse size
    memcpy(&psize,&buffer[4],sizeof(unsigned int));

    //and parse the body of packet
    for(size_t i = 0; i < psize; i++)
        *packet << (unsigned char)buffer[8+i];

    return packet;
}

void Session::ProcessPacket(SmartPacket* packet, Player* pSource)
{
    switch(packet->GetOpcode())
    {
        case CMSG_INITIATE_SESSION:
        {
            SmartPacket response(SMSG_INITIATE_SESSION_RESPONSE);
            response << uint32(sGlobalStorage->GetLock());
            SendPacket(pSource, &response);
            break;
        }
        case CMSG_VALIDATE_VERSION:
        {
            SmartPacket response(SMSG_VALIDATE_VERSION_RESPONSE);
            response << VERSION_STR;
            response << uint32(1);   // I pres to hratelny - jen doporuci update pri rozdilu verzi
            SendPacket(pSource, &response);

            break;
        }
        case CMSG_REQUEST_INSTANCE_LIST:
        {
            const char* inststr = sInstanceManager->GetInstanceString();
            SmartPacket response(SMSG_INSTANCE_LIST);
            response << inststr;
            SendPacket(pSource, &response);

            break;
        }
        case CMSG_ENTER_GAME:
        {
            uint32 instanceId;
            std::string nickname;

            *packet >> instanceId;
            nickname = packet->readstr();

            if (sSession->GetPlayerByName(nickname.c_str()))
            {
                SmartPacket response(SMSG_ENTER_GAME_RESULT);
                response << uint8(1); //error - nick uz ma nekdo jiny
                SendPacket(pSource, &response);
            }

            pSource->m_nickName = nickname;

            sInstanceManager->RegisterPlayer(pSource, instanceId);
            if (sInstanceManager->GetPlayerInstanceId(pSource) != instanceId)
            {
                SmartPacket response(SMSG_ENTER_GAME_RESULT);
                response << uint8(2); //error - nelze pridat do instance
                SendPacket(pSource, &response);
                return;
            }

            // TODO: nacteni start. pozic ze souboru
            pSource->m_positionX = 3.0f;
            pSource->m_positionY = 3.0f;

            SmartPacket response(SMSG_ENTER_GAME_RESULT);
            response << uint8(0); //vsechno ok
            response << float(pSource->m_positionX); // startovni pozice X
            response << float(pSource->m_positionY); // startovni pozice Y (klientsky Z)
            response << uint32(pSource->m_socket);   // jako ID pouzijeme socket ID
            response << instanceId;
            SendPacket(pSource, &response);

            SmartPacket inotify(SMSG_NEW_PLAYER);
            inotify << uint32(pSource->m_socket);       // ID
            inotify << pSource->m_positionX;            // pozice X
            inotify << pSource->m_positionY;            // pozice Y
            inotify << uint8(pSource->m_modelIdOffset); // offset modelu, klient si s tim poradi
            inotify << nickname.c_str();                // nick
            sInstanceManager->SendInstancePacket(&inotify, instanceId);
            break;
        }
        case MSG_NONE:
        default:
            sLog->ErrorOut("Received unknown/invalid opcode: %u",packet->GetOpcode());
            break;
    }
}
