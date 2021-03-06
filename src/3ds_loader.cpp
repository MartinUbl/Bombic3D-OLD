// Original code by DigiBen (digiben@gametutorials.com)
#include <game_inc.h>

// Global
int gBuffer[50000] = {0}; // This is a global buffer used to read past unwanted data

float RoundFloat(float number)
{
    // If the float passed in is a really small number, set it to zero
    if(number > 0 && number <  0.001f) number = 0;
    if(number < 0 && number > -0.001f) number = 0;

    // Return the float changed or unchanged
    return number;
}

//Inicializace pametu pro docasne uloziste
t3DSLoader::t3DSLoader()
{
    m_CurrentChunk = new tChunk; // Initialize and allocate our current chunk
    m_TempChunk = new tChunk;    // Initialize and allocate a temporary chunk
}

//A destruktor - opetovne uvolneni pameti
t3DSLoader::~t3DSLoader()
{
    delete m_CurrentChunk;       // Free the current chunk
    delete m_TempChunk;          // Free our temporary chunk
}

//Nacteni 3DS modelu
bool t3DSLoader::Import3DS(t3DModel *pModel, char *strFileName)
{
    char strMessage[255] = {0};

    // Open the 3DS file
    m_FilePointer = fopen(strFileName, "rb");

    // Make sure we have a valid file pointer (we found the file)
    if(!m_FilePointer)
    {
        sprintf(strMessage, "Unable to find the file: %s!", strFileName);
        MessageBox(NULL, strMessage, "Error", MB_OK);
        return false;
    }

    // Read the first chuck of the file to see if it's a 3DS file
    ReadChunk(m_CurrentChunk);

    // Make sure this is a 3DS file
    if (m_CurrentChunk->ID != PRIMARY)
    {
        sprintf(strMessage, "Unable to load PRIMARY chuck from file: %s!", strFileName);
        MessageBox(NULL, strMessage, "Error", MB_OK);
        return false;
    }

    // Now we actually start reading in the data.  ProcessNextChunk() is recursive

    // Begin loading objects, by calling this recursive function
    ProcessNextChunk(pModel, m_CurrentChunk);

    //Nastaveni prvotniho zvetseni na 1x
    pModel->currentScale = 1.0f;

    //Zmenit velikost...
    ResizeObjects(pModel);

    //Zjistit maximalni a minimalni hodnoty na osach
    GetMaximumMinimumValues(pModel);

    // After we have read the whole 3DS file, we want to calculate our own vertex normals.
    ComputeNormals(pModel);

    // Clean up after everything
    CleanUp();

    return true;
}

//Zvetsit/zmensit podle globalniho nastaveni velikosti modelu
void t3DSLoader::ResizeObjects(t3DModel* pModel)
{
    if(pModel->numOfObjects <= 0)
        return;

    for(int index = 0; index < pModel->numOfObjects; index++)
    {
        for(int i = 0; i <= pModel->numberOfFrames; ++i)
        {
            if (pModel->pObject[index].vScale.size() <= i)
            {
                pModel->pObject[index].vScale.resize(pModel->pObject[index].vScale.size()+1);
                pModel->pObject[index].vScale[i].x = 1;
                pModel->pObject[index].vScale[i].y = 1;
                pModel->pObject[index].vScale[i].z = 1;
            }

            pModel->pObject[index].vScale[i].x *= MODEL_SCALE;
            pModel->pObject[index].vScale[i].y *= MODEL_SCALE;
            pModel->pObject[index].vScale[i].z *= MODEL_SCALE;

            if (pModel->pObject[index].vPosition.size() <= i)
                pModel->pObject[index].vPosition.resize(pModel->pObject[index].vPosition.size()+1);

            pModel->pObject[index].vPosition[i].x *= MODEL_SCALE;
            pModel->pObject[index].vPosition[i].y *= MODEL_SCALE;
            pModel->pObject[index].vPosition[i].z *= MODEL_SCALE;
        }
    }
}

//Pro "krabicovou" kolizi zjisteni a zapsani maximalnich souradnic
void t3DSLoader::GetMaximumMinimumValues(t3DModel *pModel)
{
    if(pModel->numOfObjects <= 0)
        return;

    float temp = 0.0f;

    for(int index = 0; index < pModel->numOfObjects; index++)
    {
        for(int i = 0; i <= pModel->numberOfFrames; ++i)
        {
            for(int j = 0; j < pModel->pObject[index].numOfVerts; ++j)
            {
                temp = pModel->pObject[index].pVerts[j].x*pModel->pObject[index].vScale[i].x + pModel->pObject[index].vPosition[i].x;
                if(temp > pModel->Maximum.x)
                    pModel->Maximum.x = temp;
                if(temp < pModel->Minimum.x)
                    pModel->Minimum.x = temp;

                temp = pModel->pObject[index].pVerts[j].y*pModel->pObject[index].vScale[i].y + pModel->pObject[index].vPosition[i].y;
                if(temp > pModel->Maximum.y)
                    pModel->Maximum.y = temp;
                if(temp < pModel->Minimum.y)
                    pModel->Minimum.y = temp;

                temp = pModel->pObject[index].pVerts[j].z*pModel->pObject[index].vScale[i].z + pModel->pObject[index].vPosition[i].z;
                if(temp > pModel->Maximum.z)
                    pModel->Maximum.z = temp;
                if(temp < pModel->Minimum.z)
                    pModel->Minimum.z = temp;
            }
        }
    }
}

//Cleanup
void t3DSLoader::CleanUp()
{
    fclose(m_FilePointer);       // Close the current file pointer
}

//Rekurzivni funkce ktera nacte prvni data z 3DS souboru a rekurzivne zavola sama sebe pro nacteni dalsich
void t3DSLoader::ProcessNextChunk(t3DModel *pModel, tChunk *pPreviousChunk)
{
    t3DObject newObject;                        // This is used to add to our object list
    tMaterialInfo newTexture = {0};                // This is used to add to our material list
    int version = 0;                            // This will hold the file version

    m_CurrentChunk = new tChunk;                // Allocate a new chunk

    // Continue to read the sub chunks until we have reached the length.
    while (pPreviousChunk->bytesRead < pPreviousChunk->length)
    {
        // Read next Chunk
        ReadChunk(m_CurrentChunk);

        // Check the chunk ID
        switch (m_CurrentChunk->ID)
        {
        case VERSION:                            // This holds the version of the file
            // Read the file version and add the bytes read to our bytesRead variable
            m_CurrentChunk->bytesRead += fread(&version, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);

            // If the file version is over 3, give a warning that there could be a problem
            if (version > 0x03)
                MessageBox(NULL, "This 3DS file is over version 3 so it may load incorrectly", "Warning", MB_OK);
            break;
        case OBJECTINFO:                        // This holds the version of the mesh
            // Read the next chunk
            ReadChunk(m_TempChunk);

            // Get the version of the mesh
            m_TempChunk->bytesRead += fread(&version, 1, m_TempChunk->length - m_TempChunk->bytesRead, m_FilePointer);

            // Increase the bytesRead by the bytes read from the last chunk
            m_CurrentChunk->bytesRead += m_TempChunk->bytesRead;

            // Go to the next chunk, which is the object has a texture, it should be MATERIAL, then OBJECT.
            ProcessNextChunk(pModel, m_CurrentChunk);
            break;
        case MATERIAL:                            // This holds the material information
            // Increase the number of materials
            pModel->numOfMaterials++;

            // Add a empty texture structure to our texture list.
            pModel->pMaterials.push_back(newTexture);

            // Proceed to the material loading function
            ProcessNextMaterialChunk(pModel, m_CurrentChunk);
            break;
        case OBJECT:                            // This holds the name of the object being read
            // Increase the object count
            pModel->numOfObjects++;

            // Add a new tObject node to our list of objects (like a link list)
            pModel->pObject.push_back(newObject);

            // Initialize the object and all it's data members
            memset(&(pModel->pObject[pModel->numOfObjects - 1]), 0, sizeof(t3DObject));

            // Get the name of the object and store it, then add the read bytes to our byte counter.
            m_CurrentChunk->bytesRead += GetString(pModel->pObject[pModel->numOfObjects - 1].strName);

            // Now proceed to read in the rest of the object information
            ProcessNextObjectChunk(pModel, &(pModel->pObject[pModel->numOfObjects - 1]), m_CurrentChunk);
            break;
        case KEYFRAME:
            // This is where we starting to read in all the key frame information.
            // This is read in at the END of the file.  It stores all the animation data.

            // Recurse further to read in all the animation data
            ProcessNextKeyFrameChunk(pModel, m_CurrentChunk);

            // Read past this chunk and add the bytes read to the byte counter
            m_CurrentChunk->bytesRead += fread(gBuffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
            break;
        default:
            // If we didn't care about a chunk, then we get here.  We still need
            // to read past the unknown or ignored chunk and add the bytes read to the byte counter.
            m_CurrentChunk->bytesRead += fread(gBuffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
            break;
        }

        // Add the bytes read from the last chunk to the previous chunk passed in.
        pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
    }

    // Free the current chunk and set it back to the previous chunk (since it started that way)
    delete m_CurrentChunk;
    m_CurrentChunk = pPreviousChunk;
}

//Nacteni dalsiho chunku z daneho objektu (rekurzivni)
void t3DSLoader::ProcessNextObjectChunk(t3DModel *pModel, t3DObject *pObject, tChunk *pPreviousChunk)
{
    // Allocate a new chunk to work with
    m_CurrentChunk = new tChunk;

    // Continue to read these chunks until we read the end of this sub chunk
    while (pPreviousChunk->bytesRead < pPreviousChunk->length)
    {
        // Read the next chunk
        ReadChunk(m_CurrentChunk);

        // Check which chunk we just read
        switch (m_CurrentChunk->ID)
        {
        case OBJECT_MESH:                    // This lets us know that we are reading a new object
            // We found a new object, so let's read in it's info using recursion
            ProcessNextObjectChunk(pModel, pObject, m_CurrentChunk);
            break;
        case OBJECT_VERTICES:                // This is the objects vertices
            ReadVertices(pObject, m_CurrentChunk);
            break;
        case OBJECT_FACES:                    // This is the objects face information
            ReadVertexIndices(pObject, m_CurrentChunk);
            break;
        case OBJECT_MATERIAL:                // This holds the material name that the object has
            // We now will read the name of the material assigned to this object
            ReadObjectMaterial(pModel, pObject, m_CurrentChunk);
            break;
        case OBJECT_UV:                        // This holds the UV texture coordinates for the object
            // This chunk holds all of the UV coordinates for our object.  Let's read them in.
            ReadUVCoordinates(pObject, m_CurrentChunk);
            break;
        default:
            // Read past the ignored or unknown chunks
            m_CurrentChunk->bytesRead += fread(gBuffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
            break;
        }

        // Add the bytes read from the last chunk to the previous chunk passed in.
        pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
    }

    // Free the current chunk and set it back to the previous chunk (since it started that way)
    delete m_CurrentChunk;
    m_CurrentChunk = pPreviousChunk;
}

//Opet rekurzivni funkce pro nacteni dalsiho chunku materialu
void t3DSLoader::ProcessNextMaterialChunk(t3DModel *pModel, tChunk *pPreviousChunk)
{
    // Allocate a new chunk to work with
    m_CurrentChunk = new tChunk;

    // Continue to read these chunks until we read the end of this sub chunk
    while (pPreviousChunk->bytesRead < pPreviousChunk->length)
    {
        // Read the next chunk
        ReadChunk(m_CurrentChunk);

        // Check which chunk we just read in
        switch (pModel, m_CurrentChunk->ID)
        {
        case MATNAME:                            // This chunk holds the name of the material
            // Here we read in the material name
            m_CurrentChunk->bytesRead += fread(pModel->pMaterials[pModel->numOfMaterials - 1].strName, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
            break;
        case MATDIFFUSE:                        // This holds the R G B color of our object
            ReadColorChunk(&(pModel->pMaterials[pModel->numOfMaterials - 1]), m_CurrentChunk);
            break;
        case MATMAP:                            // This is the header for the texture info
            // Proceed to read in the material information
            ProcessNextMaterialChunk(pModel, m_CurrentChunk);
            break;
        case MATMAPFILE:                        // This stores the file name of the material
            // Here we read in the material's file name
            m_CurrentChunk->bytesRead += fread(pModel->pMaterials[pModel->numOfMaterials - 1].strFile, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
            break;
        default:
            // Read past the ignored or unknown chunks
            m_CurrentChunk->bytesRead += fread(gBuffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
            break;
        }

        // Add the bytes read from the last chunk to the previous chunk passed in.
        pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
    }

    // Free the current chunk and set it back to the previous chunk (since it started that way)
    delete m_CurrentChunk;
    m_CurrentChunk = pPreviousChunk;
}

//Rekurzivni funkce pro nacitani keyframe (animace)
void t3DSLoader::ProcessNextKeyFrameChunk(t3DModel *pModel, tChunk *pPreviousChunk)
{
    char strKeyFrameObject[50] = {0};            // This stores the name of the current object being described
    float temp = 0.0f;                            // This is used to swap the pivot point's y and z.

    // Allocate a new chunk to work with
    m_CurrentChunk = new tChunk;

    // Continue to read these chunks until we read the end of this sub chunk
    while (pPreviousChunk->bytesRead < pPreviousChunk->length)
    {
        // Read the next chunk
        ReadChunk(m_CurrentChunk);

        // Check which chunk we just read
        switch (m_CurrentChunk->ID)
        {
        case KEYFRAME_MESH_INFO:                // This tells us there is a new object being described
            // This tells us that we have another objects animation data to be read,
            // so let's use recursion again so we read the next chunk and not read past this.
            ProcessNextKeyFrameChunk(pModel, m_CurrentChunk);
            break;
        case KEYFRAME_OBJECT_NAME:                // This stores the current objects name
            // Get the name of the object that the animation data being read is about.
            m_CurrentChunk->bytesRead += GetString(strKeyFrameObject);
            // Now that we have the object that is being described, set the m_CurrentObject.
            // That way we have a pointer to the object in the model to store the anim data.
            SetCurrentObject(pModel, strKeyFrameObject);

            // Read past 2 flags and heirarchy number (3 shorts - Not used by this loader).
            // This will be used when we do a bone animation tutorial.
            m_CurrentChunk->bytesRead += fread(gBuffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
            break;
        case KEYFRAME_START_AND_END:            // This chunk stores the start and end frame
            // Read in the beginning frame and the end frame.  We just write over the
            // beginning frame because it is assumed that we will always start at the beginning (0)
            m_CurrentChunk->bytesRead += fread(&(pModel->numberOfFrames), 1, 4, m_FilePointer);
            m_CurrentChunk->bytesRead += fread(&(pModel->numberOfFrames), 1, 4, m_FilePointer);
            break;
        case PIVOT:                                // This stores the pivot point of the object
            // Here we read in 3 floats which are the (X, Y, Z) for the objects pivot point.
            // The pivot point is the local axis in which the object rotates around.  This is
            // By default (0, 0, 0), but may be changed manually in 3DS Max.
            m_CurrentChunk->bytesRead +=
            fread(&(m_CurrentObject->vPivot), 1, sizeof(CVector3), m_FilePointer);

            // Because 3DS Studio Max is Z up and OpenGL is Y up, we need to swap
            // the pivot point's y and z value, then negate the z value.
            temp = m_CurrentObject->vPivot.y;
            m_CurrentObject->vPivot.y = m_CurrentObject->vPivot.z;
            m_CurrentObject->vPivot.z = -temp;
            break;
        case POSITION_TRACK_TAG:                // This stores the translation position each frame
            // Now we want to read in the positions for each frame of the animation
            ReadKeyFramePositions(pModel, m_CurrentChunk);
            break;
        case ROTATION_TRACK_TAG:                // This stores the rotation values for each KEY frame
            // Now we want to read in the rotations for each KEY frame of the animation.
            // This doesn't store rotation values for each frame like scale and translation,
            // so we need to interpolate between each frame.
            ReadKeyFrameRotations(pModel, m_CurrentChunk);
            break;
        case SCALE_TRACK_TAG:                    // This stores the scale values for each frame
            // Now we want to read in the scale value for each frame of the animation
            ReadKeyFrameScales(pModel, m_CurrentChunk);
            break;
        default:
            // Read past the ignored or unknown chunks
            m_CurrentChunk->bytesRead += fread(gBuffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
            break;
        }

        // Add the bytes read from the last chunk to the previous chunk passed in.
        pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
    }

    // Free the current chunk and set it back to the previous chunk (since it started that way)
    delete m_CurrentChunk;
    m_CurrentChunk = pPreviousChunk;
}

//Nacte defaultni chunk z 3DS souboru
void t3DSLoader::ReadChunk(tChunk *pChunk)
{
    // This reads the chunk ID which is 2 bytes.
    pChunk->bytesRead = fread(&pChunk->ID, 1, 2, m_FilePointer);

    // Then, we read the length of the chunk which is 4 bytes.
    pChunk->bytesRead += fread(&pChunk->length, 1, 4, m_FilePointer);
}

//Presne jak napovida nazev funkce - ziska retezec
int t3DSLoader::GetString(char *pBuffer)
{
    int index = 0;

    // Read 1 byte of data which is the first letter of the string
    fread(pBuffer, 1, 1, m_FilePointer);

    // Loop until we get NULL
    while (*(pBuffer + index++) != 0) {

        // Read in a character at a time until we hit NULL.
        fread(pBuffer + index, 1, 1, m_FilePointer);
    }

    // Return the string length, which is how many bytes we read in (including the NULL)
    return strlen(pBuffer) + 1;
}

//Precte barvy v RGB formatu
void t3DSLoader::ReadColorChunk(tMaterialInfo *pMaterial, tChunk *pChunk)
{
    // Read the color chunk info
    ReadChunk(m_TempChunk);

    // Read in the R G B color (3 bytes - 0 through 255)
    m_TempChunk->bytesRead += fread(pMaterial->color, 1, m_TempChunk->length - m_TempChunk->bytesRead, m_FilePointer);

    // Add the bytes read to our chunk
    pChunk->bytesRead += m_TempChunk->bytesRead;
}

//Nacte indices vertexu
void t3DSLoader::ReadVertexIndices(t3DObject *pObject, tChunk *pPreviousChunk)
{
    unsigned short index = 0;                    // This is used to read in the current face index

    // Read in the number of faces that are in this object (int)
    pPreviousChunk->bytesRead += fread(&pObject->numOfFaces, 1, 2, m_FilePointer);

    // Alloc enough memory for the faces and initialize the structure
    pObject->pFaces = new tFace [pObject->numOfFaces];
    memset(pObject->pFaces, 0, sizeof(tFace) * pObject->numOfFaces);

    // Go through all of the faces in this object
    for(int i = 0; i < pObject->numOfFaces; i++)
    {
        // Next, we read in the A then B then C index for the face, but ignore the 4th value.
        // The fourth value is a visibility flag for 3D Studio Max, we don't care about this.
        for(int j = 0; j < 4; j++)
        {
            // Read the first vertice index for the current face
            pPreviousChunk->bytesRead += fread(&index, 1, sizeof(index), m_FilePointer);

            if(j < 3)
            {
                // Store the index in our face structure.
                pObject->pFaces[i].vertIndex[j] = index;
            }
        }
    }
}

//Nacte UV koordinaty
void t3DSLoader::ReadUVCoordinates(t3DObject *pObject, tChunk *pPreviousChunk)
{
    // Read in the number of UV coordinates there are (int)
    pPreviousChunk->bytesRead += fread(&pObject->numTexVertex, 1, 2, m_FilePointer);

    // Allocate memory to hold the UV coordinates
    pObject->pTexVerts = new CVector2 [pObject->numTexVertex];

    // Read in the texture coodinates (an array 2 float)
    pPreviousChunk->bytesRead += fread(pObject->pTexVerts, 1, pPreviousChunk->length - pPreviousChunk->bytesRead, m_FilePointer);
}

//Nacte vertexy
void t3DSLoader::ReadVertices(t3DObject *pObject, tChunk *pPreviousChunk)
{
    // Read in the number of vertices (int)
    pPreviousChunk->bytesRead += fread(&(pObject->numOfVerts), 1, 2, m_FilePointer);

    // Allocate the memory for the verts and initialize the structure
    pObject->pVerts = new CVector3 [pObject->numOfVerts];
    memset(pObject->pVerts, 0, sizeof(CVector3) * pObject->numOfVerts);

    // Read in the array of vertices (an array of 3 floats)
    pPreviousChunk->bytesRead += fread(pObject->pVerts, 1, pPreviousChunk->length - pPreviousChunk->bytesRead, m_FilePointer);

    // Go through all of the vertices that we just read and swap the Y and Z values
    for(int i = 0; i < pObject->numOfVerts; i++)
    {
        // Store off the Y value
        float fTempY = pObject->pVerts[i].y;

        // Set the Y value to the Z value
        pObject->pVerts[i].y = pObject->pVerts[i].z;

        // Set the Z value to the Y value,
        // but negative Z because 3D Studio max does the opposite.
        pObject->pVerts[i].z = -fTempY;
    }
}

//Nacte material objektu
void t3DSLoader::ReadObjectMaterial(t3DModel *pModel, t3DObject *pObject, tChunk *pPreviousChunk)
{
    char strMaterial[255] = {0};            // This is used to hold the objects material name

    // Here we read the material name that is assigned to the current object.
    // strMaterial should now have a string of the material name, like "Material #2" etc..
    pPreviousChunk->bytesRead += GetString(strMaterial);

    // Go through all of the textures
    for(int i = 0; i < pModel->numOfMaterials; i++)
    {
        // If the material we just read in matches the current texture name
        if(strcmp(strMaterial, pModel->pMaterials[i].strName) == 0)
        {
            // Set the material ID to the current index 'i' and stop checking
            pObject->materialID = i;

            // Now that we found the material, check if it's a texture map.
            // If the strFile has a string length of 1 and over it's a texture
            if(strlen(pModel->pMaterials[i].strFile) > 0) {

                // Set the object's flag to say it has a texture map to bind.
                pObject->bHasTexture = true;
            }
            break;
        }
        else
        {
            // Set the ID to -1 to show there is no material for this object
            pObject->materialID = -1;
        }
    }

    // Read past the rest of the chunk since we don't care about shared vertices
    // You will notice we subtract the bytes already read in this chunk from the total length.
    pPreviousChunk->bytesRead += fread(gBuffer, 1, pPreviousChunk->length - pPreviousChunk->bytesRead, m_FilePointer);
}

//Pozice keyframe..
void t3DSLoader::ReadKeyFramePositions(t3DModel *pModel, tChunk *pPreviousChunk)
{
    short frameNumber= 0, flags= 0, ignored= 0;
    long  lunknown= 0;
    int i;

    // Read past the ignored data
    pPreviousChunk->bytesRead += fread(&ignored, 1, sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignored, 1, sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignored, 1, sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignored, 1, sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignored, 1, sizeof(short), m_FilePointer);

    // Here we read in the number of position frames this object has.
    // In other words, how many times the object moves to a new location
    pPreviousChunk->bytesRead += fread(&(m_CurrentObject->positionFrames), 1, sizeof(short), m_FilePointer);

    // Read past one more ignored short
    pPreviousChunk->bytesRead += fread(&ignored, 1, sizeof(short), m_FilePointer);

    // Now we need to go through ALL of the frames of animation and set
    // the position of the object for every frame.  Even if we only have one
    // or 50 position changes out of 100 frames, we will set the remaining to
    // the last position it moved too.
    for(i = 0; i <= (pModel->numberOfFrames + 1); i++)
    {
        // Here we add a new CVector3 to our list of positions.  This will
        // store the current position for the current frame of animation 'i'.
        // Using our STL vector, we just pass in a constructor to a CVector3 and
        // it passes back a default CVector3 through the copy constructor.
        // This is the same thing as creating a new CVector3 and passing it to push_back().
        m_CurrentObject->vPosition.push_back(CVector3());

        // If the current frame of animation hasn't gone over the position frames,
        // we want to read in the next position for the current frame.
        if(i < m_CurrentObject->positionFrames)
        {
            // Read in the current frame number (not used ever, we just use i)
            pPreviousChunk->bytesRead += fread(&frameNumber, 1, sizeof(short), m_FilePointer);

            // Next we read past an unknown long
            pPreviousChunk->bytesRead += fread(&lunknown, 1, sizeof(long), m_FilePointer);

            // Here we read in 3 floats that store the (x, y, z) of the position.
            // Remember, CVector3 is 3 floats so it's the same thing as sizeof(float) * 3.
            pPreviousChunk->bytesRead +=
            fread(&(m_CurrentObject->vPosition[i]), 1, sizeof(CVector3), m_FilePointer);

            // Since 3D Studio Max has the Z axis up, We need to convert it to OpenGL's
            // Y axis up.  To do that we swap the y and z, then negate the new z value.
            float temp = m_CurrentObject->vPosition[i].y;
            m_CurrentObject->vPosition[i].y = m_CurrentObject->vPosition[i].z;
            m_CurrentObject->vPosition[i].z = -temp;
        }
        // Otherwise we just set the current frames position to the last position read in
        else
        {
            // Set the current frame's position to the last position read in.
            m_CurrentObject->vPosition[i] = m_CurrentObject->vPosition[m_CurrentObject->positionFrames - 1];
        }
    }

    for(i = 0; i < m_CurrentObject->numOfVerts; i++)
    {
        // Subtract the current frames position and pivtor point from each vertice to make it easier.
        m_CurrentObject->pVerts[i].x -= m_CurrentObject->vPosition[0].x + m_CurrentObject->vPivot.x;
        m_CurrentObject->pVerts[i].y -= m_CurrentObject->vPosition[0].y + m_CurrentObject->vPivot.y;
        m_CurrentObject->pVerts[i].z -= m_CurrentObject->vPosition[0].z + m_CurrentObject->vPivot.z;
    }
}

//Keyframe otoceni
void t3DSLoader::ReadKeyFrameRotations(t3DModel *pModel, tChunk *pPreviousChunk)
{
    short frameNumber = 0, flags = 0, rotkeys = 0, ignored = 0;
    long lunknown = 0;
    float rotationDegree = 0;
    vector<int> vFrameNumber;
    vector<float> vRotDegree;
    vector<CVector3> vRotation;
    int i;

    // Read past the ignored data
    pPreviousChunk->bytesRead += fread(&ignored, 1, sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignored, 1, sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignored, 1,  sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignored, 1,  sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignored, 1,  sizeof(short), m_FilePointer);

    // Read in the number of rotation key frames for the animation.
    // Remember, unlike the scale and translation data, it does not store
    // the rotation degree and axis for every frame, only for every key frame.
    // That is why we need to interpolate below between each key frame.
    pPreviousChunk->bytesRead += fread(&(m_CurrentObject->rotationFrames), 1, sizeof(short), m_FilePointer);

    // Read past an ignored short
    pPreviousChunk->bytesRead += fread(&ignored, 1,  sizeof(short), m_FilePointer);

    // Now we need to go through ALL of the frames of animation and set
    // the rotation of the object for every frame.  We will need to interpolate
    // between key frames if there is more than 1 (there is always at least 1).
    for(i = 0; i < m_CurrentObject->rotationFrames; i++)
    {
        // Here we add a new CVector3 to our list of rotations.  This will
        // store the current rotation axis for the current frame of animation 'i'.
        // Using our STL vector, we just pass in a constructor to a CVector3 and
        // it passes back a default CVector3 through the copy constructor.
        // This is the same thing as creating a new CVector3 and passing it to push_back().
        vRotation.push_back(CVector3());

        // Next we read in the frame number that the rotation takes place
        pPreviousChunk->bytesRead += fread(&frameNumber, 1, sizeof(short), m_FilePointer);

        // We add the frame number to our list of frame numbers for later when interpolating
        vFrameNumber.push_back(frameNumber);

        // Then we read past some unknown data
        pPreviousChunk->bytesRead += fread(&lunknown, 1, sizeof(long), m_FilePointer);

        // Read in the current rotation degree for this key frame.  We will
        // also inteprolate between this degree down below if needed.
        pPreviousChunk->bytesRead += fread(&rotationDegree, 1, sizeof(float), m_FilePointer);

        // Because 3DS Max stores the rotation degree in radians, we need to
        // convert it to degrees because glRotatef() takes degree not radians.

        // Convert the radians to degress (Radians * (180 / PI) = degrees)
        rotationDegree = rotationDegree * (180.0f / 3.14159f);

        // Add the rotation degree to our rotation list to be interpolated if needed
        vRotDegree.push_back(rotationDegree);

        // Here we read in the actual axis that the object will rotate around.
        // This will NOT need to be interpolated because the rotation degree is what matters.
        pPreviousChunk->bytesRead += fread(&(vRotation[i]), 1, sizeof(CVector3), m_FilePointer);

        // Because I was having problems before with really small numbers getting
        // set to scientific notation and not being passed in correctly to glRotatef(),
        // I just decided to round them down to 0 if they were too small.
        // This isn't ideal, but it seemed to work for me.
        vRotation[i].x = RoundFloat(vRotation[i].x);
        vRotation[i].y = RoundFloat(vRotation[i].y);
        vRotation[i].z = RoundFloat(vRotation[i].z);

        // Since 3DS Max is Z up, we need to convert it to Y up.
        // We just switch the Y and Z values.  This is a bit different
        // through because we don't negate the new z value, but negate
        // the new Y value.  Also, below we negate the X value.
        // Because rotations are different than just points this is how it's done.
        float temp = vRotation[i].y;
        vRotation[i].y = -vRotation[i].z;
        vRotation[i].z = temp;

        // We negate the X value to set it right from Z up to Y up axis'
        vRotation[i].x *= -1;
    }

    // Add the rotation axis for the first frame to our list.
    m_CurrentObject->vRotation.push_back(vRotation[0]);

    // Add the rotation degree for the first frame to our list.  If we did NO rotation
    // in our animation the rotation degree should be 0.
    m_CurrentObject->vRotDegree.push_back(vRotDegree[0]);

    // Create a counter for the current rotation key we are on (Only used if rotKeys are > 1)
    int currentKey = 1;

    // Not to be repetitive or anything, but we need to go through all of the
    // frames of animation again so we can interpolate between each rotation key frame.
    // If there is only one rotation key frame, we just set the rest of the rotation degrees to 0.

    // Go through all of the frames of animation plus 1 because it's zero based
    for(i = 1; i <= (pModel->numberOfFrames + 1); i++)
    {
        // Check if the current key frame is less than or equal to the max key frames
        if(currentKey < m_CurrentObject->rotationFrames)
        {
            // Get the current and previous key frame number, along with the rotation degree.
            // This just makes it easier code to work with, especially since you can't
            // debug vectors easily because they are operator overloaded.
            int currentFrame = vFrameNumber[currentKey];
            int previousFrame = vFrameNumber[currentKey - 1];
            float degree = vRotDegree[currentKey];

            // Interpolate the rotation degrees between the current and last key frame.
            // Basically, this sickningly simple algorithm is just getting how many
            // frames are between the last and current keyframe (currentFrame - previousFrame),
            // Then dividing the current degree by that number.  For instance, say there
            // is a key frame at frame 0, and frame 50.  Well, 50 - 0 == 50 so it comes out
            // to rotationDegree / 50.  This will give us the rotation needed for each frame.
            float rotDegree = degree / (currentFrame - previousFrame);

            // Add the current rotation degree and vector for this frame
            m_CurrentObject->vRotation.push_back(vRotation[currentKey]);
            m_CurrentObject->vRotDegree.push_back(rotDegree);

            // Check if we need to go to the next key frame
            // (if the current frame i == the current key's frame number)
            if(vFrameNumber[currentKey] <= i)
                currentKey++;
        }
        // Otherwise, we are done with key frames, so no more interpolating
        else
        {
            // Set the rest of the rotations to 0 since we don't need to rotate it anymore
            // The rotation axis doesn't matter since the degree is 0.
            m_CurrentObject->vRotation.push_back(vRotation[currentKey - 1]);
            m_CurrentObject->vRotDegree.push_back(0.0f);
        }
    }
}

//Keyframe zvetseni/zmenseni
void t3DSLoader::ReadKeyFrameScales(t3DModel *pModel, tChunk *pPreviousChunk)
{
    short frameNumber = 0, ignore = 0, flags = 0;
    long lunknown = 0;
    int i = 0;

    // Read past the ignored data
    pPreviousChunk->bytesRead += fread(&ignore, 1, sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignore, 1, sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignore, 1, sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignore, 1, sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignore, 1, sizeof(short), m_FilePointer);

    // Here we read in the amount of scale frames there are in the animation.
    // If there is 100 frames of animation and only 50 frames of scaling, we just
    // set the rest of the 50 frames to the last scale frame.
    pPreviousChunk->bytesRead += fread(&(m_CurrentObject->scaleFrames), 1, sizeof(short), m_FilePointer);

    // Read past ignored data
    pPreviousChunk->bytesRead += fread(&ignore, 1, sizeof(short), m_FilePointer);

    // Now we need to go through ALL of the frames of animation and set
    // the scale value of the object for every frame.  Even if we only have one
    // or 50 scale changes out of 100 frames, we will set the remaining to
    // the last position it scaled too.
    for(i = 0; i <= (pModel->numberOfFrames + 1); i++)
    {
        // Here we add a new CVector3 to our list of scale values.
        // Using our STL vector, we just pass in a constructor to a CVector3 and
        // it passes back a default CVector3 through the copy constructor.
        // This is the same thing as creating a new CVector3 and passing it to push_back().
        m_CurrentObject->vScale.push_back(CVector3());

        // If the current frame is less than the amount of scale frames, read scale data in
        if(i < m_CurrentObject->scaleFrames)
        {
            // Read in the current frame number (not used because there is no interpolation)
            pPreviousChunk->bytesRead += fread(&frameNumber, 1, sizeof(short), m_FilePointer);

            // Read past an unknown long
            pPreviousChunk->bytesRead += fread(&lunknown, 1, sizeof(long), m_FilePointer);

            // Read in the (X, Y, Z) scale value for the current frame.  We will pass this
            // into glScalef() when animating in AnimateModel()
            pPreviousChunk->bytesRead +=
            fread(&(m_CurrentObject->vScale[i]), 1, sizeof(CVector3), m_FilePointer);

            // Because 3DS Max has Z up, we need to convert it to Y up like OpenGL has.
            // We don't need to negate the z though since it's a scale ratio not a 3D point.
            float temp = m_CurrentObject->vScale[i].y;
            m_CurrentObject->vScale[i].y = m_CurrentObject->vScale[i].z;
            m_CurrentObject->vScale[i].z = temp;
        }
        // Otherwise we are done with the scale frames so set the rest to the last scale value
        else
        {
            // Set the current frame's scale value to the last scale value for the animation
            m_CurrentObject->vScale[i] = m_CurrentObject->vScale[m_CurrentObject->scaleFrames - 1];
        }
    }
}

//Jen organizace
void t3DSLoader::SetCurrentObject(t3DModel *pModel, char *strObjectName)
{
    // Make sure there was a valid object name passed in
    if(!strObjectName)
    {
        // Set the current object to NULL and return
        m_CurrentObject = NULL;
        return;
    }

    // Go through all of the models objects and match up the name passed in
    for(int i = 0; i < pModel->numOfObjects; i++)
    {
        // Check if the current model being looked at has the same name as strObjectName
        if(strcmp(pModel->pObject[i].strName, strObjectName) == 0)
        {
            // Get a pointer to the object with the name passed in.
            // This will be the object that the next animation data is describing
            m_CurrentObject =&(pModel->pObject[i]);
            return;
        }
    }

    // Give an error message (better to have an assert()
    MessageBox(NULL, "ERROR: No object in model with given name! (SetCurrentObject)", "Error!", MB_OK);

    // Set the current object to NULL since we didn't find an object with that name
    m_CurrentObject = NULL;
}

//////////////////////////////    Math Functions  ////////////////////////////////*

// This computes the magnitude of a normal.   (magnitude = sqrt(x^2 + y^2 + z^2)
#define Mag(Normal) (sqrt(Normal.x*Normal.x + Normal.y*Normal.y + Normal.z*Normal.z))

// This calculates a vector between 2 points and returns the result
CVector3 Vector(CVector3 vPoint1, CVector3 vPoint2)
{
    CVector3 vVector;                            // The variable to hold the resultant vector

    vVector.x = vPoint1.x - vPoint2.x;            // Subtract point1 and point2 x's
    vVector.y = vPoint1.y - vPoint2.y;            // Subtract point1 and point2 y's
    vVector.z = vPoint1.z - vPoint2.z;            // Subtract point1 and point2 z's

    return vVector;                                // Return the resultant vector
}

// This adds 2 vectors together and returns the result
CVector3 AddVector(CVector3 vVector1, CVector3 vVector2)
{
    CVector3 vResult;                            // The variable to hold the resultant vector

    vResult.x = vVector2.x + vVector1.x;        // Add Vector1 and Vector2 x's
    vResult.y = vVector2.y + vVector1.y;        // Add Vector1 and Vector2 y's
    vResult.z = vVector2.z + vVector1.z;        // Add Vector1 and Vector2 z's

    return vResult;                                // Return the resultant vector
}

// This divides a vector by a single number (scalar) and returns the result
CVector3 DivideVectorByScaler(CVector3 vVector1, float Scaler)
{
    CVector3 vResult;                            // The variable to hold the resultant vector

    vResult.x = vVector1.x / Scaler;            // Divide Vector1's x value by the scaler
    vResult.y = vVector1.y / Scaler;            // Divide Vector1's y value by the scaler
    vResult.z = vVector1.z / Scaler;            // Divide Vector1's z value by the scaler

    return vResult;                                // Return the resultant vector
}

// This returns the cross product between 2 vectors
CVector3 Cross(CVector3 vVector1, CVector3 vVector2)
{
    CVector3 vCross;                                // The vector to hold the cross product
                                                // Get the X value
    vCross.x = ((vVector1.y * vVector2.z) - (vVector1.z * vVector2.y));
                                                // Get the Y value
    vCross.y = ((vVector1.z * vVector2.x) - (vVector1.x * vVector2.z));
                                                // Get the Z value
    vCross.z = ((vVector1.x * vVector2.y) - (vVector1.y * vVector2.x));

    return vCross;                                // Return the cross product
}

// This returns the normal of a vector
CVector3 Normalize(CVector3 vNormal)
{
    double Magnitude;                            // This holds the magitude

    Magnitude = Mag(vNormal);                    // Get the magnitude

    vNormal.x /= (float)Magnitude;                // Divide the vector's X by the magnitude
    vNormal.y /= (float)Magnitude;                // Divide the vector's Y by the magnitude
    vNormal.z /= (float)Magnitude;                // Divide the vector's Z by the magnitude

    return vNormal;                                // Return the normal
}

//Vypocte normaly pro vertexy
void t3DSLoader::ComputeNormals(t3DModel *pModel)
{
    CVector3 vVector1, vVector2, vNormal, vPoly[3];

    // If there are no objects, we can skip this part
    if(pModel->numOfObjects <= 0)
        return;

    // Go through each of the objects to calculate their normals
    for(int index = 0; index < pModel->numOfObjects; index++)
    {
        // Get the current object
        t3DObject *pObject = &(pModel->pObject[index]);

        // Here we allocate all the memory we need to calculate the normals
        CVector3 *pNormals        = new CVector3 [pObject->numOfFaces];
        CVector3 *pTempNormals    = new CVector3 [pObject->numOfFaces];
        pObject->pNormals        = new CVector3 [pObject->numOfVerts];

        // Go though all of the faces of this object
        for(int i=0; i < pObject->numOfFaces; i++)
        {
            // To cut down LARGE code, we extract the 3 points of this face
            vPoly[0] = pObject->pVerts[pObject->pFaces[i].vertIndex[0]];
            vPoly[1] = pObject->pVerts[pObject->pFaces[i].vertIndex[1]];
            vPoly[2] = pObject->pVerts[pObject->pFaces[i].vertIndex[2]];

            // Now let's calculate the face normals (Get 2 vectors and find the cross product of those 2)

            vVector1 = Vector(vPoly[0], vPoly[2]);        // Get the vector of the polygon (we just need 2 sides for the normal)
            vVector2 = Vector(vPoly[2], vPoly[1]);        // Get a second vector of the polygon

            vNormal  = Cross(vVector1, vVector2);        // Return the cross product of the 2 vectors (normalize vector, but not a unit vector)
            pTempNormals[i] = vNormal;                    // Save the un-normalized normal for the vertex normals
            vNormal  = Normalize(vNormal);                // Normalize the cross product to give us the polygons normal

            pNormals[i] = vNormal;                        // Assign the normal to the list of normals
        }

        //////////////// Now Get The Vertex Normals /////////////////

        CVector3 vSum = {0.0, 0.0, 0.0};
        CVector3 vZero = vSum;
        int shared=0;

        for (int i = 0; i < pObject->numOfVerts; i++)            // Go through all of the vertices
        {
            for (int j = 0; j < pObject->numOfFaces; j++)    // Go through all of the triangles
            {                                                // Check if the vertex is shared by another face
                if (pObject->pFaces[j].vertIndex[0] == i ||
                    pObject->pFaces[j].vertIndex[1] == i ||
                    pObject->pFaces[j].vertIndex[2] == i)
                {
                    vSum = AddVector(vSum, pTempNormals[j]);// Add the un-normalized normal of the shared face
                    shared++;                                // Increase the number of shared triangles
                }
            }

            // Get the normal by dividing the sum by the shared.  We negate the shared so it has the normals pointing out.
            pObject->pNormals[i] = DivideVectorByScaler(vSum, float(-shared));

            // Normalize the normal for the final vertex normal
            pObject->pNormals[i] = Normalize(pObject->pNormals[i]);

            vSum = vZero;                                    // Reset the sum
            shared = 0;                                        // Reset the shared
        }

        // Free our memory and start over on the next object
        delete [] pTempNormals;
        delete [] pNormals;
    }
}
