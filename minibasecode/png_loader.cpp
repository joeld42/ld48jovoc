/*
 *  PNGLoader.c
 *
 *  Created by Alexander Okafor on 10/11/09.
 *  Modified and brought into Luddite 12/14/10 JBD
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <png.h>

#include "png_loader.h"

#define PNG_HEADER_SIZE 8

PNGImage LoadImagePNG(const char *path)
{
	PNGImage loadedImage;
	loadedImage.loadedSuccessfully = GL_FALSE;
	
	FILE *PNG_file = fopen(path, "rb");
    if (PNG_file == NULL)
    {
        printf("Can't open PNG file %s\n", path);
        return loadedImage;
    }
    
    GLubyte PNG_header[PNG_HEADER_SIZE];
    
    fread(PNG_header, 1, PNG_HEADER_SIZE, PNG_file);
    if (png_sig_cmp(PNG_header, 0, PNG_HEADER_SIZE) != 0)
    {
        printf("%s is not a PNG file\n", path);
        return loadedImage;
    }
    
    png_structp PNG_reader
	= png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (PNG_reader == NULL)
    {
        printf("Can't start reading PNG file %s\n", path);
        fclose(PNG_file);
        return loadedImage;
    }
	
    png_infop PNG_info = png_create_info_struct(PNG_reader);
    if (PNG_info == NULL)
    {
        printf("Can't get info for PNG file %s\n", path);
		
        png_destroy_read_struct(&PNG_reader, NULL, NULL);
        fclose(PNG_file);
        return loadedImage;
    }
	
    png_infop PNG_end_info = png_create_info_struct(PNG_reader);
    if (PNG_end_info == NULL)
    {
        printf("Can't get end info for PNG file %s\n", path);
		
        png_destroy_read_struct(&PNG_reader, &PNG_info, NULL);
        fclose(PNG_file);
        return loadedImage;
    }
    
    if (setjmp(png_jmpbuf(PNG_reader)))
    {
        printf("Can't load PNG file %s\n", path);
		
        png_destroy_read_struct(&PNG_reader, &PNG_info, &PNG_end_info);
        fclose(PNG_file);
        return loadedImage;
    }
    
    png_init_io(PNG_reader, PNG_file);
    png_set_sig_bytes(PNG_reader, PNG_HEADER_SIZE);
    
    png_read_info(PNG_reader, PNG_info);
	
	// we have to do a custom transformation to premultiply the alpha of the image
    png_set_read_user_transform_fn(PNG_reader, png_read_premultiply_alpha);
    
	png_uint_32 width, height;
    int bit_depth, color_type;
    
	png_get_IHDR(PNG_reader, PNG_info, &width, &height, &bit_depth, &color_type,NULL, NULL, NULL);
	
    if (color_type == PNG_COLOR_TYPE_PALETTE)
    {
        png_set_palette_to_rgb(PNG_reader);
    }
	
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) 
    {
        png_set_expand_gray_1_2_4_to_8(PNG_reader);
    }
    
    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        png_set_gray_to_rgb(PNG_reader);
    }
	
    if (png_get_valid(PNG_reader, PNG_info, PNG_INFO_tRNS))
    {
        png_set_tRNS_to_alpha(PNG_reader);
    }
    else
    {
        png_set_filler(PNG_reader, 0xff, PNG_FILLER_AFTER);
    }
    

    if (bit_depth == 16)
    {
        png_set_strip_16(PNG_reader);
    }
	
    png_read_update_info(PNG_reader, PNG_info);
    
	png_uint_32 widthPow2, heightPow2;
	widthPow2 = pow2(width);
	heightPow2 = pow2(height);
	
    png_byte* PNG_image_buffer = (png_byte*)malloc(4 * widthPow2 * heightPow2);
	memset(PNG_image_buffer,0,4*widthPow2*heightPow2); // clear image buffer
    png_byte** PNG_rows = (png_byte**)malloc(height * sizeof(png_byte*));


	png_uint_32 rowBytes = widthPow2*4;

	// load the image from the bottom up
	/*
	 image texture in mem looks like:
	 --------
	 |       |
	 |xxxx	 |
	 |xxxx	 |
	 --------
	 where 'x's represent actual image data and the lines are the image buffer.
	 so the image is aligned at the (0,0) texel coordinate of the image buffer.
	 */
	unsigned int row;
    for (row = 0; row < height; ++row)
    {
        PNG_rows[height-1-row] = PNG_image_buffer + (row * rowBytes);
    }
    png_read_image(PNG_reader, PNG_rows);
	
    free(PNG_rows);
   
    png_destroy_read_struct(&PNG_reader, &PNG_info, &PNG_end_info);
    fclose(PNG_file);
    
	
	GLuint textureID = 0;
	glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	glTexImage2D(
				 GL_TEXTURE_2D,
				 0,
				 GL_RGBA,
				 widthPow2,
				 heightPow2,
				 0,
				 GL_RGBA,
				 GL_UNSIGNED_BYTE,
				 PNG_image_buffer);
    
     free(PNG_image_buffer);
	
	loadedImage.width = width;
	loadedImage.height = height;
	loadedImage.widthPow2 = widthPow2;
	loadedImage.heightPow2 = heightPow2;
	loadedImage.textureId = textureID;
	loadedImage.loadedSuccessfully = GL_TRUE;
	
    return loadedImage;
}

void PrintPixels(const char *imgName, png_byte *image_buffer, int width, int height, int w2, int h2)
{
	int rowWidth = w2 * 4;
	int hWidth = (width/2)*4;
	printf("image: %s\n",imgName);
	
	//for(int i=0; i < 5; ++i)
	{
		png_byte *pixel = image_buffer + (h2-1)*(rowWidth)-hWidth;
		
		int r,g,b,a;
		r = (int)pixel[0];
		g = (int)pixel[1];
		b = (int)pixel[2];
		a = (int)pixel[3];
	//	if(a < 255 && a > 0)
			printf("pixels : [%d %d %d %d]\n",r,g,b,a);
	}
	printf("\n");
}

// pre-multiply the color channels by the alpha
void png_read_premultiply_alpha(png_structp png_ptr, png_row_infop row_info, png_bytep data)
{
	png_bytep row_pointer = data;
	if (png_ptr == NULL)return;
	
	/* Contents of row_info:
	 *  png_uint_32 width      width of row
	 *  png_uint_32 rowbytes   number of bytes in row
	 *  png_byte color_type    color type of pixels
	 *  png_byte bit_depth     bit depth of samples
	 *  png_byte channels      number of channels (1-4)
	 *  png_byte pixel_depth   bits per pixel (depth*channels)
	 */
	if (row_info->bit_depth == 8) // only supporting 8-bit depth png's right now.
	{	
		png_uint_32 rowBytes;
		rowBytes = row_info->rowbytes;
		png_byte *pixel = row_pointer;
		png_byte *stop = row_pointer + rowBytes;
		//int pos =0;
		while (pixel != stop)
		{
			//png_byte *pixel = row_pointer + (pos*row_info->channels);
			float alphaMultiply = pixel[3] / 255.0;
			pixel[0] = (png_byte)(pixel[0] * alphaMultiply);
			pixel[1] = (png_byte)(pixel[1] * alphaMultiply);
			pixel[2] = (png_byte)(pixel[2] * alphaMultiply);
			
			pixel += row_info->channels;
		}
	}
}


int pow2(int n)
{
	int x = 1;
	while( x < n)
	{
		x <<= 1;
	}
	return x;
}

