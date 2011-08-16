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

void DrawUIElement(uint32 textureId, float left, float top, float width, float height)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D,gDisplayStore.FloorTextures[textureId]);

    glBegin(GL_QUADS);
      glColor3ub(255, 255, 255);
        glTexCoord2f(1.0f, 0.0f); glVertex2d(left+width, top+0);
        glTexCoord2f(1.0f, 1.0f); glVertex2d(left+width, top+height);
        glTexCoord2f(0.0f, 1.0f); glVertex2d(left+0, top+height);
        glTexCoord2f(0.0f, 0.0f); glVertex2d(left+0, top+0);
    glEnd();
}

/// Handler vykresleni menu - nova hra
void MenuDrawHandler_NewGame()
{
    DrawUIElement(4, 90*WIDTH_PCT-340, 10*HEIGHT_PCT, 340, 80);
}
void MenuDrawHandler_ExitGame()
{
    DrawUIElement(11, 90*WIDTH_PCT-340, 10*HEIGHT_PCT+80+30, 340, 80);
}
/// Handler osetreni kliknuti menu
bool MenuClickHandler(unsigned int x, unsigned int y, MouseButton button, bool press)
{
    // Nova Hra
    if ( (x > 90*WIDTH_PCT-340) && (x < 90*WIDTH_PCT) && (y > 10*HEIGHT_PCT) && (y < 10*HEIGHT_PCT+80) )
    {
        gDisplay.SetGameState(GAME_PRECONNECTING);
        gNetwork.Connect("127.0.0.1",2530);
        if (gNetwork.IsConnected())
        {
            SmartPacket data(CMSG_INITIATE_SESSION);
            data << uint32(0);
            gNetwork.SendPacket(&data);
        }
        return true;
    }
    // Odejit
    else if ( (x > 90*WIDTH_PCT-340) && (x < 90*WIDTH_PCT) && (y > 10*HEIGHT_PCT+80+30) && (y < 10*HEIGHT_PCT+80+80+30) )
    {
        exit(0);
        return true;
    }

    return false; // Neosetrili jsme
}

void PreConnectingDrawHandler()
{
    switch(gDisplay.GetGameStateStage())
    {
        case 0:
            gDisplay.DrawText(100,100,"Pøipojování...");
            break;
        case 1:
            gDisplay.DrawText(150,150,"Ovìøování...");
            break;
    }
}

void ConnectingDrawHandler_NicknameField()
{
    DrawUIElement(12, 20, 20, 500, 80);

    UIRecord* pField = gInterface.GetUIRecordByType(UI_TYPE_CONNECTING_FIELD_NICKNAME);
    if (!pField)
        return;

    gDisplay.DrawText(52,82,"%s",pField->fieldcontent.c_str());
}
void ConnectingDrawHandler_RoomList()
{
    DrawUIElement(13, 20, 120, 200, 40);
    // Textura 14 = cervena 40% alpha, 15 = modra 40% alpha

    DrawUIElement(15, 5*WIDTH_PCT, 180, 90*WIDTH_PCT, 100*HEIGHT_PCT-180-10*HEIGHT_PCT);

    UIRecord* pField = gInterface.GetUIRecordByType(UI_TYPE_CONNECTING_FIELD_ROOMS_LIST);
    if (!pField)
        return;

    // Priklad
    //pField->fieldcontent = "1|Jmeno hry 1|0/4|Mapa 1|2|Druha hra|4/4|Mapa 2";

    // Priblizne se prepocita podle vysky okna (18 je pro 768px na vysku okna)
    std::vector<string> exploded = explode(pField->fieldcontent.c_str(),'|');
    for (int i = 0; (i < exploded.size()/4) && (i < (18*gConfig.WindowHeight/768)); i++)
    {
        gDisplay.DrawText(5*WIDTH_PCT+10, 194.0f+i*27,"%s",exploded[(i*4)+0].c_str());
        gDisplay.DrawText(10*WIDTH_PCT  , 194.0f+i*27,"%s",exploded[(i*4)+1].c_str());
        gDisplay.DrawText(50*WIDTH_PCT  , 194.0f+i*27,"%s",exploded[(i*4)+2].c_str());
        gDisplay.DrawText(70*WIDTH_PCT  , 194.0f+i*27,"%s",exploded[(i*4)+3].c_str());
    }

    DrawUIElement(14, 5*WIDTH_PCT, 190.0f+pField->store[0]*27-2, 90*WIDTH_PCT, 28);

    DrawUIElement(16, 95*WIDTH_PCT-200, 92*HEIGHT_PCT, 200, 40);
}
bool ConnectingClickHandler(unsigned int x, unsigned int y, MouseButton button, bool press)
{
    if (!press)
        return false;

    UIRecord* pNickField = gInterface.GetUIRecordByType(UI_TYPE_CONNECTING_FIELD_NICKNAME);
    if (!pNickField)
        return false;

    if (x > 20 && x < 20+500 && y > 20 && y < 20+80)
    {
        if (!pNickField->active)
        {
            pNickField->active = true;
            pNickField->fieldcontent.append(" |");
        }
    }
    else
    {
        if (pNickField->active)
        {
            pNickField->fieldcontent.erase(pNickField->fieldcontent.size()-1);
            pNickField->fieldcontent.erase(pNickField->fieldcontent.size()-1);
            pNickField->active = false;
        }
    }

    UIRecord* pField = gInterface.GetUIRecordByType(UI_TYPE_CONNECTING_FIELD_ROOMS_LIST);
    if (!pField)
        return false;

    if (x > 5*WIDTH_PCT && x < 95*WIDTH_PCT && y > 190 && ((y < 190+(18*gConfig.WindowHeight/768)*27) && y < 90*HEIGHT_PCT))
    {
        uint32 pos = (y-190)/27;

        std::vector<string> exploded = explode(pField->fieldcontent.c_str(),'|');
        if ((exploded.size()/4) > pos)
            pField->store[0] = (y-190)/27;
    }

    if (x > 95*WIDTH_PCT-200 && x < 95*WIDTH_PCT && y > 92*HEIGHT_PCT && y < 92*HEIGHT_PCT+40)
    {
        SmartPacket data(CMSG_ENTER_GAME);
        data << uint32(pField->store[0]);
        if (pNickField->active)
        {
            // Smazat z konce " |"
            pNickField->fieldcontent.erase(pNickField->fieldcontent.size()-1);
            pNickField->fieldcontent.erase(pNickField->fieldcontent.size()-1);
        }
        data << pNickField->fieldcontent.c_str();
        gNetwork.SendPacket(&data);
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
UIRecord* DHInstall_UIPreConnecting_Info()
{
    UIRecord* pNew = new UIRecord;

    pNew->x = 0;
    pNew->y = 0;
    pNew->StateRestriction = GAME_PRECONNECTING;
    pNew->UIType = UI_TYPE_PRECONNECTING_INFO;
    pNew->DrawHandler = &PreConnectingDrawHandler;
    //pNew->ClickHandler = &PreConnectingClickHandler;

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
