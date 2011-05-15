#ifndef _3DS_H
#define _3DS_H

#include <game_inc.h>
#include <vector>

enum AnimType
{
    ANIM_NONE,
    ANIM_IDLE,
    ANIM_WALK,
    ANIM_RUN,
};

//>------ Primary Chunk, at the beginning of each file
#define PRIMARY       0x4D4D

//>------ Main Chunks
#define OBJECTINFO    0x3D3D                // This gives the version of the mesh and is found right before the material and object information
#define VERSION       0x0002                // This gives the version of the .3ds file

//>------ sub defines of OBJECTINFO
#define MATERIAL      0xAFFF                // This stored the texture info
#define OBJECT          0x4000                // This stores the faces, vertices, etc...

//>------ sub defines of MATERIAL
#define MATNAME       0xA000                // This holds the material name
#define MATDIFFUSE    0xA020                // This holds the color of the object/material
#define MATMAP        0xA200                // This is a header for a new material
#define MATMAPFILE    0xA300                // This holds the file name of the texture

#define OBJECT_MESH   0x4100                // This lets us know that we are reading a new object

//>------ sub defines of OBJECT_MESH
#define OBJECT_VERTICES     0x4110            // The objects vertices
#define OBJECT_FACES        0x4120            // The objects faces
#define OBJECT_MATERIAL        0x4130            // This is found if the object has a material, either texture map or color
#define OBJECT_UV            0x4140            // The UV texture coordinates

#define    KEYFRAME                    0xB000    // This lets us know that we are reading in a keyframe
#define KEYFRAME_MESH_INFO            0xB002
#define KEYFRAME_OBJECT_NAME        0xB010
#define KEYFRAME_START_AND_END        0xB008
#define PIVOT                        0xB013
#define POSITION_TRACK_TAG            0xB020
#define ROTATION_TRACK_TAG            0xB021
#define SCALE_TRACK_TAG                0xB022

// This holds the chunk info
struct tChunk
{
    unsigned short int ID;                    // The chunk's ID
    unsigned int length;                    // The length of the chunk
    unsigned int bytesRead;                    // The amount of bytes read within that chunk
};

// This is our 3D point class.  This will be used to store the vertices of our model.
class CVector3 
{
public:
    float x, y, z;
};

// This is our 2D point class.  This will be used to store the UV coordinates.
class CVector2 
{
public:
    float x, y;
};

// This is our face structure.  This is is used for indexing into the vertex 
// and texture coordinate arrays.  From this information we know which vertices
// from our vertex array go to which face, along with the correct texture coordinates.
struct tFace
{
    int vertIndex[3];            // indicies for the verts that make up this triangle
    int coordIndex[3];            // indicies for the tex coords to texture this face
};

// This holds the information for a material.  It may be a texture map of a color.
// Some of these are not used, but I left them because you will want to eventually
// read in the UV tile ratio and the UV tile offset for some models.
struct tMaterialInfo
{
    char  strName[255];            // The texture name
    char  strFile[255];            // The texture file name (If this is set it's a texture map)
    BYTE  color[3];                // The color of the object (R, G, B)
    int   texureId;                // the texture ID
    float uTile;                // u tiling of texture  (Currently not used)
    float vTile;                // v tiling of texture    (Currently not used)
    float uOffset;                // u offset of texture    (Currently not used)
    float vOffset;                // v offset of texture    (Currently not used)
} ;

// This holds all the information for our model/scene. 
// You should eventually turn into a robust class that 
// has loading/drawing/querying functions like:
// LoadModel(...); DrawObject(...); DrawModel(...); DestroyModel(...);
struct t3DObject 
{
    int  numOfVerts;            // The number of verts in the model
    int  numOfFaces;            // The number of faces in the model
    int  numTexVertex;            // The number of texture coordinates
    int  materialID;            // The texture ID to use, which is the index into our texture array
    bool bHasTexture;            // This is TRUE if there is a texture map for this object
    char strName[255];            // The name of the object

    int  positionFrames;        // The number of key frames for the position
    int  rotationFrames;        // The number of key frames for the rotations
    int  scaleFrames;            // The number of key frames for the scaling

    CVector3 vPivot;            // The object pivot point (The local axis it rotates around)

    vector<CVector3> vPosition;    // The object's current position list
    vector<CVector3> vRotation;    // The object's current rotation list
    vector<CVector3> vScale;    // The object's current scale list

    vector<float> vRotDegree;    // The object's current rotation degree (associated with vRotation)

    CVector3  *pVerts;            // The object's vertices
    CVector3  *pNormals;        // The object's normals
    CVector2  *pTexVerts;        // The texture's UV coordinates
    tFace *pFaces;                // The faces information of the object
};

// This holds our model information.  This should also turn into a robust class.
// We use STL's (Standard Template Library) vector class to ease our link list burdens. :)
struct t3DModel 
{
    t3DModel()
    {
        numOfObjects = 0;
        numOfMaterials = 0;
        numberOfFrames = 1;
        currentFrame = 1;
        pMaterials.clear();
        pObject.clear();
        Maximum.x = 0; Minimum.x = 0;
        Maximum.y = 0; Minimum.y = 0;
        Maximum.z = 0; Minimum.z = 0;
    }
    int numOfObjects;            // The number of objects in the model
    int numOfMaterials;            // The number of materials for the model

    int numberOfFrames;            // The number of frames of animation this model (at least 1)
    int currentFrame;            // The current frame of animation that the model is in

    vector<tMaterialInfo> pMaterials;    // The list of material information (Textures and colors)
    vector<t3DObject> pObject;            // The object list for our model

    CVector3 Maximum; //Uchovava maximalni X,Y,Z souradnice kvuli snazsi detekci kolizi
    CVector3 Minimum; //..minimalni souradnice

    float currentScale; //Aktualni zvetseni modelu
};

// This class handles all of the loading code
class t3DSLoader
{
public:
    t3DSLoader();                                // This inits the data members
    ~t3DSLoader();

    // This is the function that you call to load the 3DS
    bool Import3DS(t3DModel *pModel, char *strFileName);

private:
    // This reads in a string and saves it in the char array passed in
    int GetString(char *);

    // This reads the next chunk
    void ReadChunk(tChunk *);

    // This reads the next large chunk
    void ProcessNextChunk(t3DModel *pModel, tChunk *);

    // This reads the object chunks
    void ProcessNextObjectChunk(t3DModel *pModel, t3DObject *pObject, tChunk *);

    // This reads the material chunks
    void ProcessNextMaterialChunk(t3DModel *pModel, tChunk *);

    // This reads the keyframe chunks (animation data)
    void ProcessNextKeyFrameChunk(t3DModel *pModel, tChunk *);

    // This reads the RGB value for the object's color
    void ReadColorChunk(tMaterialInfo *pMaterial, tChunk *pChunk);

    // This reads the objects vertices
    void ReadVertices(t3DObject *pObject, tChunk *);

    // This reads the objects face information
    void ReadVertexIndices(t3DObject *pObject, tChunk *);

    // This reads the texture coodinates of the object
    void ReadUVCoordinates(t3DObject *pObject, tChunk *);

    // This reads in the material name assigned to the object and sets the materialID
    void ReadObjectMaterial(t3DModel *pModel, t3DObject *pObject, tChunk *pPreviousChunk);

    // This reads in the positions of each frame in the animation
    void ReadKeyFramePositions(t3DModel *pModel, tChunk *pPreviousChunk);

    // This reads in the rotation key frames
    void ReadKeyFrameRotations(t3DModel *pModel, tChunk *pPreviousChunk);

    // This reads in the scale key frames
    void ReadKeyFrameScales(t3DModel *pModel, tChunk *pPreviousChunk);

    // This sets the current model that animation is being read in for by it's name
    void SetCurrentObject(t3DModel *pModel, char *strObjectName);

    //Zmensi/zvetsi modely podle nastavene MODEL_SCALE
    void ResizeObjects(t3DModel *pModel);

    //Zjisti a zapise maximalni a minimalni hodnoty na osach u daneho modelu
    void GetMaximumMinimumValues(t3DModel *pModel);

    // This computes the vertex normals for the object (used for lighting)
    void ComputeNormals(t3DModel *pModel);

    // This frees memory and closes the file
    void CleanUp();

    // The file pointer
    FILE *m_FilePointer;

    // These are used through the loading process to hold the chunk information
    tChunk *m_CurrentChunk;
    tChunk *m_TempChunk;

    // This will hold the object that the animation data is being read in for
    t3DObject *m_CurrentObject;
};


#endif

