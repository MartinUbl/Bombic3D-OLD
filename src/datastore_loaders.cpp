#include <game_inc.h>

//Hlavni nacitaci funkce. Pri nezdaru vrati false a program nebude pokracovat
bool DataStore::LoadEFCDS()
{
    bool success = true;

    success = LoadTextureData();
    success = LoadModelData();

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

    q.get_result("select * from model_animation");

    if(q.num_rows() == 0)
        return false;

    while (q.fetch_row())
    {
        modelid = q.getval();
        AnimType type = (AnimType)q.getval();
        ModelData[modelid].AnimData[type].first = (uint32)q.getval();
        ModelData[modelid].AnimData[type].second = (uint32)q.getval();
    }
    q.free_result();

    return true;
}
