//------------------------------- interface.cpp ---------------------------
#include <game_inc.h>

/*
 * Zde by se v budoucnu mìla mìnit funkce LoadUI - vše ostatní týkající se
 * uživatelského rozhraní je obsaženo v souboru uihandlers.cpp
 */

/*************************************************************************/
/* Interface                                                             */
/*************************************************************************/

/// Deklarace globalniho interface
Interface gInterface;

void Interface::Initialize()
{
    // Vynulovani poctu zaznamu
    numrecords = 0;

    // Nacteni vsech prvku uzivatelskeho rozhrani
    LoadUI();
}

void Interface::Draw()
{
    // Priprava a prechod do 2D projekce
    int vPort[4];
    glGetIntegerv(GL_VIEWPORT, vPort);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(vPort[0], vPort[0]+vPort[2], vPort[1]+vPort[3], vPort[1], -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0.375f, 0.375f, 0.0f);

    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_TEXTURE_2D);

    /* Vykresleni vsech UI prvku pomoci jejich draw handleru
     * Vyjimkou muze byt napriklad fixne ulozeny mizejici prvek, tomu
     * se pak priradi zaporne ID a neni vykreslen
     *
     * UI prvek nemusi byt nutne vykreslen, pokud se DrawHandler == NULL
     */
    for(int i = 0; i < numrecords; i++)
        if(pUIRecords[i] >= 0 && pUIRecords[i]->DrawHandler != NULL)
            if((pUIRecords[i]->StateRestriction == 0) || (gDisplay.GetGameState() == pUIRecords[i]->StateRestriction))
                pUIRecords[i]->DrawHandler();



    // Prechod zpatky do 3D, vraceni matrixu do puvodnich parametru
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);    gDisplay.DrawTexts();
}

UIRecord* Interface::GetUIRecordByType(UIRecordType type)
{
    if (pUIRecords.empty())
        return NULL;

    for (std::map<uint32, UIRecord*>::iterator itr = pUIRecords.begin(); itr != pUIRecords.end(); ++itr)
    {
        if ((*itr).second->UIType == type)
            return (*itr).second;
    }

    return NULL;
}

void Interface::RegisterUIRecord(UIRecord* rec)
{
    /* Nejdrive se ulozi cislo zaznamu do ID rekordu (pro zpetnou manipulaci),
     * nasledne se ulozi zaznam do mapy zaznamu, a nakonec se inkrementuje celk.
     * pocet prvku
     *
     * V budoucnu nutne osetrit overovani prvku s ID < 0, pripravenych k prepsani
     */
    rec->id = numrecords;
    pUIRecords[numrecords] = rec;
    numrecords++;
}

void Interface::LoadUI()
{
    /* Zaregistrovani vsech dostupnych a potrebnych UI prvku (nektere mohou
     * pribyt az za behu programu (docasne, mizejici, atp..)
     */
    RegisterUIRecord(DHInstall_UIMenu_NewGame());
    RegisterUIRecord(DHInstall_UIMenu_ExitGame());
    RegisterUIRecord(DHInstall_UIConnecting_NicknameField());
    RegisterUIRecord(DHInstall_UIConnecting_RoomList());
}

