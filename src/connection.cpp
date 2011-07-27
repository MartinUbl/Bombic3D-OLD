#include <game_inc.h>
#include "connection.h"

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
            //boost::this_thread::sleep(boost::posix_time::milliseconds(100));
            continue;
        }

        result = recv(m_mySocket, buf, BUFFER_LEN, 0);
        if (result > 0)
        {
            // received data
        }
        else if (result == 0)
        {
            // disconnected
        }
        else
        {
            // some error
        }

        //boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    }
}
