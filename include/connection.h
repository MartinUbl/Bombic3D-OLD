#ifndef __CONNECTION_H_
#define __CONNECTION_H_

#include <game_inc.h>
#include <shared.h>

//Vychozi adresa serveru a vychizi port
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 9987

//maximalni mozna delka packetu. Nejspise nikdy nepresahne 1kB, ale co uz..
#define MAXLEN (1*10*1024) // 10 KB

class Network
{
public:
    Network();

    void DoConnect(string phost = DEFAULT_IP, unsigned int pport = DEFAULT_PORT);
    bool IsConnected();
private:
    bool connected;
    IPaddress ip;
    TCPsocket sock;
    Uint16 port;
    char *name;
};

extern SDL_Thread *net_thread;

extern int SendToServer(GamePacket* packet);

#endif
