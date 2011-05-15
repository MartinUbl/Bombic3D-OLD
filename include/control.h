#ifndef _CONTROL_H__
#define _CONTROL_H__

#include <game_inc.h>

enum MouseButton
{
    MOUSE_LEFT,
    MOUSE_MIDDLE,
    MOUSE_RIGHT,
    MOUSE_BUTTONS_MAX
};

struct UIRecord
{
    short id;
    uint32 x,y;
    void (*DrawHandler)(void);
    bool (*ClickHandler)(uint32 x, uint32 y, MouseButton button, bool press);
};

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
extern UIRecord* DHInstall_UITest();

#endif

