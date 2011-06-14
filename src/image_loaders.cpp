#include <game_inc.h>
#include <setjmp.h>
#include "jpeglib.h"

struct my_error_mgr
{
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
    my_error_ptr myerr = (my_error_ptr) cinfo->err;
    (*cinfo->err->output_message) (cinfo);
    longjmp(myerr->setjmp_buffer, 1);
}

// Nacist JPEG soubor a nacist ho jako opengl texturu
GLOBAL(int)
read_JPEG_file (char * filename, unsigned int * textureID)
{
    unsigned char * big_buff;
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;

    FILE * infile;
    JSAMPARRAY buffer;
    int row_stride;

    if ((infile = fopen(filename, "rb")) == NULL)
    {
        fprintf(stderr, "Nelze otevrit %s\n", filename);
        return 0;
    }

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        fprintf(stderr, "Chyba formatu\n");
        return 0;
    }
    jpeg_create_decompress(&cinfo);

    jpeg_stdio_src(&cinfo, infile);

    (void) jpeg_read_header(&cinfo, TRUE);
    (void) jpeg_start_decompress(&cinfo);
    row_stride = cinfo.output_width * cinfo.output_components;

    buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
    big_buff = (unsigned char*)malloc(cinfo.output_height * cinfo.output_width * cinfo.output_components);

    while (cinfo.output_scanline < cinfo.output_height)
    {
        JDIMENSION read_now = jpeg_read_scanlines(&cinfo, buffer, 1);
        memcpy(&big_buff[(cinfo.output_scanline - read_now) * cinfo.output_width * cinfo.output_components], buffer[0], row_stride);
    }

    glGenTextures(1, textureID);
    glBindTexture(GL_TEXTURE_2D, textureID[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, cinfo.output_width, cinfo.output_height, GL_RGB, GL_UNSIGNED_BYTE, big_buff);

    free(big_buff);

    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 1;
}

//Nacteni BMP textury
AUX_RGBImageRec* DisplayStore::PreLoadBMP(char *Filename)
{
    FILE *File = fopen(Filename,"r");

    if (File)
    {
        fclose(File);
        return auxDIBImageLoad(Filename);
    }
    return NULL;
}

void DisplayStore::LoadBMP(char* filename, unsigned int *textureID)
{
    AUX_RGBImageRec *TextureImage = PreLoadBMP(filename);
    if(TextureImage)
    {
        glGenTextures(1, textureID);

        glBindTexture(GL_TEXTURE_2D, *textureID);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, TextureImage->sizeX, TextureImage->sizeY, GL_RGB, GL_UNSIGNED_BYTE, TextureImage->data);

        if(TextureImage->data)
            free(TextureImage->data);
        free(TextureImage);
    }
}

//Nacteni JPG textury
void DisplayStore::LoadJPG(char *filename, unsigned int *textureID)
{
    read_JPEG_file(filename,textureID);
}

//Nacteni PNG textury
void DisplayStore::LoadPNG(char *filename, unsigned int *textureID)
{
    int width = 0, height = 0, channels = 0;
    GLubyte *imgArray;
    imgArray = SOIL_load_image(filename, &width, &height, &channels, SOIL_LOAD_AUTO);

    glGenTextures(1, textureID);
    glBindTexture(GL_TEXTURE_2D, *textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    if(channels == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgArray);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imgArray);

    SOIL_free_image_data( imgArray );
}

TextureFileType DisplayStore::GetImageFormat(char *Filename)
{
    vector<string> parsed;
    parsed.clear();
    parsed = explode(Filename,'.');
    if(parsed.size() > 0)
    {
        string extension = parsed[parsed.size()-1].substr(parsed[parsed.size()-1].find_last_of(".")+1);

        for(int i = 0; i < extension.size(); ++i)
            extension[i] = toupper(extension.c_str()[i]);

        if(extension == "JPG" || extension == "JPEG")
            return IMG_TYPE_JPG;
        if(extension == "BMP")
            return IMG_TYPE_BMP;
        if(extension == "PNG")
            return IMG_TYPE_PNG;
    }
    return IMG_TYPE_NOT_SUPPORTED;
}
