/*
 *  PNGLoader.h
 *
 *  Created by Alexander Okafor on 10/11/09.
 *
 */
#ifndef PNGLOADER_H
#define PNGLOADER_H

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

//#include <OpenGLES/ES1/gl.h>
//#include <GL/gl.h>
#include "GLee.h"

#include <png.h>

#if defined(__cplusplus)
extern "C" {
#endif
	
	typedef struct  {
		int width;
		int widthPow2;
		int height;
		int heightPow2;
		GLuint textureId;
		GLboolean loadedSuccessfully;
        unsigned char *pixelData;
	} PNGImage;
	
extern PNGImage LoadImagePNG(const char *path, bool loadGL, bool keepData );
	


#if defined(__cplusplus)
}
#endif

int pow2(int n);
void PrintPixels(const char *imgName, png_byte *image_buffer, int width, int height, int w2, int h2);
void png_read_premultiply_alpha(png_structp png_ptr, png_row_infop row_info, png_bytep data);
#endif
