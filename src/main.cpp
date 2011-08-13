#include <game_inc.h>
#include <locale>

HDC hDC=NULL;
HGLRC hRC=NULL;
HWND hWnd=NULL;
HINSTANCE hInstance;

bool active=TRUE;
bool fullscreen=TRUE;

//docasne, test
GLfloat LightAmbient[]= { 1.0f, 1.0f, 1.0f, 1.0f };

GLYPHMETRICSFLOAT gmf[256]; //buffer pro font
GLuint base;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

TSettings gConfig;

//Funkce odesilajici cele cislo pomoci WM_USER+5 zpravy do custom externiho programu
bool ExtLog(int i)
{
    HWND handle = FindWindow(NULL,TEXT("Logger"));
    if(handle)
    {
        SendMessage(handle,WM_USER+5,1,i);
        return true;
    }
    else
    {
        return false;
    }
}

//pomocna funkce pro zjisteni typu souboru podle pripony
vector<string> explode(const string& str, const char& ch) {
    string next = "";
    vector<string> result;

    // For each character in the string
    for (string::const_iterator it = str.begin(); it != str.end(); it++) {
        // If we've hit the terminal character
        if (*it == ch) {
                // If we have some characters accumulated
                if (next.length() > 0) {
                        // Add them to the result vector
                        result.push_back(next);
                        next = "";
                }
        } else {
                // Accumulate the next character into the sequence
                next += *it;
        }
    }

    if(next.length() > 0)
        result.push_back(next);

    return result;
}

//Vytvoreni fontu
GLvoid BuildFont(GLvoid)                                // Build Our Bitmap Font
{
    HFONT    font;                                        // Windows Font ID
    HFONT    oldfont;                                    // Used For Good House Keeping

    base = glGenLists(96);                                // Storage For 96 Characters

    font = CreateFont(    -24,                            // Height Of Font
                        0,                                // Width Of Font
                        0,                                // Angle Of Escapement
                        0,                                // Orientation Angle
                        FW_BOLD,                        // Font Weight
                        FALSE,                            // Italic
                        FALSE,                            // Underline
                        FALSE,                            // Strikeout
                        ANSI_CHARSET,                    // Character Set Identifier
                        OUT_TT_PRECIS,                    // Output Precision
                        CLIP_DEFAULT_PRECIS,            // Clipping Precision
                        ANTIALIASED_QUALITY,            // Output Quality
                        FF_DONTCARE|DEFAULT_PITCH,        // Family And Pitch
                        "Courier New");                    // Font Name

    oldfont = (HFONT)SelectObject(hDC, font);           // Selects The Font We Want
    wglUseFontBitmaps(hDC, 32, 96, base);                // Builds 96 Characters Starting At Character 32
    SelectObject(hDC, oldfont);                            // Selects The Font We Want
    DeleteObject(font);                                    // Delete The Font
}

GLvoid KillFont(GLvoid)
{
    glDeleteLists(base, 96);
}

//Vykresleni textu
GLvoid glPrint(const char *fmt, ...)                    // Custom GL "Print" Routine
{
    char        text[256];                                // Holds Our String
    va_list        ap;                                        // Pointer To List Of Arguments

    if (fmt == NULL)                                    // If There's No Text
        return;                                            // Do Nothing

    va_start(ap, fmt);                                    // Parses The String For Variables
        vsprintf(text, fmt, ap);                        // And Converts Symbols To Actual Numbers
    va_end(ap);                                            // Results Are Stored In Text

    glPushAttrib(GL_LIST_BIT);                            // Pushes The Display List Bits
    glListBase(base - 32);                                // Sets The Base Character to 32
    glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);    // Draws The Display List Text
    glPopAttrib();                                        // Pops The Display List Bits
}

//Cteni jedne radky ze souboru, helper funkce
bool readstr(FILE *f, char *string)
{
    fgets(string, 255, f);
    if((string[0] == '/') || (string[0] == '\n')) return false;
    return true;
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)
{
    if (height == 0)
    {
        height = 1;
    }

    glViewport(0,0,width,height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

//Nacteni configu
//Pokud existuje uzivatelsky, nacist, pokud ne, nacist defaultni, a pokud ani ten ne, vychozi hodnoty
void LoadConfig()
{
    char dest[200];

    //Globalni uzivatelsky config
    sprintf(dest,"%s/global.set",SETTINGS_PATH);
    FILE* config = fopen(dest,"r");

    //Pokud neexistuje, zkusit nacist defaultni config (nemenny, dodavany autory)
    if(!config)
    {
        sprintf(dest,"%s/default_global.set",SETTINGS_PATH);
        config = fopen(dest,"r");
    }

    //Naplnit config vychozimi hodnotami
    gConfig.WindowWidth  = DEF_WINDOW_WIDTH;
    gConfig.WindowHeight = DEF_WINDOW_HEIGHT;
    gConfig.ColorDepth   = DEF_COLOR_DEPTH;
    gConfig.fullscreen   = false;

    //A pokud se nenacetl soubor configu, opustit
    if(!config)
        return;

    //Nejake pracovni promenne
    char setting[50];
    char value[50];
    int ivalue = 0;
    unsigned int i,j;
    locale loc;

    while(fgets(dest, 200, config))
    {
        i = 0;
        while(dest[i] != '=' && dest[i] != '\n')
        {
            setting[i] = dest[i];
            i++;
        }
        setting[i] = '\0';
        i++;
        j = 0;
        while(dest[i] != '\n')
        {
            value[j] = dest[i];
            i++; j++;
        }
        value[j] = '\0';
        
        //Rozpoznani nastaveni
        for(int k = 0; k < strlen(setting); k++)
            setting[k] = std::toupper(setting[k],loc);

        ivalue = atoi(value);
        
        if(strcmp(setting,"WINDOW_WIDTH") == 0)
            if(ivalue > 0)
                gConfig.WindowWidth = ivalue;

        if(strcmp(setting,"WINDOW_HEIGHT") == 0)
            if(ivalue > 0)
                gConfig.WindowHeight = ivalue;

        if(strcmp(setting,"COLOR_DEPTH") == 0)
            if(ivalue > 0)
                gConfig.ColorDepth = ivalue;

        if(strcmp(setting,"FULLSCREEN") == 0)
            gConfig.fullscreen = ivalue?true:false;
    }
}

//Inicializace OpenGL vykreslovani a programovych dat
int InitGL(GLvoid)
{
    //Nacteni klientskych databazi
    gDataStore.LoadEFCDS();

    //Inicializace zobrazovaci tridy
    gDisplay.Initialize();
    //Inicializace uzivatelskeho rozhrani
    gInterface.Initialize();
    //Inicializace casovace
    gTimer.Initialize();

    glEnable(GL_TEXTURE_2D);        //Povoleni texturovani
    glShadeModel(GL_SMOOTH);        //Model shadingu
    glClearColor(0.0f, 0.0f, 1.0f, 0.0f);    //Barva pozadi (default, zde modra)
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT, GL_FILL);
    glPolygonMode(GL_BACK, GL_FILL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);    //Nejlepsi perspektivni korekce
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);

    //Osvetleni, bude jeste prepracovano
    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);

    //Nacteni fontu
    BuildFont();

    return TRUE;
}

clock_t lasttime;

//Vykresleni sceny
//Hlavni cast probehne ve tride Display
int DrawGLScene(GLvoid)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    clock_t diff = clock()- lasttime;
    lasttime = clock();
    gDisplay.SetDiff(diff);
    gDisplay.DoTick();
    gTimer.Update(diff);
    gEmitterMgr.Update(diff);
    gTextureAnimationMgr.Update(diff);

    return TRUE;
}

GLvoid KillGLWindow(GLvoid)
{
    if(fullscreen)
    {
        ChangeDisplaySettings(NULL,0);
        ShowCursor(TRUE);
    }

    if(hRC)
    {
        if(!wglMakeCurrent(NULL,NULL))
        {
            MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        }

        if(!wglDeleteContext(hRC))
        {
            MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        }
        hRC = NULL;
    }

    if(hDC && !ReleaseDC(hWnd,hDC))
    {
        MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        hDC = NULL;
    }

    if(hWnd && !DestroyWindow(hWnd))
    {
        MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        hWnd = NULL;
    }

    if(!UnregisterClass("OpenGL",hInstance))
    {
        MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        hInstance = NULL;
    }
}
 
BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
    GLuint PixelFormat;
    WNDCLASS wc;
    DWORD dwExStyle;
    DWORD dwStyle;
    RECT WindowRect;
    WindowRect.left = (long)0;
    WindowRect.right = (long)width;
    WindowRect.top = (long)0;
    WindowRect.bottom = (long)height;

    fullscreen = fullscreenflag;

    hInstance            = GetModuleHandle(NULL);
    wc.style            = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc        = (WNDPROC) WndProc;
    wc.cbClsExtra        = 0;
    wc.cbWndExtra        = 0;
    wc.hInstance        = hInstance;
    wc.hIcon            = LoadIcon(NULL, IDI_WINLOGO);
    wc.hCursor            = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = NULL;
    wc.lpszMenuName        = NULL;
    wc.lpszClassName    = "OpenGL";

    if(!RegisterClass(&wc))
    {
        MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    if(fullscreen)
    {
        DEVMODE dmScreenSettings;
        memset(&dmScreenSettings,0,sizeof(dmScreenSettings));
        dmScreenSettings.dmSize=sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth    = width;
        dmScreenSettings.dmPelsHeight    = height;
        dmScreenSettings.dmBitsPerPel    = bits;
        dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

        if(ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
        {
            if(MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","ERROR",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
            {
                fullscreen = FALSE;
            }
            else
            {
                MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
                return FALSE;
            }
        }
    }

    if(fullscreen)
    {
        dwExStyle = WS_EX_APPWINDOW;
        dwStyle = WS_POPUP;
        ShowCursor(FALSE);
    }
    else
    {
        dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        dwStyle = WS_OVERLAPPEDWINDOW;
    }

    AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

    if (!(hWnd = CreateWindowEx(dwExStyle,
                                "OpenGL",
                                title,
                                dwStyle |
                                WS_CLIPSIBLINGS |
                                WS_CLIPCHILDREN,
                                0, 0,
                                WindowRect.right-WindowRect.left,
                                WindowRect.bottom-WindowRect.top,
                                NULL,
                                NULL,
                                hInstance,
                                NULL)))
    {
        KillGLWindow();
        MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    static PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        bits,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        16,
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    if(!(hDC = GetDC(hWnd)))
    {
        KillGLWindow();
        MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    if(!(PixelFormat = ChoosePixelFormat(hDC,&pfd)))
    {
        KillGLWindow();
        MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    if(!SetPixelFormat(hDC,PixelFormat,&pfd))
    {
        KillGLWindow();
        MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    if(!(hRC = wglCreateContext(hDC)))
    {
        KillGLWindow();
        MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    if(!wglMakeCurrent(hDC,hRC))
    {
        KillGLWindow();
        MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    ShowWindow(hWnd,SW_SHOW);
    SetForegroundWindow(hWnd);
    SetFocus(hWnd);
    ReSizeGLScene(width, height);

    if(!InitGL())
    {
        KillGLWindow();
        MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    return TRUE;
}

LRESULT CALLBACK WndProc(    HWND    hWnd,
                            UINT    uMsg,
                            WPARAM    wParam,
                            LPARAM    lParam)
{
    switch (uMsg)
    {
        case WM_ACTIVATE:
        {
            if (!HIWORD(wParam))
            {
                active=TRUE;
            }
            else
            {
                active=FALSE;
            }

            return 0;
        }

        case WM_SYSCOMMAND:
        {
            switch (wParam)
            {
                case SC_SCREENSAVE:
                case SC_MONITORPOWER:
                return 0;
            }
            break;
        }

        case WM_CLOSE:
        {
            PostQuitMessage(0);
            return 0;
        }

        case WM_KEYDOWN:
        {
            gInterface.KeyPress(wParam);
            return 0;
        }
        case WM_KEYUP:
        {
            gInterface.KeyRelease(wParam);
            return 0;
        }

        case WM_LBUTTONDOWN:
        {
            gInterface.MouseBtnPress(MOUSE_LEFT, LOWORD(lParam), HIWORD(lParam));
            return 0;
        }
        case WM_LBUTTONUP:
        {
            gInterface.MouseBtnRelease(MOUSE_LEFT);
            return 0;
        }

        case WM_MBUTTONDOWN:
        {
            gInterface.MouseBtnPress(MOUSE_MIDDLE, LOWORD(lParam), HIWORD(lParam));
            return 0;
        }
        case WM_MBUTTONUP:
        {
            gInterface.MouseBtnRelease(MOUSE_MIDDLE);
            return 0;
        }

        case WM_RBUTTONDOWN:
        {
            gInterface.MouseBtnPress(MOUSE_RIGHT, LOWORD(lParam), HIWORD(lParam));
            return 0;
        }
        case WM_RBUTTONUP:
        {
            gInterface.MouseBtnRelease(MOUSE_RIGHT);
            return 0;
        }

        case WM_MOUSEMOVE:
        {
            gInterface.MouseMove(LOWORD(lParam), HIWORD(lParam));
            return 0;
        }

        case WM_SIZE:
        {
            ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));
            return 0;
        }
    }

    return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

int WINAPI WinMain(    HINSTANCE    hInstance,
                    HINSTANCE    hPrevInstance,
                    LPSTR        lpCmdLine,
                    int            nCmdShow)
{
    MSG msg;
    BOOL done = FALSE;

    LoadConfig();

    srand ( (unsigned int)time(NULL) );

    if(!CreateGLWindow("Bomberman 3D",gConfig.WindowWidth,gConfig.WindowHeight,gConfig.ColorDepth,gConfig.fullscreen))
    {
        return 0;
    }

    // TODO: our own cursor
    ShowCursor(true);

    while(!done)
    {
        if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
        {
            if (msg.message==WM_QUIT)
            {
                done=TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            //uint64 StartTime = gTimer.GetTime();

            // Nutne prepracovat s podporou vice vlaken!
            //while(gTimer.GetTime() < StartTime + float(steps[0] * 2.0f))
            //{
            //}

            if((active && !DrawGLScene()) || gInterface.IsKeyPressed(VK_ESCAPE))
            {
                done=TRUE;
            }
            else
            {
                SwapBuffers(hDC);
                gInterface.Update();
            }
        }
    }

    gDisplay.UnloadMap();
    KillGLWindow();
    KillFont();
    return (msg.wParam);
}

