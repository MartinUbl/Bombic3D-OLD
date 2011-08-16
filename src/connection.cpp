#include <game_inc.h>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include "connection.h"
#include "Opcodes.h"
#include "SmartPacket.h"

Network gNetwork;

Network::Network()
{
}

Network::~Network()
{
    closesocket(m_mySocket);
    WSACleanup();
}

bool Network::Startup()
{
    WORD wVersionRequested = MAKEWORD(1,1);
    WSADATA data;

    if (WSAStartup(wVersionRequested, &data) != 0)
        return false;

    if ((m_mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        return false;

    return true;
}

void Network::Connect(std::string host, uint32 port)
{
    if ((m_hostent = gethostbyname(host.c_str())) == NULL)
    {
        m_connected = false;
        return;
    }

    m_serverSock.sin_family = AF_INET;
    m_serverSock.sin_port = htons(port);
    memcpy(&(m_serverSock.sin_addr), m_hostent->h_addr, m_hostent->h_length);
    if (connect(m_mySocket, (sockaddr*)&m_serverSock, sizeof(m_serverSock)) == -1)
    {
        m_connected = false;
        return;
    }

    m_connected = true;
}

void Network::Worker()
{
    char *buf = new char[BUFFER_LEN];
    int result;

    while(1)
    {
        if (!m_connected)
        {
            boost::this_thread::sleep(boost::posix_time::milliseconds(100));
            continue;
        }

        result = recv(m_mySocket, buf, BUFFER_LEN, 0);
        if (result > 0)
        {
            HandlePacket(BuildPacket(buf, result));
        }
        else if (result == 0)
        {
            // disconnected
        }
        else
        {
            // some error
        }

        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    }
}

void runNetworkWorker()
{
    gNetwork.Worker();
}

void Network::SendPacket(SmartPacket *data)
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

    send(m_mySocket, buff, psize, 0);
}

SmartPacket* Network::BuildPacket(const char *buffer, uint32 size)
{
    if (size < 8)
    {
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

void Network::HandlePacket(SmartPacket *data)
{
    switch (data->GetOpcode())
    {
        case SMSG_INITIATE_SESSION_RESPONSE:
        {
            uint32 lock;
            *data >> lock;

            if (lock > 0)
                gDisplay.SetGameState(GAME_MENU);
            gDisplay.SetGameStateStage(1);

            SmartPacket data(CMSG_VALIDATE_VERSION);
            data << VERSION_STR;
            SendPacket(&data);
            break;
        }
        case SMSG_VALIDATE_VERSION_RESPONSE:
        {
            std::string version;
            uint32 matter;

            version = data->readstr();
            *data >> matter;

            // Server ma verzi 'version', 'matter' je priznak toho, zdali muze na tehle verzi hrat
            // 0 = nemuze, 1 = muze

            SmartPacket data(CMSG_REQUEST_INSTANCE_LIST);
            data << uint32(0);
            SendPacket(&data);
            break;
        }
        case SMSG_INSTANCE_LIST:
        {
            std::string str;

            str = data->readstr();

            UIRecord* pField = gInterface.GetUIRecordByType(UI_TYPE_CONNECTING_FIELD_ROOMS_LIST);
            if (!pField)
                return;

            pField->fieldcontent = str.c_str();
            gDisplay.SetGameState(GAME_CONNECTING);
            gDisplay.SetGameStateStage(0);
            break;
        }
        case SMSG_ENTER_GAME_RESULT:
        {
            uint8 result;
            float startPosX, startPosZ;
            uint32 myId;
            uint32 instanceId;

            *data >> result;
            *data >> startPosX >> startPosZ;
            *data >> myId;
            *data >> instanceId;

            if (result != 0)
            {
                //TODO: zobrazit chybu
                gDisplay.SetGameState(GAME_MENU);
                return;
            }

            gDisplay.SetViewX(-(float(startPosX-1)*MAP_SCALE_X+0.15f));
            gDisplay.SetViewZ(-(float(startPosZ-1)*MAP_SCALE_X+0.15f));

            SetMyID(myId);
            m_myInstanceId = instanceId;

            gDisplay.FlushModelDisplayList();
            gDisplay.InitModelDisplayList();
            gDisplay.SetGameState(GAME_GAME);

            break;
        }
    }
}
