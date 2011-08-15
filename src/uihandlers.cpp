//------------------------------- uihandlers.cpp --------------------------
#include <game_inc.h>

/* Soubor obsahujici pouze handlery pro jednotlive prvky UI a jejich
 * "instalacni" funkce, ktere jen vrati zinicializovany ukazatel na
 * pripraveny a vyplneny UIRecord.
 */

/*
 Poznamka pro vykresleni textu:
 X range zhruba <-0.055f;0.055f> (odleva doprava)
 Y range zhruba <-0.042f;0.042f> (odspoda nahoru)
*/

/// Handler vykresleni menu - nova hra
void MenuDrawHandler_NewGame()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D,gDisplayStore.FloorTextures[4]);

    glBegin(GL_QUADS);
      glColor3ub(255, 255, 255);
        GLfloat lpos = 90*WIDTH_PCT-340; // pozice od leva
        GLfloat bpos = 90*HEIGHT_PCT-80; // pozice od spoda
        glTexCoord2f(1.0f, 1.0f); glVertex2d(lpos+340, bpos+0);
        glTexCoord2f(1.0f, 0.0f); glVertex2d(lpos+340, bpos+80);
        glTexCoord2f(0.0f, 0.0f); glVertex2d(lpos+0, bpos+80);
        glTexCoord2f(0.0f, 1.0f); glVertex2d(lpos+0, bpos+0);
    glEnd();
}
void MenuDrawHandler_ExitGame()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D,gDisplayStore.FloorTextures[11]);

    glBegin(GL_QUADS);
      glColor3ub(255, 255, 255);
        GLfloat lpos = 90*WIDTH_PCT-340; // pozice od leva
        GLfloat bpos = 90*HEIGHT_PCT-80-80-30; // pozice od spoda
        glTexCoord2f(1.0f, 1.0f); glVertex2d(lpos+340, bpos+0);
        glTexCoord2f(1.0f, 0.0f); glVertex2d(lpos+340, bpos+80);
        glTexCoord2f(0.0f, 0.0f); glVertex2d(lpos+0, bpos+80);
        glTexCoord2f(0.0f, 1.0f); glVertex2d(lpos+0, bpos+0);
    glEnd();
}
/// Handler osetreni kliknuti menu
bool MenuClickHandler(unsigned int x, unsigned int y, MouseButton button, bool press)
{
    // Nova Hra
    if ( (x > 90*WIDTH_PCT-340) && (x < 90*WIDTH_PCT) && (y > 90*HEIGHT_PCT-80) && (y < 90*HEIGHT_PCT) )
    {
        //gDisplay.FlushModelDisplayList();
        //gDisplay.InitModelDisplayList();
        gDisplay.SetGameState(GAME_CONNECTING);
        return true;
    }
    // Odejit
    else if ( (x > 90*WIDTH_PCT-340) && (x < 90*WIDTH_PCT) && (y > 90*HEIGHT_PCT-80-80-30) && (y < 90*HEIGHT_PCT-80-30) )
    {
        exit(0);
        return true;
    }

    return false; // Neosetrili jsme
}

void ConnectingDrawHandler_NicknameField()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D,gDisplayStore.FloorTextures[12]);

    glBegin(GL_QUADS);
      glColor3ub(255, 255, 255);
        GLfloat lpos = 20; // pozice od leva
        GLfloat bpos = 670; // pozice od spoda
        glTexCoord2f(1.0f, 1.0f); glVertex2d(lpos+500, bpos+0);
        glTexCoord2f(1.0f, 0.0f); glVertex2d(lpos+500, bpos+80);
        glTexCoord2f(0.0f, 0.0f); glVertex2d(lpos+0, bpos+80);
        glTexCoord2f(0.0f, 1.0f); glVertex2d(lpos+0, bpos+0);
    glEnd();

    UIRecord* pField = gInterface.GetUIRecordByType(UI_TYPE_CONNECTING_FIELD_NICKNAME);
    if (!pField)
        return;

    gDisplay.DrawText(-0.049f,0.033f,"%s",pField->fieldcontent.c_str());
}
void ConnectingDrawHandler_RoomList()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D,gDisplayStore.FloorTextures[13]);

    glBegin(GL_QUADS);
      glColor3ub(255, 255, 255);
        GLfloat lpos = 20; // pozice od leva
        GLfloat bpos = 570; // pozice od spoda
        glTexCoord2f(1.0f, 1.0f); glVertex2d(lpos+200, bpos+0);
        glTexCoord2f(1.0f, 0.0f); glVertex2d(lpos+200, bpos+40);
        glTexCoord2f(0.0f, 0.0f); glVertex2d(lpos+0, bpos+40);
        glTexCoord2f(0.0f, 1.0f); glVertex2d(lpos+0, bpos+0);
    glEnd();

    // Textura 14 = cervena 40% alpha, 15 = modra 40% alpha

    glBindTexture(GL_TEXTURE_2D,gDisplayStore.FloorTextures[15]);
    glBegin(GL_QUADS);
      glColor3ub(255, 255, 255);
        lpos = 5*WIDTH_PCT; // pozice od leva
        bpos = 550; // pozice od spoda
        glTexCoord2f(1.0f, 1.0f); glVertex2d(95*WIDTH_PCT, bpos+0);
        glTexCoord2f(1.0f, 0.0f); glVertex2d(95*WIDTH_PCT, 10*HEIGHT_PCT);
        glTexCoord2f(0.0f, 0.0f); glVertex2d(lpos+0, 10*HEIGHT_PCT);
        glTexCoord2f(0.0f, 1.0f); glVertex2d(lpos+0, bpos+0);
    glEnd();

    glBindTexture(GL_TEXTURE_2D,gDisplayStore.FloorTextures[16]);
    glBegin(GL_QUADS);
      glColor3ub(255, 255, 255);
        lpos = 95*WIDTH_PCT-200; // pozice od leva
        bpos = 8*HEIGHT_PCT-40; // pozice od spoda
        glTexCoord2f(1.0f, 1.0f); glVertex2d(lpos+200, bpos+0);
        glTexCoord2f(1.0f, 0.0f); glVertex2d(lpos+200, bpos+40);
        glTexCoord2f(0.0f, 0.0f); glVertex2d(lpos+0, bpos+40);
        glTexCoord2f(0.0f, 1.0f); glVertex2d(lpos+0, bpos+0);
    glEnd();
}
bool ConnectingClickHandler(unsigned int x, unsigned int y, MouseButton button, bool press)
{
    if (!press)
        return false;

    UIRecord* pField = gInterface.GetUIRecordByType(UI_TYPE_CONNECTING_FIELD_NICKNAME);
    if (!pField)
        return false;

    if (x > 20 && x < 20+500 && y > 670 && y < 670+80)
    {
        if (!pField->active)
        {
            pField->active = true;
            pField->fieldcontent.append(" |");
        }
    }
    else
    {
        if (pField->active)
        {
            pField->fieldcontent.erase(pField->fieldcontent.size()-1);
            pField->fieldcontent.erase(pField->fieldcontent.size()-1);
            pField->active = false;
        }
    }

    if (x > 95*WIDTH_PCT-200 && x < 95*WIDTH_PCT && y > 8*HEIGHT_PCT-40 && y < 8*HEIGHT_PCT)
    {
        // TODO:  handling do network vlakna, zde jen poslat packet!
        gDisplay.FlushModelDisplayList();
        gDisplay.InitModelDisplayList();
        gDisplay.SetGameState(GAME_GAME);
    }

    return true;
}
bool ConnectingKeyStateHandler(uint16 key, bool press)
{
    UIRecord* pField = gInterface.GetUIRecordByType(UI_TYPE_CONNECTING_FIELD_NICKNAME);
    if (!pField)
        return false;

    if (press && pField->active)
    {
        // Backspace
        if (key == 8)
        {
            if (pField->fieldcontent.size() > 2)
            {
                pField->fieldcontent.erase(pField->fieldcontent.size()-1);
                pField->fieldcontent.erase(pField->fieldcontent.size()-1);
                pField->fieldcontent.erase(pField->fieldcontent.size()-1);
                pField->fieldcontent.append(" |");
            }
        }
        else
        {
            if (pField->fieldcontent.size() > 20)
                return true;

            // 0-9, /-+* , A-Z, a-z
            if ((key >= 40) || (key >= 65 && key <= 90) || (key >= 97 && key <= 122))
            {
                pField->fieldcontent.erase(pField->fieldcontent.size()-1);
                pField->fieldcontent.erase(pField->fieldcontent.size()-1);
                pField->fieldcontent.push_back((uint8)key);
                pField->fieldcontent.append(" |");
            }
        }

        return true;
    }

    return false;
}

/// Instalacni funkce pro jeden prvek (menu)
UIRecord* DHInstall_UIMenu_NewGame()
{
    // Nejdrive je nutne vytvorit si ukazatel a pridelit mu misto v pameti
    UIRecord* pNew = new UIRecord;

    // Nasledne musime vyplnit souradnice (absolutne)
    pNew->x = 0;
    pNew->y = 0;
    // Restrikce pro status hry
    pNew->StateRestriction = GAME_MENU;
    // UI typ zaznamu
    pNew->UIType = UI_TYPE_MENU_BUTTON_NEW_GAME;
    // Pote nesmime zapomenout priradit ukazatel na handler vykresleni
    pNew->DrawHandler = &MenuDrawHandler_NewGame;
    // A handler kliknuti
    pNew->ClickHandler = &MenuClickHandler;

    // Vracime ukazatel na nami vytvoreny prvek
    return pNew;
}
UIRecord* DHInstall_UIMenu_ExitGame()
{
    UIRecord* pNew = new UIRecord;

    pNew->x = 0;
    pNew->y = 0;
    pNew->StateRestriction = GAME_MENU;
    pNew->UIType = UI_TYPE_MENU_BUTTON_EXIT_GAME;
    pNew->DrawHandler = &MenuDrawHandler_ExitGame;
    pNew->ClickHandler = &MenuClickHandler;

    return pNew;
}
UIRecord* DHInstall_UIConnecting_NicknameField()
{
    UIRecord* pNew = new UIRecord;

    pNew->x = 0;
    pNew->y = 0;
    pNew->StateRestriction = GAME_CONNECTING;
    pNew->UIType = UI_TYPE_CONNECTING_FIELD_NICKNAME;
    pNew->DrawHandler = &ConnectingDrawHandler_NicknameField;
    pNew->ClickHandler = &ConnectingClickHandler;
    pNew->KeyStateHandler = &ConnectingKeyStateHandler;

    return pNew;
}
UIRecord* DHInstall_UIConnecting_RoomList()
{
    UIRecord* pNew = new UIRecord;

    pNew->x = 0;
    pNew->y = 0;
    pNew->StateRestriction = GAME_CONNECTING;
    pNew->UIType = UI_TYPE_CONNECTING_FIELD_ROOMS_LIST;
    pNew->DrawHandler = &ConnectingDrawHandler_RoomList;
    pNew->ClickHandler = &ConnectingClickHandler;
    pNew->KeyStateHandler = &ConnectingKeyStateHandler;

    return pNew;
}
