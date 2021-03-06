#include <game_inc.h>
#include <math.h>
#include <gameplay.h>
#include <gl/glut.h>

std::locale loc;

//Pri stisknuti klavesy
void Interface::KeyPress(uint8 key)
{
    if(!keys[key])         //Jen pro kontrolu
    {
        //Nastavi promennou v arrayi klaves na true, aby bylo mozne
        //pomoci fci KeyRelease a Update dale prebirat klavesy
        keys[key] = true;

        uint16 uikey = key;
        if (key >= 65 && key <= 90)
        {
            if (keys[16])
                uikey = std::toupper(key, loc);
            else
                uikey = std::tolower(key, loc);
        }
        // Namapovat numerickou klavesnici na cisla
        else if (key >= 96 && key <= 97+14)
        {
            //                     96   97   98   99  100  101  102  103  104  105  106  107  108  109  110  111
            const char mapped[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '+', '-', '-', ',', '/'};
            uikey = mapped[key-96];
        }

        for(int i = 0; i < numrecords; i++)
            if(pUIRecords[i] >= 0 && pUIRecords[i]->KeyStateHandler != NULL)
                if((pUIRecords[i]->StateRestriction == 0) || (gDisplay.GetGameState() == pUIRecords[i]->StateRestriction))
                    if(pUIRecords[i]->KeyStateHandler(uikey,true))
                        break;

        //Specificke udalosti po stisknuti klavesy
        switch(key)
        {
            case 'W':
            case 'A':
            case 'S':
            case 'D':
            {
                gDisplay.SetPlayerAnim(ANIM_WALK);
                SmartPacket data(CMSG_MOVE_START);

                // 1 dopredu, 2 dozadu, 3 doleva, 4 doprava

                if (key == 'W')
                    data << uint8(1);
                else if (key == 'A')
                    data << uint8(3);
                else if (key == 'D')
                    data << uint8(4);
                else if (key == 'S')
                    data << uint8(2);

                data << gDisplay.GetViewX();
                data << gDisplay.GetViewZ();
                data << gDisplay.GetHAngle();
                gNetwork.SendPacket(&data);
                break;
            }
            default:
                break;
        }
    }
}

//Pri pusteni klavesy
void Interface::KeyRelease(uint8 key)
{
    if(keys[key])          //Opet jen pro kontrolu
    {
        //Nastavi promennou v arrayi klaves na false, opet kvuli
        //zastaveni prebirani fci Update a moznosti ji nechat znovu nastavit na true
        keys[key] = false;

        for(int i = 0; i < numrecords; i++)
            if(pUIRecords[i] >= 0 && pUIRecords[i]->KeyStateHandler != NULL)
                if((pUIRecords[i]->StateRestriction == 0) || (gDisplay.GetGameState() == pUIRecords[i]->StateRestriction))
                    if(pUIRecords[i]->KeyStateHandler(key,false))
                        break;

        //Specificke udalosti po pusteni klavesy
        switch(key)
        {
            case 'W':
            case 'A':
            case 'S':
            case 'D':
            {
                if(!keys['W'] && !keys['A'] && !keys['D'] && !keys['S'])
                    gDisplay.SetPlayerAnim(ANIM_IDLE);

                SmartPacket data(CMSG_MOVE_STOP);

                // 1 dopredu, 2 dozadu, 3 doleva, 4 doprava

                if (key == 'W')
                    data << uint8(1);
                else if (key == 'A')
                    data << uint8(3);
                else if (key == 'D')
                    data << uint8(4);
                else if (key == 'S')
                    data << uint8(2);

                data << gDisplay.GetViewX();
                data << gDisplay.GetViewZ();
                gNetwork.SendPacket(&data);

                break;
            }
            default:
                break;
        }
    }
}

void Interface::MouseBtnPress(MouseButton btn, uint32 x, uint32 y)
{
    if(!mousebtn[btn])
    {
        mousebtn[btn] = true;
        mousedown_x = x;
        mousedown_y = y;

        if (btn == MOUSE_LEFT && gDisplay.GetGameState() == GAME_GAME)
            gGamePlay.PlantBomb();

        /* Projdou se vsechny UI prvky a postupne se vyzkousi, zdali si kliknuti
         * zpracuji. Pokud ano, cyklus se prerusi, protoze staci osefovat kliknuti
         * jednim handlerem.
         *
         * UI prvek nemusi mit nutne handler pro kliknut�, pokud se ClickHandler == NULL
         */
        for(int i = 0; i < numrecords; i++)
            if(pUIRecords[i] >= 0 && pUIRecords[i]->ClickHandler != NULL)
                if((pUIRecords[i]->StateRestriction == 0) || (gDisplay.GetGameState() == pUIRecords[i]->StateRestriction))
                    if(pUIRecords[i]->ClickHandler(x,y,btn,true))
                        break;
    }
}

void Interface::MouseBtnRelease(MouseButton btn)
{
    if(mousebtn[btn])
    {
        mousebtn[btn] = false;

        // Viz. vyse
        for(int i = 0; i < numrecords; i++)
            if(pUIRecords[i] >= 0 && pUIRecords[i]->ClickHandler != NULL)
                if(pUIRecords[i]->ClickHandler(0,0,btn,false))
                    break;
    }
}

void Interface::MouseMove(uint32 x, uint32 y)
{
    //Pouze pokud je stisknuto nejake mysitko. V budoucnu odstranit!
    /*for(int i = 0; i < MOUSE_BUTTONS_MAX; ++i)
    {
        if(mousebtn[i])
        {
            mousedown_x = x;
            mousedown_y = y;
            return;
        }
    }*/
    mousedown_x = x;
    mousedown_y = y;
}

//Update - pro stale pohyby
void Interface::Update()
{
    if (gDisplay.GetGameState() == GAME_GAME)
    {
        POINT mousePos;
        int middleX = gConfig.WindowWidth >> 1;
        int middleY = gConfig.WindowHeight >> 1;
        GetCursorPos(&mousePos);
        SetCursorPos(middleX, middleY);
        if (!( (mousePos.x == middleX) && (mousePos.y == middleY) ))
        {
            gDisplay.SetHAngle( 0.05f*(-mousePos.x+middleX),true);
            //Pouze v FPS !
            //gDisplay.SetVAngle( 0.05f*( mousePos.y-middleY),true);
        }

        //Posun doprava (sipka vpravo nebo D)
        //if(keys[VK_RIGHT] || keys['D'])
        //    gDisplay.SetHAngle(-DEFAULT_PITCH_SPEED * PI,true);

        //Posun doleva (sipka doleva nebo A)
        //if(keys[VK_LEFT] || keys['A'])
        //    gDisplay.SetHAngle( DEFAULT_PITCH_SPEED * PI,true);

        //Posun dopredu (sipka nahoru nebo W)
        if(keys[VK_UP] || keys['W'])
        {
            unsigned short colision = 0;
            float newx = gDisplay.GetViewX() + ((GLfloat)sin(gDisplay.GetHAngle()*(PI/180)) / 285)*(uint32(gDisplay.GetDiff())/DIFFMOD_CONTROL);
            float newy = gDisplay.GetViewY();
            float newz = gDisplay.GetViewZ();
            colision = gDisplay.CheckColision(newx,newy,newz);
            if(!(colision & AXIS_X))
                gDisplay.SetViewX(newx,false);

            newx = gDisplay.GetViewX();
            newy = gDisplay.GetViewY();
            newz = gDisplay.GetViewZ() + ((GLfloat)cos(gDisplay.GetHAngle()*(PI/180)) / 285)*(uint32(gDisplay.GetDiff())/DIFFMOD_CONTROL); //175
            colision = gDisplay.CheckColision(newx,newy,newz);
            if(!(colision & AXIS_Z))
                gDisplay.SetViewZ(newz,false);
        }

        //Posun dozadu (sipka dolu nebo S)
        if(keys[VK_DOWN] || keys['S'])
        {
            unsigned short colision = 0;
            float newx = gDisplay.GetViewX() - ((GLfloat)sin(gDisplay.GetHAngle()*(PI/180)) / 300)*(uint32(gDisplay.GetDiff())/DIFFMOD_CONTROL);
            float newy = gDisplay.GetViewY();
            float newz = gDisplay.GetViewZ();
            colision = gDisplay.CheckColision(newx,newy,newz);
            if(!(colision & AXIS_X))
                gDisplay.SetViewX(newx,false);

            newx = gDisplay.GetViewX();
            newy = gDisplay.GetViewY();
            newz = gDisplay.GetViewZ() - ((GLfloat)cos(gDisplay.GetHAngle()*(PI/180)) / 300)*(uint32(gDisplay.GetDiff())/DIFFMOD_CONTROL);
            colision = gDisplay.CheckColision(newx,newy,newz);
            if(!(colision & AXIS_Z))
                gDisplay.SetViewZ(newz,false);
        }

        //Posun ostre doprava (klavesa D)
        //Posun o 90� vice doprava oproti uhlu zobrazeni
        if(keys['D']) //puvodne E !
        {
            unsigned short colision = 0;
            float newx = gDisplay.GetViewX() + ((GLfloat)sin((gDisplay.GetHAngle()-90.0f)*(PI/180)) / 285)*(uint32(gDisplay.GetDiff())/DIFFMOD_CONTROL);
            float newy = gDisplay.GetViewY();
            float newz = gDisplay.GetViewZ();
            colision = gDisplay.CheckColision(newx,newy,newz);
            if(!(colision & AXIS_X))
                gDisplay.SetViewX(newx,false);

            newx = gDisplay.GetViewX();
            newy = gDisplay.GetViewY();
            newz = gDisplay.GetViewZ() + ((GLfloat)cos((gDisplay.GetHAngle()-90.0f)*(PI/180)) / 285)*(uint32(gDisplay.GetDiff())/DIFFMOD_CONTROL);
            colision = gDisplay.CheckColision(newx,newy,newz);
            if(!(colision & AXIS_Z))
                gDisplay.SetViewZ(newz,false);
        }

        //Posun ostre doleva (klavesa A)
        //Posun o 90� vice doleva oproti uhlu zobrazeni
        if(keys['A']) //puvodne Q !
        {
            unsigned short colision = 0;
            float newx = gDisplay.GetViewX() + ((GLfloat)sin((gDisplay.GetHAngle()+90.0f)*(PI/180)) / 285)*(uint32(gDisplay.GetDiff())/DIFFMOD_CONTROL);
            float newy = gDisplay.GetViewY();
            float newz = gDisplay.GetViewZ();
            colision = gDisplay.CheckColision(newx,newy,newz);
            if(!(colision & AXIS_X))
                gDisplay.SetViewX(newx,false);

            newx = gDisplay.GetViewX();
            newy = gDisplay.GetViewY();
            newz = gDisplay.GetViewZ() + ((GLfloat)cos((gDisplay.GetHAngle()+90.0f)*(PI/180)) / 285)*(uint32(gDisplay.GetDiff())/DIFFMOD_CONTROL);
            colision = gDisplay.CheckColision(newx,newy,newz);
            if(!(colision & AXIS_Z))
                gDisplay.SetViewZ(newz,false);
        }

        if(keys['R'])
        {
            gDisplay.SetVAngle(-0.5f,true);
        }
        if(keys['F'])
        {
            gDisplay.SetVAngle(0.5f,true);
        }
    }
}

