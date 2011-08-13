#ifndef __GAME_INC_H_
#define __GAME_INC_H_

#include <windows.h>
#include <iostream>
#include <fstream>
#include <windowsx.h>
#include <commctrl.h>
#include <basetsd.h>
#include <stdio.h>
#include <tchar.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glaux.h>
#include <objbase.h>
#include <math.h>
#include <vector>
#include <time.h>
#include <stdarg.h>
#include <map>
#include <list>
#include <string>
#include <locale>
#include <algorithm>
#include <SDL.h>
#include <SDL_net.h>

//Definice nejpouzivanejsich typu
//Inspirace z emulatoru MaNGOS
typedef unsigned long uint64;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef long int64;
typedef int int32;
typedef short int16;
typedef char int8;

#ifdef _DEBUG
 #define DATA_PATH "../../data"
 #define SETTINGS_PATH "../../data/settings"
#else
 #define DATA_PATH "./data"
 #define SETTINGS_PATH "./settings"
#endif

using namespace std;

#pragma warning (disable:4996) //"fopen","fscanf",... may be unsafe
#pragma warning (disable:4018) //signed/unsigned mismatch
#pragma warning (disable:4068) //unknown pragma

#define DEF_WINDOW_WIDTH 800  //Vychozi sirka okna
#define DEF_WINDOW_HEIGHT 600 //- ''  - vyska okna
#define DEF_COLOR_DEPTH 32    //- ''  - barevna hloubka

#include "sqlite3.h"
#include "sqlite3ext.h"
#include "libsqlitewrapped.h"

#include "3ds.h"
#include "drawing.h"
#include "connection.h"
#include "control.h"
#include "datastore.h"
#include "effects.h"
#include "timer.h"
#include "util.h"
#include <shared.h>

extern HWND hWnd;
extern HDC hDc;

extern bool ExtLog(int i);
extern bool readstr(FILE *f, char *string);
extern GLvoid glPrint(const char *fmt, ...);
extern vector<string> explode(const string& str, const char& ch);

struct TSettings
{
    unsigned int WindowWidth;
    unsigned int WindowHeight;
    unsigned char ColorDepth;
    bool fullscreen;
    unsigned int RefreshRate;
};

extern TSettings gConfig;

#endif

