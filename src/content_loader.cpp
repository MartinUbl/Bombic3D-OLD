#include <game_inc.h>

//Nacteni textur pro podlahu
void DisplayStore::LoadFloorTextures()
{
    if(NeededFloorTextures.empty())
        return;

    for(std::list<uint32>::const_iterator itr = NeededFloorTextures.begin(); itr != NeededFloorTextures.end(); ++itr)
    {
        uint32 TextureID = *itr;
        const char *ImageFilename = gDataStore.TextureData[TextureID].filename.c_str();

        switch(GetImageFormat((char*)gDataStore.TextureData[TextureID].filename.c_str()))
        {
            case IMG_TYPE_BMP:
            {
                LoadBMP((char*)ImageFilename,&FloorTextures[TextureID]);
                gDataStore.TextureData[TextureID].loaded = true;
                break;
            }
            case IMG_TYPE_JPG:
            {
                LoadJPG((char*)ImageFilename,&FloorTextures[TextureID]);
                gDataStore.TextureData[TextureID].loaded = true;
                break;
            }
            case IMG_TYPE_PNG:
            {
                LoadPNG((char*)ImageFilename,&FloorTextures[TextureID]);
                gDataStore.TextureData[TextureID].loaded = true;
                break;
            }
            default:
                break;
        }
    }
}

//Nacteni textur pro modely
void DisplayStore::LoadModelTextures()
{
    if(NeededModelTextures.empty())
        return;

    for(std::list<uint32>::const_iterator itr = NeededModelTextures.begin(); itr != NeededModelTextures.end(); ++itr)
    {
        uint32 TextureID = *itr;
        const char *ImageFilename = ModelTextureFilenames[TextureID].first.c_str();

        switch(GetImageFormat((char*)ImageFilename))
        {
            case IMG_TYPE_BMP:
            {
                LoadBMP((char*)ImageFilename,&ModelTextures[TextureID]);
                ModelTextureFilenames[TextureID].second = true;
                Models[TextureMaterialAssign[TextureID].first].pMaterials[TextureMaterialAssign[TextureID].second].texureId = ModelTextures[TextureID];
                break;
            }
            case IMG_TYPE_JPG:
            {
                LoadJPG((char*)ImageFilename,&ModelTextures[TextureID]);
                ModelTextureFilenames[TextureID].second = true;
                Models[TextureMaterialAssign[TextureID].first].pMaterials[TextureMaterialAssign[TextureID].second].texureId = ModelTextures[TextureID];
                break;
            }
            case IMG_TYPE_PNG:
            {
                LoadPNG((char*)ImageFilename,&ModelTextures[TextureID]);
                ModelTextureFilenames[TextureID].second = true;
                Models[TextureMaterialAssign[TextureID].first].pMaterials[TextureMaterialAssign[TextureID].second].texureId = ModelTextures[TextureID];
                break;
            }
            default:
                break;
        }
    }
}

//Nacteni 3D modelu do pameti (podle listu potrebnych)
void DisplayStore::LoadModels()
{
    if(NeededModels.empty())
        return;

    TextureMaterialAssign.clear();
    ModelTextureFilenames.clear();

    unsigned int TextureID = 0;

    for(std::list<uint32>::const_iterator itr = NeededModels.begin(); itr != NeededModels.end(); ++itr)
    {
        uint32 ModelID = (*itr);
        const char* ModelFilename = gDataStore.ModelData[ModelID].filename.c_str();

        if(!ModelFilename || ModelFilename == "")
        {
            //TODO: vyhodit error
            break;
        }
        if(!fopen(ModelFilename,"r"))
        {
            //TODO: vyhodit error
            break;
        }
        Models[ModelID].pMaterials.clear();
        Models[ModelID].pObject.clear();
        memset(&Models[ModelID],0,sizeof(t3DModel));
        if(!(ModelLoader.Import3DS(&Models[ModelID], (char*)ModelFilename)))
        {
            //TODO: vyhodit error
            break;
        }
        for(int i = 0; i < Models[ModelID].numOfMaterials; i++)
        {
            if(strlen(Models[ModelID].pMaterials[i].strFile) > 0)
            {
                char TmpFilename[64];
                sprintf(TmpFilename,"%s/%s",DATA_PATH,Models[ModelID].pMaterials[i].strFile);

                ModelTextureFilenames[TextureID].first = TmpFilename; //Jmeno souboru s texturou
                ModelTextureFilenames[TextureID].second = false;      //Textura neni nactena
                TextureMaterialAssign[TextureID].first = ModelID;     //K jakemu modelu textura patri
                TextureMaterialAssign[TextureID].second = i;          //A k jakemu materialu patri

                NeededModelTextures.push_back(TextureID); //A pridat nasi texturu do listu potrebnych textur

                TextureID++;
            }
            Models[ModelID].pMaterials[i].texureId = TextureID;
        }
    }
}

