#include <game_inc.h>
#include <math.h>

#define MODPOS_X ((GLfloat)sin(h_angle*(PI/180)) * PLAYER_VIEW_DISTANCE)
#define MODPOS_Z ((GLfloat)cos(h_angle*(PI/180)) * PLAYER_VIEW_DISTANCE)

Display gDisplay;           //hlavni deklarovana ridici display class
DisplayStore gDisplayStore; //hlavni uloziste zobrazovacich dat

Display::Display()
{
    //Inicializacni hodnoty (temp)
    h_angle = 0.0f;
    v_angle = 35.0f;

    view_x = -0.5f;
    view_y = -DEFAULT_MIN_VIEW_HEIGHT;
    view_z = -2.0f;

    PlayerModelId = PLAYER_DEFAULT_MODELID;
    PlayerModelListId = 0;
}

//Inicializace display class, volana v main vlakne
void Display::Initialize()
{
    //Docasne natvrdo zadane mapy, pozdeji nacteni podle aktualni pozice
    char target_hmap[256];
    char target_tmap[256];
    sprintf(target_hmap,"%s/hmap1.txt",DATA_PATH);
    sprintf(target_tmap,"%s/tmap1.txt",DATA_PATH);
    if(LoadMap(target_hmap,target_tmap))
    {
        actmap.MapId = 1; //TODO: priradit mapam ID
        gDisplayStore.FillCustomNeededData();
        gDisplayStore.LoadFloorTextures();
        gDisplayStore.LoadModels();
        gDisplayStore.LoadModelTextures();
        FlushModelDisplayList();
        InitModelDisplayList();

        //TEMP, inicializovat az se sitovou hrou
        if (actmap.StartLoc[0] && actmap.StartLoc[1])
        {
            view_x = -(float(actmap.StartLoc[0]-1)*MAP_SCALE_X+0.15f);
            view_z = -(float(actmap.StartLoc[1]-1)*MAP_SCALE_Z+0.15f);
        }
    } else {
        MessageBox(hWnd,"Nepovedlo se naèíst mapu","Error",0);
        exit(0);
    }
    //Pracovni emitter, vymazat po doladeni
    //gEmitterMgr.AddEmitter(-view_x,-view_y,-view_z,0,2,0,1,3,0.3f,0.5f,3,6,10000,10000,1000,10,false,EMIT_FLAG_RANDOM_ROTATE);
    //Pracovni animace
    //TODO: vymyslet hezci zpusob inicializace
    uint32 *text = new uint32[3];
    text[0] = 3;
    text[1] = 2;
    text[2] = 0;
    gTextureAnimationMgr.AddAnimatedTexture(text,3,500);
    //Pracovni billboard
    DrawBillboard(2,4,2,3,1,3,true);
}

void DisplayStore::FillCustomNeededData()
{
    NeededFloorTextures.push_back(3);
}

//Inicializace display listu pro aktualni mapu a umisteni
void Display::InitModelDisplayList()
{
    //Pridani modelu hrace do display listu + ulozeni jeho pozice ve vektoru
    ModelDisplayListRecord* temp = new ModelDisplayListRecord;
    temp->ModelID = PlayerModelId;
    temp->x = ( -view_x                           )/MAP_SCALE_X;
    temp->y = ( -view_y - DEFAULT_MIN_VIEW_HEIGHT )/MAP_SCALE_Y;
    temp->z = ( -view_z                           )/MAP_SCALE_Z;
    temp->collision = false;
    temp->scale = 0.3f;
    temp->rotate = 0.0f;
    temp->Animation = ANIM_IDLE;
    temp->AnimProgress = gDataStore.ModelData[PlayerModelId].AnimData[ANIM_IDLE].first;
    temp->id = gDisplayStore.GetFreeID(TYPE_MODEL);
    gDisplayStore.ModelDisplayList.push_back(temp);
    PlayerModelListId = gDisplayStore.ModelDisplayList.size()-1;
}

//Aktualizace zobrazeni (v promenne diff ulozen cas od posledni aktualizace)
void Display::DoTick()
{
    //vertikalni natoceni kamery
    glRotatef(v_angle,1.0f,0.0f,0.0f);

    //Natoceni a preklad podle aktualnich pozic
    glRotatef(h_angle,0.0f,-1.0f,0.0f);
    glTranslatef(view_x-MODPOS_X,view_y,view_z-MODPOS_Z);

    //Player Model posun a rotace
    UpdatePlayerModelPosition();

    //Bodove svetlo, po dodelani prostredi vymazat!
    GLfloat DiffuseLightParam[] = {0.2f,0.2f,0.2f,1.0f};
    GLfloat DiffuseLightPos[]   = {view_x,view_y,view_z,1.0f};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, DiffuseLightParam);
    glLightfv(GL_LIGHT1, GL_POSITION,DiffuseLightPos);

    //Povoleni mapovani textur
    glEnable(GL_TEXTURE_2D);
    //Nabindovat defaultni texturu
    BindMapDefaultTexture();

    //Vykresleni mapy
    DrawMap();

    //Vykresleni billboardu
    DrawBillboards();

    uint32 pdiff = (uint32)m_diff & 0xFFFF;
    //Vykresleni fontu (pozdeji taky bude nutne nahradit, po zpojizdneni font display listu)
    this->DrawText(-0.052f,-0.037f, "X: %f, Y: %f, Z: %f, [%u,%u]", view_x, view_y, view_z,int(fabs((view_x)/MAP_SCALE_X))+1,int(fabs((view_z)/MAP_SCALE_Z))+1);

    //Vykresleni uzivatelskeho rozhrani
    //gInterface.Draw();

    DrawModels();
    DrawTexts();

    glTranslatef(+MODPOS_X,0,+MODPOS_Z);

    FlushTextDisplayList();
}

//Horizontalni otoceni kamery
void Display::SetHAngle(float angle, bool relative)
{
    h_angle = relative? h_angle+angle : angle;
}

void Display::SetVAngle(float angle, bool relative)
{
    v_angle = relative? v_angle+angle : angle;
}

//Vraci aktualni vysku zapsanou v reliefu mapy
float Display::GetViewHeight()
{
    return actmap.Content[int(fabs(view_x/MAP_SCALE_X))+1][int(fabs(view_z/MAP_SCALE_Z))+1]*MAP_SCALE_Y;
}

//Vraci modifier na ose Z pro vyskovou mapu pri pohybu po nerovnem terenu
float Display::CalculateFloatZCoef()
{
    int ContentPosX = int(fabs((view_x)/MAP_SCALE_X))+1;
    int ContentPosZ = int(fabs((view_z)/MAP_SCALE_Z))+1;

    if(ContentPosX+1 >= actmap.Width)
        ContentPosX -= 1;

    if(ContentPosZ+1 >= actmap.Height)
        ContentPosZ -= 1;

    float TgValue = (fabs( actmap.Content[ContentPosX][ContentPosZ+1] - actmap.Content[ContentPosX][ContentPosZ] )*MAP_SCALE_Y) / (1.0f*MAP_SCALE_Z);

    if( actmap.Content[ContentPosX][ContentPosZ+1] > actmap.Content[ContentPosX][ContentPosZ] )
        return ( TgValue ) * (fabs( (-view_z) - ((ContentPosZ-1)*MAP_SCALE_Z) ));
    else
        return -( TgValue ) * (fabs( (-view_z) - ((ContentPosZ-1)*MAP_SCALE_Z) ));
}

//Vraci modifier na ose X pro vyskovou mapu pri pohybu po nerovnem terenu
float Display::CalculateFloatXCoef()
{
    int ContentPosX = int(fabs((view_x)/MAP_SCALE_X))+1;
    int ContentPosZ = int(fabs((view_z)/MAP_SCALE_Z))+1;

    if(ContentPosX+1 >= actmap.Width)
        ContentPosX -= 1;

    if(ContentPosZ+1 >= actmap.Height)
        ContentPosZ -= 1;

    float TgValue = (fabs( actmap.Content[ContentPosX+1][ContentPosZ] - actmap.Content[ContentPosX][ContentPosZ] )*MAP_SCALE_Y) / (1.0f*MAP_SCALE_X);

    if( actmap.Content[ContentPosX+1][ContentPosZ] > actmap.Content[ContentPosX][ContentPosZ] )
        return ( TgValue ) * (fabs( (-view_x) - ((ContentPosX-1)*MAP_SCALE_X) ));
    else
        return -( TgValue ) * (fabs( (-view_x) - ((ContentPosX-1)*MAP_SCALE_X) ));
}

void Display::UpdatePlayerModelPosition()
{
    ModelDisplayListRecord* mdl = gDisplayStore.ModelDisplayList[PlayerModelListId];

    mdl->x = ( -view_x                           )/MAP_SCALE_X;
    mdl->y = ( -view_y - DEFAULT_MIN_VIEW_HEIGHT )/MAP_SCALE_Y;
    mdl->z = ( -view_z                           )/MAP_SCALE_Z;
    mdl->rotate = h_angle-180.0f;
}

void Display::SetPlayerAnim(AnimType Animation)
{
    gDisplayStore.ModelDisplayList[PlayerModelListId]->Animation = Animation;
}

uint32 DisplayStore::GetFreeID(DisplayListTypes type)
{
    if (type == TYPE_MODEL)
    {
        // Vytvori si list IDcek, ktere uz jsou prirazeny
        std::list<uint32> UsedIDs;
        UsedIDs.clear();
        for (std::vector<ModelDisplayListRecord*>::const_iterator itr = ModelDisplayList.begin(); itr != ModelDisplayList.end(); ++itr)
            UsedIDs.push_back((*itr)->id);

        // Pote projde vsechna cisla od 1 do 0xFFFFFF, a pokud se dane ID nenachazi v listu ke zobrazeni,
        // vrati ho jako vysledek
        bool cont = false;
        for (int i = 1; i < 0x00FFFFFF; i++)
        {
            for (std::list<uint32>::const_iterator itr = UsedIDs.begin(); itr != UsedIDs.end(); ++itr)
                if ((*itr) == MAKEID(i,type))
                {
                    cont = true;
                    break;
                }
            if (cont)
            {
                cont = false;
                continue;
            }
            return MAKEID(i,type);
        }
    }
    else if (type == TYPE_BILLBOARD)
    {
        // Vytvori si list IDcek, ktere uz jsou prirazeny
        std::list<uint32> UsedIDs;
        UsedIDs.clear();
        for (std::vector<BillboardDisplayListRecord*>::const_iterator itr = BillboardDisplayList.begin(); itr != BillboardDisplayList.end(); ++itr)
            UsedIDs.push_back((*itr)->id);

        // Pote projde vsechna cisla od 1 do 0xFFFFFF, a pokud se dane ID nenachazi v listu ke zobrazeni,
        // vrati ho jako vysledek
        for (int i = 1; i < 0x00FFFFFF; i++)
        {
            for (std::list<uint32>::const_iterator itr = UsedIDs.begin(); itr != UsedIDs.end(); ++itr)
                if ((*itr) == MAKEID(i,type))
                    continue;
            return MAKEID(i,type);
        }
    }
    return 0;
}

//Odebrani displaylist recordu podle id
//typ je rozpoznan z prvniho bytu id
void DisplayStore::RemoveDisplayRecord(uint32 id)
{
    if (IDTYPE(id) == TYPE_MODEL)
    {
        for (std::vector<ModelDisplayListRecord*>::const_iterator itr = ModelDisplayList.begin(); itr != ModelDisplayList.end(); ++itr)
            if ((*itr)->id == id)
            {
                ModelDisplayList.erase(itr);
                break;
            }
    }
    else if (IDTYPE(id) == TYPE_BILLBOARD)
    {
        for (std::vector<BillboardDisplayListRecord*>::const_iterator itr = BillboardDisplayList.begin(); itr != BillboardDisplayList.end(); ++itr)
            if ((*itr)->id == id)
            {
                BillboardDisplayList.erase(itr);
                break;
            }
    }
}

//Vykresli model modelid na souradnice x,y,z
ModelDisplayListRecord* Display::DrawModel(float x, float y, float z, uint32 modelid, AnimType Animation, bool collision, float scale, float rotate)
{
    ModelDisplayListRecord* temp = new ModelDisplayListRecord;
    temp->ModelID = modelid;
    temp->x = x;
    temp->y = y;
    temp->z = z;
    temp->scale = scale;
    temp->rotate = rotate;
    temp->collision = collision;
    temp->Animation = Animation;
    temp->AnimProgress = gDataStore.ModelData[modelid].AnimData[Animation].first;
    temp->id = gDisplayStore.GetFreeID(TYPE_MODEL);
    gDisplayStore.ModelDisplayList.push_back(temp);

    return temp;
}

void Display::DrawModels()
{
    float x,y,z;
    ModelDisplayListRecord* temp = NULL;
    uint32 AnimFirst, AnimLast;

    for(std::vector<ModelDisplayListRecord*>::iterator itr = gDisplayStore.ModelDisplayList.begin(); itr != gDisplayStore.ModelDisplayList.end(); ++itr)
    {
        temp = *itr;

        if (temp->remove)
        {
            itr = gDisplayStore.ModelDisplayList.erase(itr);
            if (itr == gDisplayStore.ModelDisplayList.end())
                break;
        }

        x = temp->x*MAP_SCALE_X;
        y = temp->y*MAP_SCALE_Y;
        z = temp->z*MAP_SCALE_Z;

        glLoadIdentity();

        glRotatef(v_angle,1.0f, 0.0f,0.0f);
        glRotatef(h_angle,0.0f,-1.0f,0.0f);

        glTranslatef(view_x-MODPOS_X,view_y,view_z-MODPOS_Z);
        glTranslatef(x,y,z);

        glRotatef(temp->rotate,0.0f,1.0f,0.0f);

        t3DModel* pModel = &gDisplayStore.Models[temp->ModelID];

        //Posunout frame animace modelu pri kazdem pokusu o vykresleni
        if(temp->Animation != ANIM_NONE)
        {
            AnimFirst = gDataStore.ModelData[temp->ModelID].AnimData[temp->Animation].first;
            AnimLast = gDataStore.ModelData[temp->ModelID].AnimData[temp->Animation].second;
            temp->AnimProgress += 1+(uint32(m_diff)/2);
            if(temp->AnimProgress > pModel->numberOfFrames || temp->AnimProgress > AnimLast)
                temp->AnimProgress = AnimFirst;
        }

        if(temp->AnimProgress == 0)
            temp->AnimProgress = 1;

        for(int i = 0; i < pModel->numOfObjects; i++)
        {
            if(pModel->pObject.size() <= 0) break;
            t3DObject *pObject = &pModel->pObject[i];

            glPushMatrix();
            AnimateModelObject(pObject, temp);

            if(pObject->bHasTexture)
            {
                glEnable(GL_TEXTURE_2D);
                glColor3ub(255, 255, 255);
                glBindTexture(GL_TEXTURE_2D, gDisplayStore.ModelTextures[gDisplayStore.GetTextureId(pObject->materialID)]);
            }
            else
            {
                glDisable(GL_TEXTURE_2D);
                if(pModel->pMaterials.size() && pObject->materialID >= 0)
                {
                    BYTE *pColor = pModel->pMaterials[pObject->materialID].color;
                    glColor3ub(pColor[0], pColor[1], pColor[2]);
                }
            }

            glBegin(GL_TRIANGLES);

            for(int j = 0; j < pObject->numOfFaces; j++)
            {
                for(int whichVertex = 0; whichVertex < 3; whichVertex++)
                {
                    int index = pObject->pFaces[j].vertIndex[whichVertex];

                    if(pObject->pNormals)
                    {
                        CVector3* pNormal = &pObject->pNormals[index];
                        glNormal3f(pNormal->x, pNormal->y, pNormal->z);
                    }

                    if(pObject->bHasTexture && pObject->pTexVerts != NULL)
                    {
                        CVector2* pTexVert = &pObject->pTexVerts[index];
                        glTexCoord2f(pTexVert->x, pTexVert->y);
                    }

                    if(pObject->pVerts)
                    {
                        CVector3* pVert = &pObject->pVerts[index];
                        glVertex3f(pVert->x, pVert->y, pVert->z);
                    }
                }
            }

            glEnd();

            glPopMatrix();
        }
    }

    glColor3ub(255, 255, 255);
}

void Display::DrawText(float x, float y, const char* fmt, ...)
{
    TextDisplayListRecord* temp = new TextDisplayListRecord;
    char tmp[512];

    temp->what = "";
    va_list argList;
    va_start(argList, fmt);
    vsprintf(tmp,fmt,argList);
    va_end(argList);
    temp->what = tmp;
    temp->x = x;
    temp->y = y;
    gDisplayStore.TextDisplayList.push_back(temp);
}

void Display::DrawTexts()
{
    if(gDisplayStore.TextDisplayList.empty())
        return;

    glLoadIdentity();
    glTranslatef(0.0f,0.0f,-0.1f);

    for(int i = 0; i < gDisplayStore.TextDisplayList.size(); ++i)
    {
        if(gDisplayStore.TextDisplayList[i]->what.c_str())
        {
            glRasterPos2f(gDisplayStore.TextDisplayList[i]->x,gDisplayStore.TextDisplayList[i]->y);
            glPrint(gDisplayStore.TextDisplayList[i]->what.c_str());
        }
    }

    glLoadIdentity();
}

void Display::FlushTextDisplayList()
{
    gDisplayStore.TextDisplayList.clear();
}

void Display::FlushModelDisplayList()
{
    gDisplayStore.ModelDisplayList.clear();
}

void Display::BindMapDefaultTexture()
{
    //Nabindovani defaultni textury
    LastTextureID = actmap.DefaultTexture;
    glBindTexture(GL_TEXTURE_2D, gDisplayStore.FloorTextures[actmap.DefaultTexture]);
}

void Display::DrawMap()
{
    //Vykresleni mapy
    for(int i=0;i<(uint32)actmap.Width;++i)
    {
        for(int j=0;j<(uint32)actmap.Height;++j)
        {
            //Pokud se zmenila textura
            if(actmap.ContentTextures[i][j] != LastTextureID)
            {
                //A pokud je textura nactena v pameti
                if(gDataStore.TextureData[(uint32)actmap.ContentTextures[i][j]].loaded = true)
                {
                    //Nabindovat
                    LastTextureID = (uint32)actmap.ContentTextures[i][j];
                    glBindTexture(GL_TEXTURE_2D, gDisplayStore.FloorTextures[(uint32)actmap.ContentTextures[i][j]]);
                }
            }
            //Prevedeni pro lepsi orientaci
            GLfloat px = (GLfloat)i;
            GLfloat py = (GLfloat)j;
            if (actmap.Content[i][j] == TYPE_GROUND || actmap.Content[i][j] == TYPE_STARTLOC)
            {
                glBegin(GL_POLYGON);
                    glNormal3f(0.0f,-1.0f, 0.0f);
                    glTexCoord2f(1.0f, 1.0f); glVertex3f((px-1)*MAP_SCALE_X, 0*MAP_SCALE_Y, (py-1)*MAP_SCALE_Z);
                    glTexCoord2f(0.0f, 1.0f); glVertex3f((px  )*MAP_SCALE_X, 0*MAP_SCALE_Y, (py-1)*MAP_SCALE_Z);
                    glTexCoord2f(0.0f, 0.0f); glVertex3f((px  )*MAP_SCALE_X, 0*MAP_SCALE_Y, (py  )*MAP_SCALE_Z);
                    glTexCoord2f(1.0f, 0.0f); glVertex3f((px-1)*MAP_SCALE_X, 0*MAP_SCALE_Y, (py  )*MAP_SCALE_Z);
                glEnd();
            }
            else if (actmap.Content[i][j] == TYPE_BOX)
            {
                glBegin(GL_QUADS);
                    glNormal3f(0.0f,-1.0f, 0.0f);
                    // horni
                    glTexCoord2f(1.0f, 1.0f); glVertex3f((px-1)*MAP_SCALE_X, 4*MAP_SCALE_Y, (py-1)*MAP_SCALE_Z);
                    glTexCoord2f(0.0f, 1.0f); glVertex3f((px  )*MAP_SCALE_X, 4*MAP_SCALE_Y, (py-1)*MAP_SCALE_Z);
                    glTexCoord2f(0.0f, 0.0f); glVertex3f((px  )*MAP_SCALE_X, 4*MAP_SCALE_Y, (py  )*MAP_SCALE_Z);
                    glTexCoord2f(1.0f, 0.0f); glVertex3f((px-1)*MAP_SCALE_X, 4*MAP_SCALE_Y, (py  )*MAP_SCALE_Z);
                    // levobok
                    glTexCoord2f(1.0f, 1.0f); glVertex3f((px-1)*MAP_SCALE_X, 4*MAP_SCALE_Y, (py-1)*MAP_SCALE_Z);
                    glTexCoord2f(1.0f, 0.0f); glVertex3f((px-1)*MAP_SCALE_X, 0*MAP_SCALE_Y, (py-1)*MAP_SCALE_Z);
                    glTexCoord2f(0.0f, 0.0f); glVertex3f((px-1)*MAP_SCALE_X, 0*MAP_SCALE_Y, (py)*MAP_SCALE_Z);
                    glTexCoord2f(0.0f, 1.0f); glVertex3f((px-1)*MAP_SCALE_X, 4*MAP_SCALE_Y, (py)*MAP_SCALE_Z);
                    // pravobok
                    glTexCoord2f(1.0f, 1.0f); glVertex3f((px)*MAP_SCALE_X, 4*MAP_SCALE_Y, (py)*MAP_SCALE_Z);
                    glTexCoord2f(1.0f, 0.0f); glVertex3f((px)*MAP_SCALE_X, 0*MAP_SCALE_Y, (py)*MAP_SCALE_Z);
                    glTexCoord2f(0.0f, 0.0f); glVertex3f((px)*MAP_SCALE_X, 0*MAP_SCALE_Y, (py-1)*MAP_SCALE_Z);
                    glTexCoord2f(0.0f, 1.0f); glVertex3f((px)*MAP_SCALE_X, 4*MAP_SCALE_Y, (py-1)*MAP_SCALE_Z);
                    // predni
                    glTexCoord2f(1.0f, 1.0f); glVertex3f((px-1)*MAP_SCALE_X, 4*MAP_SCALE_Y, (py)*MAP_SCALE_Z);
                    glTexCoord2f(1.0f, 0.0f); glVertex3f((px-1)*MAP_SCALE_X, 0*MAP_SCALE_Y, (py)*MAP_SCALE_Z);
                    glTexCoord2f(0.0f, 0.0f); glVertex3f((px)*MAP_SCALE_X, 0*MAP_SCALE_Y, (py)*MAP_SCALE_Z);
                    glTexCoord2f(0.0f, 1.0f); glVertex3f((px)*MAP_SCALE_X, 4*MAP_SCALE_Y, (py)*MAP_SCALE_Z);
                    // zadni
                    glTexCoord2f(1.0f, 1.0f); glVertex3f((px-1)*MAP_SCALE_X, 4*MAP_SCALE_Y, (py-1)*MAP_SCALE_Z);
                    glTexCoord2f(1.0f, 0.0f); glVertex3f((px-1)*MAP_SCALE_X, 0*MAP_SCALE_Y, (py-1)*MAP_SCALE_Z);
                    glTexCoord2f(0.0f, 0.0f); glVertex3f((px)*MAP_SCALE_X, 0*MAP_SCALE_Y, (py-1)*MAP_SCALE_Z);
                    glTexCoord2f(0.0f, 1.0f); glVertex3f((px)*MAP_SCALE_X, 4*MAP_SCALE_Y, (py-1)*MAP_SCALE_Z);
                glEnd();
            }
        }
    }
}

//Vykresleni vsech billboardu
void Display::DrawBillboards()
{
    BillboardDisplayListRecord* temp = NULL;

    for (std::vector<BillboardDisplayListRecord*>::iterator itr = gDisplayStore.BillboardDisplayList.begin(); itr != gDisplayStore.BillboardDisplayList.end(); ++itr)
    {
        temp = (*itr);

        if (temp->remove)
        {
            itr = gDisplayStore.BillboardDisplayList.erase(itr);
            if (itr == gDisplayStore.BillboardDisplayList.end())
                break;
        }

        glLoadIdentity();

        glRotatef(v_angle,1.0f,0.0f,0.0f);
        glRotatef(h_angle,0.0f,-1.0f,0.0f);
        glTranslatef(view_x-MODPOS_X,view_y,view_z-MODPOS_Z);

        glBindTexture(GL_TEXTURE_2D, gDisplayStore.FloorTextures[temp->TextureID]);

        // Pruhledne objekty potrebuji mit zapnuty mod pro blending a mod one minus src alpha pro
        // spravne vykresleni pruhlednosti
        if (temp->blend)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST);
            //TODO: blending zvlast kanalu
            //glColor4f(1,1,1,0.85);
        }
        glTranslatef(temp->x*MAP_SCALE_X,temp->y*MAP_SCALE_Y,temp->z*MAP_SCALE_Z);
        glRotatef(90.0f+h_angle,0.0f,1.0f,0.0f);

        glBegin(GL_QUADS);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(0*MAP_SCALE_X, temp->height*MAP_SCALE_Y, (-temp->width/2)*MAP_SCALE_Z);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(0*MAP_SCALE_X, 0*MAP_SCALE_Y, (-temp->width/2)*MAP_SCALE_Z);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(0*MAP_SCALE_X, 0*MAP_SCALE_Y, ( temp->width/2)*MAP_SCALE_Z);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(0*MAP_SCALE_X, temp->height*MAP_SCALE_Y, ( temp->width/2)*MAP_SCALE_Z);
        glEnd();
        // Nezapomeneme vypnout blending, jen jako slusnacci
        if (temp->blend)
        {
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
        }
    }
    glLoadIdentity();

    glRotatef(v_angle,1.0f,0.0f,0.0f);
    glRotatef(h_angle,0.0f,-1.0f,0.0f);
    glTranslatef(view_x-MODPOS_X,view_y,view_z-MODPOS_Z);
}

//Pridani billboardu do listu k vykresleni
BillboardDisplayListRecord* Display::DrawBillboard(float x, float y, float z, uint32 TextureID, float width, float height, bool blend)
{
    BillboardDisplayListRecord* pTemp = new BillboardDisplayListRecord;

    pTemp->x = x;
    pTemp->y = y;
    pTemp->z = z;
    pTemp->TextureID = TextureID;
    pTemp->width = width;
    pTemp->height = height;
    pTemp->id = gDisplayStore.GetFreeID(TYPE_BILLBOARD);
    pTemp->blend = blend;

    if (gTextureAnimationMgr.IsAnimatedTexture(TextureID))
        gTextureAnimationMgr.AddAnimatedBillboard(pTemp);

    gDisplayStore.BillboardDisplayList.push_back(pTemp);
    return pTemp;
}

//Zaridi animaci objektu daneho modelu tak, aby se nastavil do polohy podle aktualniho framu
void Display::AnimateModelObject(t3DObject *pObject, ModelDisplayListRecord* pData)
{
    CVector3 vPosition = pObject->vPosition[pData->AnimProgress];
    glTranslatef(vPosition.x*pData->scale, vPosition.y*pData->scale, vPosition.z*pData->scale);
    CVector3 vScale = pObject->vScale[pData->AnimProgress];
    glScalef(vScale.x*pData->scale, vScale.y*pData->scale, vScale.z*pData->scale);

    for(int i = 1; i <= pData->AnimProgress; i++)
    {
        CVector3 vRotation = pObject->vRotation[i];
        float rotDegree = pObject->vRotDegree[i];

        if(rotDegree)
        {
            glRotatef(rotDegree, vRotation.x, vRotation.y, vRotation.z);
        }
    }
}

bool DisplayStore::IsAlreadyNeededTexture(uint32 TextureID)
{
    //Pokud neni treba zadna textura, nebo jen nebyl naplnen list
    if(NeededFloorTextures.empty())
        return false;

    for(std::list<unsigned int>::const_iterator itr = NeededFloorTextures.begin(); itr != NeededFloorTextures.end(); ++itr)
        if((*itr) == TextureID)
            return true;

    return false;
}

//Vraci texturu pro dany material objektu na modelu (ulozene hodnoty z nacitani modelu)
unsigned int DisplayStore::GetTextureId(uint32 MaterialID)
{
    if(TextureMaterialAssign.empty())
        return 0;

    for(std::map<uint32, std::pair<uint32, uint32>>::const_iterator itr = TextureMaterialAssign.begin();
        itr != TextureMaterialAssign.end(); ++itr)
    {
        if( (*itr).second.second == MaterialID )
            return (*itr).first;
    }

    return 0;
}

unsigned short Display::CheckColision(float newx, float newy, float newz)
{
    uint16 colision = 0;

    int ContentPosX = int(fabs((newx)/MAP_SCALE_X))+1;
    int ContentPosZ = int(fabs((newz)/MAP_SCALE_Z))+1;
    int ContentPosXO = int(fabs((view_x)/MAP_SCALE_X))+1;
    int ContentPosZO = int(fabs((view_z)/MAP_SCALE_Z))+1;

    if(ContentPosX+1 >= actmap.Width)
        colision |= AXIS_X;
    if(ContentPosZ+1 >= actmap.Height)
        colision |= AXIS_Z;

    if(ContentPosX <= 0)
        colision |= AXIS_X;
    if(ContentPosZ <= 0)
        colision |= AXIS_Z;

    if(newx > 0)
        colision |= AXIS_X;
    if(newz > 0)
        colision |= AXIS_Z;

    int lh_x = ContentPosX-1;
    int lh_z = ContentPosZ-1;
    int pd_x = ContentPosX+1;
    int pd_z = ContentPosZ+1;
    if (lh_x < 0)
        lh_x = 0;
    if (lh_z < 0)
        lh_z = 0;
    if (pd_x > actmap.Width-1)
        pd_x = actmap.Width-1;
    if (pd_z > actmap.Height-1)
        pd_z = actmap.Height-1;

    float ncx = fabs(newx);
    float ncz = fabs(newz);
    float ocx = fabs(view_x);
    float ocz = fabs(view_z);
    float conx = 0;
    float conz = 0;
    float sizex = 1*MAP_SCALE_X;
    float sizez = 1*MAP_SCALE_Z;

    for (int i = lh_x; i <= pd_x; i++)
    {
        for (int j = lh_z; j <= pd_z; j++)
        {
            if (actmap.Content[i][j] == TYPE_BOX)
            {
                conx = (i-1)*MAP_SCALE_X;
                conz = (j-1)*MAP_SCALE_Z;

                if (ncx < conx+sizex+COLLISION_RANGE && ncx > conx-COLLISION_RANGE &&
                    ncz < conz+sizez+COLLISION_RANGE && ncz > conz-COLLISION_RANGE)
                {
                    if (ncz != ocz)
                        colision |= AXIS_Z;
                    if (ncx != ocx)
                        colision |= AXIS_X;
                }
            }
        }
    }

    return colision;
}

//Gameplay

void Display::PlantBomb()
{
    int ContentPosX = int(fabs((view_x)/MAP_SCALE_X))+1;
    int ContentPosZ = int(fabs((view_z)/MAP_SCALE_Z))+1;
    float conx = 0.5f+(ContentPosX-1);
    float conz = 0.5f+(ContentPosZ-1);

    DrawModel(conx,0,conz,2,ANIM_IDLE,false,1);
}

