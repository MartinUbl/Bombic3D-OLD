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
        fprintf(stderr, "can't open %s\n", filename);
        return 0;
    }

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        fprintf(stderr, "jpeg error\n");
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

    /*
    * We ask opengl to generate a texture and set it's ID in textureID[0]
    */
    glGenTextures(1, textureID);
    /*
    * We tell opengl which texture we are going to use now
    */
    glBindTexture(GL_TEXTURE_2D, textureID[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
    /*
    * We call gluBuild2DMipmaps to load the texture in opengl's memory and create mipmaps
    */
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, cinfo.output_width, cinfo.output_height, GL_RGB, GL_UNSIGNED_BYTE, big_buff);
    /*
    * We no longer need the buffer since opengl has a copy now
    */
    free(big_buff);

    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 1;
}

