#ifndef _CONTROL_H__
#define _CONTROL_H__

#include <game_inc.h>
#include "drawing.h"

enum MouseButton
{
    MOUSE_LEFT,
    MOUSE_MIDDLE,
    MOUSE_RIGHT,
    MOUSE_BUTTONS_MAX
};

enum UIRecordType
{
    UI_TYPE_NONE = 0,
    UI_TYPE_MENU_BUTTON_NEW_GAME,
    UI_TYPE_MENU_BUTTON_EXIT_GAME,
    UI_TYPE_PRECONNECTING_INFO,
    UI_TYPE_CONNECTING_FIELD_NICKNAME,
    UI_TYPE_CONNECTING_FIELD_ROOMS_LIST,
    UI_TYPE_CONNECTING_BUTTON_CONNECT,
};

#define UI_TYPE_MENU_START UI_TYPE_MENU_BUTTON_NEW_GAME
#define UI_TYPE_MENU_END   UI_TYPE_MENU_BUTTON_EXIT_GAME
#define UI_TYPE_CONNECTING_START UI_TYPE_CONNECTING_FIELD_NICKNAME
#define UI_TYPE_CONNECTING_END   UI_TYPE_CONNECTING_BUTTON_CONNECT

struct UIRecord
{
    UIRecord()
    {
        StateRestriction = GAME_NONE;
        UIType = UI_TYPE_NONE;
        DrawHandler = NULL;
        ClickHandler = NULL;
        KeyStateHandler = NULL;
        active = false;
        fieldcontent = "";
        for (int i = 0; i < 4; i++)
            store[i] = 0;
    };
    short id;
    uint32 x,y;
    GameState StateRestriction;
    UIRecordType UIType;
    bool active; // Prevazne jen pro prvky typu FIELD, pro indikaci, zdali se do nich pise nebo ne
    std::string fieldcontent; // Opet jen pro prvky typu FIELD
    uint16 store[4]; // 5 uloznych promennych
    void (*DrawHandler)(void);
    bool (*ClickHandler)(uint32 x, uint32 y, MouseButton button, bool press);
    bool (*KeyStateHandler)(uint16 key, bool press);
};

#define WIDTH_PCT  (float(gConfig.WindowWidth)/100.0f)
#define HEIGHT_PCT (float(gConfig.WindowHeight)/100.0f)

class Interface
{
public:
    Interface()
    {
        for(int i=0;i<256;++i) //Vynulovat veskere klavesy
            keys[i] = false;
        for(int i=0;i<MOUSE_BUTTONS_MAX;++i)
            mousebtn[i] = false;
    };
    ~Interface() {};

    void Initialize();
    void RegisterUIRecord(UIRecord* rec);
    void LoadUI();
    void Draw();
    UIRecord* GetUIRecordByType(UIRecordType type);

    void KeyPress(uint8 key);
    void KeyRelease(uint8 key);
    void MouseBtnPress(MouseButton btn, uint32 x, uint32 y);
    void MouseBtnRelease(MouseButton btn);
    void MouseMove(uint32 x, uint32 y);
    void Update();

    bool IsKeyPressed(uint8 key)
    {
        return keys[key];
    }
private:
    std::map<uint32, UIRecord*> pUIRecords;
    uint32 numrecords;

    bool keys[256];
    bool mousebtn[MOUSE_BUTTONS_MAX];
    uint32 mousedown_x, mousedown_y;
};

extern Interface gInterface;

// Externi definice install funkci UI prvku
extern UIRecord* DHInstall_UIMenu_NewGame();
extern UIRecord* DHInstall_UIMenu_ExitGame();
extern UIRecord* DHInstall_UIConnecting_NicknameField();
extern UIRecord* DHInstall_UIConnecting_RoomList();
extern UIRecord* DHInstall_UIPreConnecting_Info();

#endif

