#include <game_inc.h>

//Hlavni nacitaci funkce. Pri nezdaru vrati false a program nebude pokracovat
bool DataStore::LoadEFCDS()
{
    bool success = true;

    success = LoadTextureData() && LoadModelData() && LoadSkyboxData();

    return success;
}

//Nacteni dat pro textury
bool DataStore::LoadTextureData()
{
    char efcds_filename[256];
    sprintf(efcds_filename,"%s/efcds/texture.efcds",DATA_PATH);
    Database db(mutex,efcds_filename,&log);
    Query q(db);

    //Nacteni ID a jmen textur
    TextureDataStore_t TempData;
    uint32 id = 0;
    TempData.filename = "";

    q.get_result("select * from texture_filename");

    if(q.num_rows() == 0)
        return false;

    while (q.fetch_row())
    {
        id = q.getval();
        TempData.filename = DATA_PATH;
        TempData.filename.append("/");
        TempData.filename.append(q.getstr());
        TempData.loaded = false;
        TextureData[id] = TempData;
    }
    q.free_result();

    return true;
}

//Nacteni dat pro modely
bool DataStore::LoadModelData()
{
    char efcds_filename[256];
    sprintf(efcds_filename,"%s/efcds/model.efcds",DATA_PATH);
    Database db(mutex,efcds_filename,&log);
    Query q(db);

    //Nacteni ID a jmen textur
    ModelDataStore_t TempData;
    uint32 id = 0;
    TempData.filename = "";

    q.get_result("select * from model_filename");

    if(q.num_rows() == 0)
        return false;

    while (q.fetch_row())
    {
        id = q.getval();
        TempData.filename = DATA_PATH;
        TempData.filename.append("/");
        TempData.filename.append(q.getstr());
        ModelData[id] = TempData;
    }
    q.free_result();

    //Nacteni dat animaci
    uint32 modelid;
    AnimData_t_s tmpanim;
    AnimType type;

    q.get_result("select * from model_animation");

    if(q.num_rows() == 0)
        return false;

    while (q.fetch_row())
    {
        tmpanim.interval = q.getval();
        modelid = q.getval();
        type = (AnimType)q.getval();
        tmpanim.first = (uint32)q.getval();
        tmpanim.last = (uint32)q.getval();
        ModelData[modelid].AnimData[type] = tmpanim;
    }
    q.free_result();

    return true;
}

//Nacteni dat pro skyboxy
bool DataStore::LoadSkyboxData()
{
    char efcds_filename[256];
    sprintf(efcds_filename,"%s/efcds/skybox.efcds",DATA_PATH);
    Database db(mutex,efcds_filename,&log);
    Query q(db);

    //Nacteni ID a jmen textur
    SkyboxDataStore_t TempData;
    uint32 id = 0;
    memset(&TempData,0,sizeof(SkyboxDataStore_t));

    q.get_result("select * from skybox_data");

    if(q.num_rows() == 0)
        return false;

    while (q.fetch_row())
    {
        id = q.getval();
        for (int i = 0; i < 6; i++)
            TempData.box_textures[i] = q.getuval();
        SkyboxData[id] = TempData;
    }
    q.free_result();

    return true;
}
