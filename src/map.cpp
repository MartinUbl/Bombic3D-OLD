#include <game_inc.h>

#define DATA_SEPARATOR 95 //znak _
#define CHAR_MOVER 48 //posun v ASCII øadì (protože 0 je 48 => 48-48 do heightmap)

bool Display::UnloadMap()
{
    //Uz byla uvolnena
    if(actmap.MapId < 0)
        return false;

    for (int i = 0; i < actmap.Width; i++)
    {
        free((void*)*(actmap.Content + i));
        free((void*)*(actmap.ContentTextures + i));
    }
    free((void*)actmap.Content);
    free((void*)actmap.ContentTextures);

    //Uvolneni textur z pameti
    gDisplayStore.FloorTextures.clear();
    gDisplayStore.NeededFloorTextures.clear();

    for(ModelDataStore::iterator itr = gDataStore.ModelData.begin(); itr != gDataStore.ModelData.end(); ++itr)
        (*itr).second.loaded = false;

    actmap.MapId = -1;

    return true;
}

//Nacte mapu ze souboru
bool Display::LoadMap(const char* HeightFile, const char* TextureMap)
{
    FILE* hmap = fopen(HeightFile,"r");

    if(!hmap)
        return false;

    gDisplayStore.NeededFloorTextures.clear();
    gDisplayStore.NeededModels.clear();
    gDisplayStore.NeededModelTextures.clear();

    //Nez bude implementovano pozicovani modelu podle model mapy
    //bude nutne nacist vsechny modely do pameti, aby byly k dispozici
    //stale
    for(ModelDataStore::const_iterator itr = gDataStore.ModelData.begin(); itr != gDataStore.ModelData.end(); ++itr)
        gDisplayStore.NeededModels.push_back((*itr).first);
    //Pote se odstrani tyto radky, a do listu potrebnych modelu se budou tlacit jen
    //modely, co jsou potrebne

    int c,sloupec,radka,sloupcu,radek,buffer;

    sloupec = 0;
    radka = 0;
    sloupcu = 0;
    radek = 0;
    buffer = 0;
    int DefTexture = 0;
    char HeaderData[64];

    ////
    //Zjisteni poctu radku a sloupcu
    if(!fgets(HeaderData, 64, hmap))
        sscanf(HeaderData,"%i",&DefTexture);

    do
    {
        c = fgetc(hmap);
        if(c == '\n')
            ++radek;
        if(radek < 1)
            ++sloupcu;
    } while (c != EOF);
    ////

    fclose(hmap);
    hmap = NULL;
    hmap = fopen(HeightFile,"r");

    if(!fgets(HeaderData, 64, hmap))
        sscanf(HeaderData,"%i",&DefTexture);

    gDisplayStore.NeededFloorTextures.push_back(DefTexture);
    actmap.DefaultTexture = DefTexture;

    actmap.Content = (GLfloat**)malloc((sloupcu+1)*sizeof(GLfloat*));
    actmap.ContentTextures = (GLfloat**)malloc((sloupcu+1)*sizeof(GLfloat*));
    for(int i = 0; i < (sloupcu+1); i++)
    {
        actmap.Content[i] = (GLfloat*)malloc((radek+1)*sizeof(GLfloat));
        actmap.ContentTextures[i] = (GLfloat*)malloc((radek+1)*sizeof(GLfloat));
    }

    uint32 startlocpos = 0;

    //Nacteni dat do pole v actmap
    do
    {
        c = fgetc(hmap);
        if (c != '\n')
        {
            c = c - CHAR_MOVER;
            if (c >= 0 && c <= 9)
            {
                actmap.Content[sloupec][radka] = (GLfloat)c;
                if (c==2)
                {
                    actmap.StartLoc[startlocpos] = sloupec;
                    actmap.StartLoc[startlocpos+1] = radka;
                    startlocpos += 2;
                }
            }
            sloupec++;
        }
        if (c == '\n' || sloupec > sloupcu)
        {
            sloupec = 0;
            radka++;
        }
        if (radka > radek)
            break;
    } while (c != EOF);

    actmap.Width = sloupcu;
    actmap.Height = radka;

    fclose(hmap);
    FILE* tmap = fopen(TextureMap,"r");

    radka = 0;
    sloupec = 0;

    if(!tmap)
        return false;

    //Nacteni texturove mapy do pole v actmap
    do
    {
        c = fgetc(tmap);
        if (c != '\n')
        {
            c = c - CHAR_MOVER;
            if (c >= 0 && c <= 9)
            {
                actmap.ContentTextures[sloupec][radka] = (GLfloat)c;
                if (c != DefTexture && !gDisplayStore.IsAlreadyNeededTexture(c))
                    gDisplayStore.NeededFloorTextures.push_back(c);
            }
            sloupec++;
        }
        if (c == '\n' || sloupec > sloupcu)
        {
            sloupec = 0;
            radka++;
        }
        if (radka > radek)
            break;
    } while (c != EOF);

    return true;
}

//Vypocet Y pozice podle vyskove mapy zadane v parametru, univerzalni
GLfloat Display::CalculateVertexPos(int32 i, int32 j, uint8 vertex)
{
    /*switch(vertex)
    {
    case V_ZL:
        return actmap.Content[i][j];
    case V_ZP:
        if(i+1 >= actmap.Width)
            return actmap.Content[i][j];
        return actmap.Content[i+1][j];
    case V_PL:
        if(j+1 >= actmap.Height)
            return actmap.Content[i][j];
        return actmap.Content[i][j+1];
    case V_PP:
        if(i+1 >= actmap.Width)
            if(j+1 < actmap.Height)
                return actmap.Content[i][j+1];
            else
                return actmap.Content[i][j];
        if(j+1 >= actmap.Height)
            if(i+1 < actmap.Width)
                return actmap.Content[i+1][j];
            else
                return actmap.Content[i][j];
        return actmap.Content[i+1][j+1];
    }*/

    //return actmap.Content[i][j];
    return 0;
}

