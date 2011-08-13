#ifndef _DATASTORE_H_
#define _DATASTORE_H_

#include <game_inc.h>

//Data o animaci modelu
//               typ animace        ,prvni frame ,posledni frame
typedef std::map<AnimType, std::pair<uint32,uint32>> AnimData_t;

//Sablona pro ID a jmena souboru textur + deklarovani typu pro snazsi orientaci
struct TextureDataStore_t
{
    std::string filename;
    bool loaded;
};
typedef std::map<uint32, TextureDataStore_t> TextureDataStore;

//Sablona pro ID a jmena souboru modelu + deklarovani typu
struct ModelDataStore_t
{
    std::string filename;
    bool loaded;
    AnimData_t AnimData;
};
typedef std::map<uint32, ModelDataStore_t> ModelDataStore;

//Sablona pro ID a data skyboxu
struct SkyboxDataStore_t
{
    uint32 box_textures[6];
};
typedef std::map<uint32, SkyboxDataStore_t> SkyboxDataStore;

//Hlavni trida pro uloziste
class DataStore
{
public:
    //Deklarace ulozist
    TextureDataStore TextureData;
    ModelDataStore ModelData;
    SkyboxDataStore SkyboxData;

    //Deklarace jednotlivych nacitacich funkci
    bool LoadTextureData();
    bool LoadModelData();
    bool LoadSkyboxData();

    //Deklarace hlavni nacitaci funkce volane z venci
    bool LoadEFCDS();
protected:
    //Mutex a log pro databaze
    Database::Mutex mutex;
    StderrLog log;
};

extern DataStore gDataStore;

#endif

