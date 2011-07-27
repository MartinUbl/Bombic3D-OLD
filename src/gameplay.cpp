#include <game_inc.h>
#include <gameplay.h>

GamePlay gGamePlay;

GamePlay::GamePlay()
{
    //
}

GamePlay::~GamePlay()
{
    //
}

//param1 = displaylist id 1
//param2 = displaylist id 2
void ClearBoom(uint32 param1, uint32 param2, uint32 param3)
{
    gDisplayStore.RemoveDisplayRecord(param1);
    gDisplayStore.RemoveDisplayRecord(param2);
}

//param1 = ContentPosX
//param2 = ContentPosZ
//param3 = displaylist id
void Boom(uint32 param1, uint32 param2, uint32 param3)
{
    float pos_x, pos_z;

    pos_x = float(param1-1)+0.5f;
    pos_z = float(param2-1)+0.5f;

    BillboardDisplayListRecord* pRec = NULL;
    BillboardDisplayListRecord* pRec2 = NULL;
    for (int i = 1; i < 5; i++)
    {
        // axis X
        pRec  = gDisplay.DrawBillboard(pos_x+i,2.0f,pos_z,3,1,3,true);
        pRec2 = gDisplay.DrawBillboard(pos_x-i,2.0f,pos_z,3,1,3,true);
        if (pRec && pRec2)
            gTimer.AddTimedEvent(2000,&ClearBoom,pRec->id,pRec2->id,0);

        // axis Z
        pRec  = gDisplay.DrawBillboard(pos_x,2.0f,pos_z+i,3,1,3,true);
        pRec2 = gDisplay.DrawBillboard(pos_x,2.0f,pos_z-i,3,1,3,true);
        if (pRec && pRec2)
            gTimer.AddTimedEvent(2000,&ClearBoom,pRec->id,pRec2->id,0);
    }

    gDisplayStore.RemoveDisplayRecord(param3);
}

void GamePlay::PlantBomb()
{
    int ContentPosX = int(fabs((gDisplay.GetViewX())/MAP_SCALE_X))+1;
    int ContentPosZ = int(fabs((gDisplay.GetViewZ())/MAP_SCALE_Z))+1;
    float conx = 0.5f+(ContentPosX-1);
    float conz = 0.5f+(ContentPosZ-1);

    ModelDisplayListRecord* temp = gDisplay.DrawModel(conx,0,conz,2,ANIM_IDLE,false,1);
    gTimer.AddTimedEvent(2000, &Boom, (uint32)ContentPosX, (uint32)ContentPosZ, temp->id);
}
