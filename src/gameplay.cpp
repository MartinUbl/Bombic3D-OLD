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

//param1 = ContentPosX
//param2 = ContentPosZ
//param3 = displaylist id
void Boom(uint32 param1, uint32 param2, uint32 param3)
{
    float pos_x, pos_z;

    pos_x = float(param1-1)+0.5f;
    pos_z = float(param2-1)+0.5f;
    //doprava
    gEmitterMgr.AddEmitter(pos_x,2.0f,pos_z,0,0,0,0,-0.25f,-0.25f,0.25f,0.25f,3,0.3f,0.5f,3,6,1000,10000,50000,0,false,EMIT_FLAG_RANDOM_ROTATE);
    //dozadu
    gEmitterMgr.AddEmitter(pos_x,2.0f,pos_z,PI/2,0,0,0,-0.25f,-0.25f,0.25f,0.25f,3,0.3f,0.5f,3,6,1000,8000,2000,1,false,EMIT_FLAG_RANDOM_ROTATE);
    //doleva
    gEmitterMgr.AddEmitter(pos_x,2.0f,pos_z,PI,0,0,0,-0.25f,-0.25f,0.25f,0.25f,3,0.3f,0.5f,3,6,1000,8000,2000,1,false,EMIT_FLAG_RANDOM_ROTATE);
    //dopredu
    gEmitterMgr.AddEmitter(pos_x,2.0f,pos_z,3*PI/2,0,0,0,-0.25f,-0.25f,0.25f,0.25f,3,0.3f,0.5f,3,6,1000,8000,2000,1,false,EMIT_FLAG_RANDOM_ROTATE);

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
