#ifndef __DRAWING_H_
#define __DRAWING_H_

#pragma once

#include <game_inc.h>
#include <time.h>

#define DEFAULT_MIN_VIEW_HEIGHT 0.5f //Minimalni vyska zobrazeni nad terenem
#define DEFAULT_PITCH_SPEED 0.1f //Vychozi rychlost otaceni

#define GOING_UP_SPEED 0.01f // Rychlost vystupu nahoru
#define GOING_DOWN_SPEED 0.007f //Rychlost vyvazovani terenu
#define FALLING_SPEED 0.005f //Rychlost padani

#define MAP_SCALE_X 0.3f  //Roztazeni (smrsteni) mapy po ose X
#define MAP_SCALE_Y 0.05f //Roztazeni (smrsteni) mapy po ose Y
#define MAP_SCALE_Z 0.3f  //Roztazeni (smrsteni) mapy po ose Z

#define MODEL_SCALE 0.01f //Koeficient velikosti 3D modelu

#define MODEL_COLLISION_RADIUS 20.0f
#define COLLISION_RANGE 0.08f //Vzdalenost od objektu pri kolizi

#define PLAYER_DEFAULT_MODELID 1
#define PLAYER_VIEW_DISTANCE 0.60f

#define DIFFMOD_DISPLAY 4
#define DIFFMOD_CONTROL 3

#define PI 3.14159265f

//Reference
struct ModelDisplayListRecord;

//pomocny enum pro pozice na vyskove mape
enum Vertices
{
    V_ZL,
    V_ZP,
    V_PL,
    V_PP
};

//typy pole na mape
enum FieldType
{
    TYPE_GROUND   = 0,
    TYPE_BOX      = 1,
    TYPE_STARTLOC = 2,
};

//kolize s osami
enum AxisColission
{
    AXIS_X = 0x1,
    AXIS_Y = 0x2,
    AXIS_Z = 0x4
};

//Struktura reliefu mapy (vyskova mapa)
struct MapRelief
{
    MapRelief()
    {
        MapId = -1;
        Width = 0;
        Height = 0;
        for (int i = 0; i < 4*2; i++)
            StartLoc[i] = 0;
    }
    int MapId;
    uint32 Width, Height;
    uint32 DefaultTexture;
    uint32 StartLoc[4*2]; //4 hraci, pro kazdeho 2 souradnice
    GLfloat** Content;
    GLfloat** ContentTextures;
};

//Hlavni trida zobrazeni
class Display
{
public:
    Display();
    float GetViewHeight();
    float CalculateFloatZCoef();
    float CalculateFloatXCoef();
    void Initialize();
    void DoTick();
    void SetHAngle(float angle, bool relative = false);
    void SetVAngle(float angle, bool relative = false);
    GLfloat GetHAngle() { return h_angle; }
    GLfloat GetVAngle() { return v_angle; }
    void SetViewPoint(GLfloat new_x, GLfloat new_y, GLfloat new_z)
    {
        view_x = new_x;
        view_y = new_y;
        view_z = new_z;
    }
    void SetViewX(GLfloat new_x, bool relative = false) { view_x = relative?view_x+new_x:new_x; }
    void SetViewY(GLfloat new_y, bool relative = false) { view_y = relative?view_y+new_y:new_y; }
    void SetViewZ(GLfloat new_z, bool relative = false) { view_z = relative?view_z+new_z:new_z; }
    GLfloat GetViewX() { return view_x; }
    GLfloat GetViewY() { return view_y; }
    GLfloat GetViewZ() { return view_z; }

    //Manazer mapy
    bool LoadMap(const char* HeightFile, const char* TextureMap);
    bool UnloadMap();
    GLfloat CalculateVertexPos(int32 i, int32 j, uint8 vertex);

    //Kolize
    unsigned short CheckColision(float newx, float newy, float newz);

    //Organizacni funkce
    void BindMapDefaultTexture();
    void UpdateGroundPosition();
    void UpdatePlayerModelPosition();

    //Objekt hrace
    void SetPlayerAnim(AnimType Animation);

    //Vykresleni textu
    void DrawText(float x, float y, const char* fmt, ...);
    void DrawTexts();
    void FlushTextDisplayList();
    //Vykresleni modelu
    ModelDisplayListRecord* DrawModel(float x, float y, float z, uint32 modelid, AnimType Animation = ANIM_IDLE, bool collision = true, float scale = 1.0f, float rotate = 0.0f);
    void DrawModels();
    void InitModelDisplayList();
    void FlushModelDisplayList();
    void AnimateModelObject(t3DObject *pObject, ModelDisplayListRecord* pData);
    //Vykresleni aktualni mapy a jejich mapobjektu
    void DrawMap();
    //Vykresleni uzivatelskeho rozhrani
    void DrawUI();

    //Diff system
    void SetDiff(clock_t diff) { m_diff = diff; };
    clock_t GetDiff() { return m_diff; };

    //Gameplay
    void PlantBomb();

protected:
    GLfloat h_angle, v_angle;
    GLfloat view_x,view_y,view_z;
    MapRelief actmap; //relief aktualni mapy
    uint32 LastTextureID;

    clock_t m_diff;

    uint32 PlayerModelId;
    uint32 PlayerModelListId;
};

//navratove hodnoty fce GetImageFormat podle pripony
enum TextureFileType
{
    IMG_TYPE_BMP,
    IMG_TYPE_JPG,
    IMG_TYPE_NOT_SUPPORTED
};

//struktura zaznamu display listu textu
struct TextDisplayListRecord
{
    GLfloat x,y;
    std::string what;
};

//struktura zaznamu display listu modelu
struct ModelDisplayListRecord
{
    ModelDisplayListRecord() { remove = false; }
    GLfloat x,y,z;
    uint32 ModelID;
    AnimType Animation;
    uint32 AnimProgress;

    bool collision;
    float scale, rotate;

    bool remove;
};

//Trida uloziste zobrazovacich dat
class DisplayStore
{
public:
    DisplayStore()
    {
        NeededFloorTextures.clear();
        FloorTextures.clear();
        TextDisplayList.clear();
        //ModelDisplayList = (ModelDisplayListRecord*)malloc(sizeof(ModelDisplayListRecord));
        //ModelDisplayListSize = 0;
        ModelDisplayList.clear();
    };
    ~DisplayStore() {};

    TextureFileType GetImageFormat(char *Filename);

    //Nacitani jednotlivych textur ze souboru
    AUX_RGBImageRec* LoadBMP(char *Filename);

    //Nacitani textur do pameti (podle listu potrebnych textur)
    void LoadFloorTextures();
    //Nacitani modelu do pameti (podle listu potrebnych)
    void LoadModels();
    //Nacitani textur modelu
    void LoadModelTextures();

    //Pomocna fce pro zjisteni, zdali se textura uz nachazi v seznamu potrebnych
    bool IsAlreadyNeededTexture(uint32 TextureID);
    //Zjisti ID textury pro dany material
    uint32 GetTextureId(uint32 MaterialID);

    //Pole vsech potrebnych textur na mape
    std::list<uint32> NeededFloorTextures;
    //Pole vsech nactenych textur
    std::map<uint32, GLuint> FloorTextures;

    //Pole vsech jmen souboru s texturami pro modely
    std::map<uint32, std::pair<std::string,bool>> ModelTextureFilenames;
    //Pole prirazeni textury k prislusnemu modelu a jeho materialu
             //ID textury            //Model       //Material
    std::map<uint32, std::pair<uint32, uint32>> TextureMaterialAssign;
    //Pole potrebnych modelu (na mape)
    std::list<uint32> NeededModels;
    //Pole potrebnych textur pro modely
    std::list<uint32> NeededModelTextures;

    //Pole vsech nactenych modelu
    std::map<uint32, t3DModel> Models;
    //Pole vsech nactenych textur modelu
    std::map<uint32, GLuint> ModelTextures;

    //Display list pro text
    std::vector<TextDisplayListRecord*> TextDisplayList;
    //Display list pro modely
    std::vector<ModelDisplayListRecord*> ModelDisplayList;
    //ModelDisplayListRecord* ModelDisplayList;
    //uint32 ModelDisplayListSize;

    t3DSLoader ModelLoader;
};

extern GLuint filter;

extern Display gDisplay;
extern DisplayStore gDisplayStore;

extern int read_JPEG_file (char * filename, uint32 * textureID);

#endif
