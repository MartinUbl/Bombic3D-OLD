//------------------------------- uihandlers.cpp --------------------------
#include <game_inc.h>

/* Soubor obsahujici pouze handlery pro jednotlive prvky UI a jejich
 * "instalacni" funkce, ktere jen vrati zinicializovany ukazatel na
 * pripraveny a vyplneny UIRecord.
 */

/// Handler vykresleni menu
void MenuDrawHandler()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D,gDisplayStore.FloorTextures[4]);

    glBegin(GL_QUADS);
      glColor3ub(255, 255, 255);
        uint32 lpos = 400; // pozice od leva
        uint32 bpos = 400; // pozice od spoda
        glTexCoord2f(1.0f, 1.0f); glVertex2d(lpos+340, bpos+0);
        glTexCoord2f(1.0f, 0.0f); glVertex2d(lpos+340, bpos+80);
        glTexCoord2f(0.0f, 0.0f); glVertex2d(lpos+0, bpos+80);
        glTexCoord2f(0.0f, 1.0f); glVertex2d(lpos+0, bpos+0);
    glEnd();
}
/// Handler osetreni kliknuti menu
bool MenuClickHandler(unsigned int x, unsigned int y, MouseButton button, bool press)
{
    if ( (x > 400) && (x < 400+340) && (y > 400) && (y < 400+80) )
    {
        gDisplay.FlushModelDisplayList();
        gDisplay.InitModelDisplayList();
        gDisplay.SetGameState(GAME_GAME);
        return true;
    }

    return false; // Neosetrili jsme
}

/// Instalacni funkce pro jeden prvek (menu)
UIRecord* DHInstall_UIMenu()
{
    // Nejdrive je nutne vytvorit si ukazatel a pridelit mu misto v pameti
    UIRecord* pNew = new UIRecord;

    // Nasledne musime vyplnit souradnice (absolutne)
    pNew->x = 0;
    pNew->y = 0;
    // Restrikce pro status hry
    pNew->StateRestriction = GAME_MENU;
    // Pote nesmime zapomenout priradit ukazatel na handler vykresleni
    pNew->DrawHandler = &MenuDrawHandler;
    // A handler kliknuti
    pNew->ClickHandler = &MenuClickHandler;

    // Vracime ukazatel na nami vytvoreny prvek
    return pNew;
}

