#ifndef __CONNECTION_H_
#define __CONNECTION_H_

#include <game_inc.h>
#include <shared.h>
#include <windows.h>
#include <winsock.h>

//Default server and port
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 2530

#define BUFFER_LEN 1*1024

class Network
{
    public:
        Network();
        ~Network();

        bool Startup();

        void Connect(std::string host, uint32 port);
        bool IsConnected() { return m_connected; };

        void Worker();

    protected:
        bool m_connected;

        std::string m_host;
        uint32 m_port;

        int m_mySocket;
        hostent* m_hostent;
        sockaddr_in m_serverSock;
};

#endif
