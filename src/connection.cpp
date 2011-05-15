#include <game_inc.h>

int net_thread_main(void *);

TCPsocket serversock;

int done = 0;
SDL_Thread *net_thread = NULL;

//Precte zpravu ze socketu a ulozi ji do bufferu *buf
char *getMsg(TCPsocket sock, char **buf)
{
	Uint32 len,result;
	static char *_buf;

	if(!buf)
		buf = &_buf;

	if(*buf)
		free(*buf);
	*buf = NULL;

	result = SDLNet_TCP_Recv(sock,&len,sizeof(len));
	if(result < sizeof(len))
	{
		if(SDLNet_GetError() && strlen(SDLNet_GetError()))
			printf("SDLNet_TCP_Recv: %s\n", SDLNet_GetError());
		return NULL;
	}

	len=SDL_SwapBE32(len);

	if(!len)
		return NULL;

	*buf = (char*)malloc(len);
	if(!(*buf))
		return NULL;

	result = SDLNet_TCP_Recv(sock,*buf,len);
	if(result < len)
	{
		if(SDLNet_GetError() && strlen(SDLNet_GetError()))
			printf("SDLNet_TCP_Recv: %s\n", SDLNet_GetError());
		free(*buf);
		buf = NULL;
	}

	return (*buf);
}

//Zapise zpravu z bufferu buf na socket
int putMsg(TCPsocket sock, char *buf)
{
	Uint32 len,result;

	if(!buf || !strlen(buf))
		return 1;

	len = strlen(buf)+1;

	len = SDL_SwapBE32(len);

	result = SDLNet_TCP_Send(sock,&len,sizeof(len));
	if(result < sizeof(len))
    {
		if(SDLNet_GetError() && strlen(SDLNet_GetError()))
			printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
		return 0;
	}

	len = SDL_SwapBE32(len);

	result = SDLNet_TCP_Send(sock,buf,len);
	if(result < len)
    {
		if(SDLNet_GetError() && strlen(SDLNet_GetError()))
			printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
		return 0;
	}

	return result;
}

//vraci cast retezce od zacatku az do prvniho nalezeneho delimiteru
char *strsep(char **stringp, const char *delim)
{
	char *p;
	
	if(!stringp)
		return NULL;

	p = *stringp;
	while(**stringp && !strchr(delim,**stringp))
		(*stringp)++;

	if(**stringp)
	{
		**stringp = '\0';
		(*stringp)++;
	}
	else
		*stringp = NULL;

	return p;
}

//Rozparsuje a posle packet
int SendPacket(TCPsocket sock, GamePacket* packet)
{
    //celkova velikost + 4 bajty na ID opkodu + 4 bajty na velikost tela packetu
    size_t psize = packet->GetSize() + 4 + 4;

    char* buff = new char[psize];
    buff[0] = HIPART32(packet->GetOpcode())/0x100;
    buff[1] = HIPART32(packet->GetOpcode())%0x100;
    buff[2] = LOPART32(packet->GetOpcode())/0x100;
    buff[3] = LOPART32(packet->GetOpcode())%0x100;

    buff[4] = HIPART32(packet->GetSize())/0x100;
    buff[5] = HIPART32(packet->GetSize())%0x100;
    buff[6] = LOPART32(packet->GetSize())/0x100;
    buff[7] = LOPART32(packet->GetSize())%0x100;

    for(size_t i = 0; i < packet->GetSize(); i++)
    {
        *packet >> buff[8+i];
    }

    char* buf = buff;
    sprintf(buf,"%s",buff);

    if(!buf || !psize)
        return 0;

    int len, result;

	len = psize+1;

	len = SDL_SwapBE32(len);

	result = SDLNet_TCP_Send(sock,&len,sizeof(len));
	if(result < sizeof(len))
    {
		if(SDLNet_GetError() && strlen(SDLNet_GetError()))
			printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
		return 0;
	}

	len = SDL_SwapBE32(len);
	
	result = SDLNet_TCP_Send(sock,buf,len);
	if(result < len)
    {
		if(SDLNet_GetError() && strlen(SDLNet_GetError()))
			printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
		return 0;
	}

    return result;
}

//Posle packet serveru (helper funkce)
int SendToServer(GamePacket* packet)
{
    return SendPacket(serversock,packet);
}

//Defaultni konstruktor tridy Network, pouze nejake safe operace
Network::Network()
{
    connected = false;
}

//Jsme pripojeni?
bool Network::IsConnected()
{
    return connected;
}

//Pripojovaci funkce
void Network::DoConnect(std::string phost, unsigned int pport)
{
    if(SDLNet_Init() == -1)
    {
        //Nelze se pripojit
        //TODO: vyhodit error
        connected = false;
        return;
    }

    port = pport;

    //Pokusit se dosahnout serveru
    if(SDLNet_ResolveHost(&ip,phost.c_str(),port) == -1)
	{
		connected = false;
		SDLNet_Quit();
        return;
	}

    //Otevrit socket
    sock = SDLNet_TCP_Open(&ip);

    if(!sock)
    {
        connected = false;
        return;
    }

    //Nejake nahodne jmeno pro nasi session (A-Z znaky)
    char* name = new char[20];
    for(int i = 0; i < 19; i++)
        name[i] = 65+rand()%25;
    name[19] = '\0';

    //Inicializacni packet se jmenem (aby server zaznamenal novou session)
    if(!putMsg(sock,(char*)name))
	{
        connected = false;
		SDLNet_TCP_Close(sock);
		SDLNet_Quit();
		SDL_Quit();
        exit(0);
		return;
	}

    serversock = sock;

    //A vytvorime u serveru novou session
    GamePacket data(CMSG_INITIATE_SESSION);
    data << uint32(strlen(VERSION_STR));
    data << VERSION_STR;
    data << uint32(strlen(name));
    data << name;
    SendPacket(sock,&data);

    //A vytvorime si pro sebe nove net_thread vlakno, ktere se bude starat o komunikaci
    net_thread = SDL_CreateThread(net_thread_main,sock);
}

//funkce pro handling prochozich packetu
void HandlePacket(GamePacket* packet, TCPsocket sock)
{
    switch(packet->GetOpcode())
    {
        case SMSG_INITIATE_SESSION_RESPONSE: //Odpoved na novou session
            {
                uint8 loginstate;
                uint32 nsize, sguid;

                *packet >> loginstate;
                *packet >> nsize;
                const char* name = packet->readstr(nsize);
                *packet >> sguid;

                if(loginstate != OK)
                {
                    //Vytvoreni session se nezdarilo (spatna verze napr.), nepokracovat
                    //TODO: chybova hlaska
                    return;
                }

                //Jit dale - login

                break;
            }
        default:
            //jen pro DEBUG
            MessageBox(0,"Neznamy opkod!","Chyba",0);
            break;
    }
}

//Zpracovani prochoziho paketu
void ProcessPacket(char* message, TCPsocket sock)
{
    unsigned int opcode, size;

    //Jako prvni si rozparsujeme opkod
    opcode =  message[0]*0x1000000;
    opcode += message[1]*0x10000;
    opcode += message[2]*0x100;
    opcode += message[3];

    GamePacket packet(opcode);

    //dale si rozparsujeme velikost tela packetu
    size =  message[4]*0x1000000;
    size += message[5]*0x10000;
    size += message[6]*0x100;
    size += message[7];

    //ted telo packetu po bytech
    for(size_t i = 0; i < size; i++)
        packet << (unsigned char)message[8+i];

    //a nakonec ho nechat ohandlovat jinou funkci
    HandlePacket(&packet, sock);
}

//Sitovaci vlakno
int net_thread_main(void *data)
{
	TCPsocket sock = (TCPsocket)data;
	SDLNet_SocketSet set;
	int numready;
	char *str = NULL;

	set = SDLNet_AllocSocketSet(1);
	if(!done && !set)
	{
		printf("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
		SDLNet_Quit();
		SDL_Quit();
		done = 2;
	}

	if(!done && SDLNet_TCP_AddSocket(set,sock) == -1)
	{
		printf("SDLNet_TCP_AddSocket: %s\n",SDLNet_GetError());
		SDLNet_Quit();
		SDL_Quit();
		done = 3;
	}
	
	while(!done)
	{
		numready = SDLNet_CheckSockets(set, (Uint32)-1);
		if(numready == -1)
		{
			printf("SDLNet_CheckSockets: %s\n",SDLNet_GetError());
			done = 4;
			break;
		}

		if(numready && SDLNet_SocketReady(sock))
		{
			if(!getMsg(sock,&str))
			{
				char *errstr=SDLNet_GetError();
				printf("getMsg: %s\n",strlen(errstr)?errstr:"Server disconnected");
				done = 5;
				break;
			}
            ProcessPacket(str, sock);
		}
	}

	if(!done)
		done = 1;

	return 0;
}
