#include <game_inc.h>
#include <math.h>
#include <gameplay.h>

//Pri stisknuti klavesy
void Interface::KeyPress(uint8 key)
{
    if(!keys[key])         //Jen pro kontrolu
    {
        //Nastavi promennou v arrayi klaves na true, aby bylo mozne
        //pomoci fci KeyRelease a Update dale prebirat klavesy
        keys[key] = true;

        //Specificke udalosti po stisknuti klavesy
        switch(key)
        {
            case 'W':
            case 'A':
            case 'D':
                gDisplay.SetPlayerAnim(ANIM_WALK);
                break;
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

        //Specificke udalosti po pusteni klavesy
        switch(key)
        {
            case 'W':
            case 'A':
            case 'D':
                if(!keys['W'] && !keys['A'] && !keys['D'])
                    gDisplay.SetPlayerAnim(ANIM_IDLE);
                break;
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

        gGamePlay.PlantBomb();

        /* Projdou se vsechny UI prvky a postupne se vyzkousi, zdali si kliknuti
         * zpracuji. Pokud ano, cyklus se prerusi, protoze staci osefovat kliknuti
         * jednim handlerem.
         *
         * UI prvek nemusi mit nutne handler pro kliknutí, pokud se ClickHandler == NULL
         */
        for(int i = 0; i < numrecords; i++)
            if(pUIRecords[i] >= 0 && pUIRecords[i]->ClickHandler != NULL)
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
    //Posun o 90° vice doprava oproti uhlu zobrazeni
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
    //Posun o 90° vice doleva oproti uhlu zobrazeni
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

