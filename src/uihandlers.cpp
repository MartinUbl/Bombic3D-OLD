//------------------------------- uihandlers.cpp --------------------------
#include <game_inc.h>

/* Soubor obsahujici pouze handlery pro jednotlive prvky UI a jejich
 * "instalacni" funkce, ktere jen vrati zinicializovany ukazatel na
 * pripraveny a vyplneny UIRecord.
 */

/// Testovaci handler vykresleni (ukazkovy)
void TestDrawHandler()
{
    glBegin(GL_QUADS);
      glColor3ub(255, 255, 255);
      glBindTexture(GL_TEXTURE_2D,gDisplayStore.FloorTextures[0]);
        glTexCoord2f(1.0f, 1.0f); glVertex2d(0, 0);
        glTexCoord2f(1.0f, 0.0f); glVertex2d(0,100);
        glTexCoord2f(0.0f, 0.0f); glVertex2d(100, 100);
        glTexCoord2f(0.0f, 1.0f); glVertex2d(100,0);
    glEnd();
}
/// Testovaci handler osetreni kliknuti(ukazkovy)
bool TestClickHandler(unsigned int x, unsigned int y, MouseButton button, bool press)
{
    return false; // Neosetrili jsme
}

/// Testovaci instalacni funkce pro jeden prvek (ukazkovy)
UIRecord* DHInstall_UITest()
{
    // Nejdrive je nutne vytvorit si ukazatel a pridelit mu misto v pameti
    UIRecord* pNew = new UIRecord;

    // Nasledne musime vyplnit souradnice (absolutne)
    pNew->x = 0;
    pNew->y = 0;
    // Pote nesmime zapomenout priradit ukazatel na handler vykresleni
    pNew->DrawHandler = &TestDrawHandler;
    // A handler kliknuti
    pNew->ClickHandler = &TestClickHandler;

    // Vracime ukazatel na nami vytvoreny prvek
    return pNew;
}

