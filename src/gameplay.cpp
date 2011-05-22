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

void Boom(uint32 param1, uint32 param2)
{
    // BUM !
}

void GamePlay::PlantBomb()
{
	int ContentPosX = int(fabs((gDisplay.GetViewX())/MAP_SCALE_X))+1;
    int ContentPosZ = int(fabs((gDisplay.GetViewZ())/MAP_SCALE_Z))+1;
    float conx = 0.5f+(ContentPosX-1);
    float conz = 0.5f+(ContentPosZ-1);

    gDisplay.DrawModel(conx,0,conz,2,ANIM_IDLE,false,1);
    gTimer.AddTimedEvent(2000, &Boom, (uint32)ContentPosX, (uint32)ContentPosZ);
}
