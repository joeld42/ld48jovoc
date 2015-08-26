#include <stdlib.h>

#include "zillaGame.h"

#include <png.h>

////////////////////////////////////////////
// this is cut and pasted with some modifications from
// the nvidia SDK, shader/nv_png.cpp. I figure this is
// okay because it's pretty much the same as the example
// code on the libPng site.

void Texture::loadPngRGBA(const char * filename )
{
	printf("loadPngRGBA: Loading texture %s\n", filename );
	FILE * fp;
    png_byte sig[8];
    int bit_depth, color_type;
    double              gamma;
    png_uint_32 channels, row_bytes;
    png_byte **row_pointers = 0;
	png_structp png_ptr = 0;
    png_infop info_ptr = 0;

	printf( "in loadPngRGBA %s\n",filename );

    // open the PNG input file
    if (!filename) return;
	
    if (!(fp=fopen(filename, "rb")))
	{
		fprintf(stderr,"Cannot open \"%s\" for read!\n", filename);
		return;
	}
	
    // first check the eight byte PNG signature
    fread(sig, 1, 8, fp);
    if (!png_check_sig(sig, 8)) { 
		fclose(fp); return; 
	}
	
	
	// start back here!!!!
	
    // create the two png(-info) structures
	
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (!png_ptr) { fclose(fp); return; }
	
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, 0, 0);
		fclose(fp);
    }
	
	// initialize the png structure
    png_init_io(png_ptr, fp);	

	png_set_sig_bytes(png_ptr, 8);
	
	// read all PNG info up to image data
	png_read_info(png_ptr, info_ptr);
	
	// get width, height, bit-depth and color-type	
	png_uint_32 ww, hh;
	png_get_IHDR(png_ptr, info_ptr, &w, &h, &bit_depth, &color_type, 0, 0, 0);
	
	// expand images of all color-type and bit-depth to 3x8 bit RGB images
	// let the library process things like alpha, transparency, background
	
	if (bit_depth == 16) png_set_strip_16(png_ptr);
	if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_expand(png_ptr);
	if (bit_depth < 8) png_set_expand(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_expand(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);
	
	// if required set gamma conversion
	if (png_get_gAMA(png_ptr, info_ptr, &gamma)) png_set_gamma(png_ptr, (double) 2.2, gamma);
	
	// after the transformations have been registered update info_ptr data
	png_read_update_info(png_ptr, info_ptr);
	
	// get again width, height and the new bit-depth and color-type
	png_get_IHDR(png_ptr, info_ptr, &w, &h, &bit_depth, &color_type, 0, 0, 0);
	
	// row_bytes is the width x number of channels
	row_bytes = png_get_rowbytes(png_ptr, info_ptr);
	channels = png_get_channels(png_ptr, info_ptr);
		
    if ( channels == 3 )
    {
		fprintf(stderr,"Unable to open image of type RGB \"%s\" using read_png_rgba().\n", filename);
		fprintf(stderr,"Try using read_png_rgb() instead!\n\n");
		return;
    }

    // now we can allocate memory to store the image

    png_byte * img = new png_byte[row_bytes * h];
	
	// and allocate memory for an array of row-pointers

	png_byte ** row = new png_byte * [h];

	
	// set the individual row-pointers to point at the correct offsets
	
	for (unsigned int i = 0; i < h; i++) {
		row[i] = img + i * row_bytes;
	}

	// now we can go ahead and just read the whole image
	
	png_read_image(png_ptr, row);
	
	// read the additional chunks in the PNG file (not really needed)
	
	png_read_end(png_ptr, NULL);
	
	hasAlpha = true;
	data = (unsigned char *)malloc( w * h * 4 );

	printf( "Loading texture width %d height %d\n", w, h );
	
	unsigned long ndx;
	for(unsigned int j=0; j < h; j++) { 
		for(int i=0; i < w; i++) {		
			ndx = (j*w*4)+(i*4);
			data[ndx] = row[j][i*4];
			data[ndx+1] = row[j][i*4+1];
			data[ndx+2] = row[j][i*4+2];
			data[ndx+3] = row[j][i*4+3];		
		}
	}

	delete [] row;
	delete [] img;


    png_destroy_read_struct(&png_ptr, 0, 0);

    fclose (fp);
}

void Texture::loadPngRGB(const char * filename )
{
	printf("loadPngRGB: Loading texture %s\n", filename );
	FILE * fp;
    png_byte sig[8];
    int bit_depth, color_type;
    double              gamma;
    png_uint_32 channels, row_bytes;
    png_byte **row_pointers = 0;
	png_structp png_ptr = 0;
    png_infop info_ptr = 0;    
	
    // open the PNG input file
    if (!filename) return;
	
    if (!(fp = fopen(filename,"rb")))
	{
		fprintf(stderr,"Cannot open \"%s\" for read!\n", filename);
		return;
	}
	
    // first check the eight byte PNG signature
    fread(sig, 1, 8, fp);
    if (!png_check_sig(sig, 8)) { fclose(fp); return; }
	
	
	// start back here!!!!
	
    // create the two png(-info) structures
	
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (!png_ptr) { fclose(fp); return; }
	
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, 0, 0);
		fclose(fp);
    }
	
	// initialize the png structure
    png_init_io(png_ptr, fp);	

	png_set_sig_bytes(png_ptr, 8);
	
	// read all PNG info up to image data
	png_read_info(png_ptr, info_ptr);
	
	// get width, height, bit-depth and color-type		
	png_get_IHDR(png_ptr, info_ptr, &w, &h, &bit_depth, &color_type, 0, 0, 0);
	
	// expand images of all color-type and bit-depth to 3x8 bit RGB images
	// let the library process things like alpha, transparency, background
	
	if (bit_depth == 16) png_set_strip_16(png_ptr);
	if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_expand(png_ptr);
	if (bit_depth < 8) png_set_expand(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_expand(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);
	if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
		png_set_strip_alpha(png_ptr);
	
	// if required set gamma conversion
	if (png_get_gAMA(png_ptr, info_ptr, &gamma)) png_set_gamma(png_ptr, (double) 2.2, gamma);
	
	// after the transformations have been registered update info_ptr data
	png_read_update_info(png_ptr, info_ptr);
	
	// get again width, height and the new bit-depth and color-type
	png_get_IHDR(png_ptr, info_ptr, &w, &h, &bit_depth, &color_type, 0, 0, 0);
	
	
	// row_bytes is the width x number of channels
	row_bytes = png_get_rowbytes(png_ptr, info_ptr);
	channels = png_get_channels(png_ptr, info_ptr);

    if ( channels == 4 )
    {
		fprintf(stderr,"Unable to open image of type RGBA \"%s\" using read_png_rgb().\n", filename);
		fprintf(stderr,"Try using read_png_rgba() instead!\n\n");
		return;
    }
		
	// now we can allocate memory to store the image

    png_byte * img = new png_byte[row_bytes * h];
	
	// and allocate memory for an array of row-pointers

	png_byte ** row = new png_byte * [h];

	
	// set the individual row-pointers to point at the correct offsets
	
	for (unsigned int i = 0; i < h; i++)
		row[i] = img + i * row_bytes;
	
	// now we can go ahead and just read the whole image
	
	png_read_image(png_ptr, row);
	
	// read the additional chunks in the PNG file (not really needed)
	
	png_read_end(png_ptr, NULL);
		
	hasAlpha = false;
	data = (unsigned char *)malloc( w * h * 3);

	int ndx;
	for(unsigned int j=0; j < h; j++) {
		for(unsigned int i=0; i < w; i++) { 
			ndx = (j*w*3)+(i*3);
			data[ndx] = row[j][i*3];
			data[ndx+1] = row[j][i*3+1];
			data[ndx+2] = row[j][i*3+2];
			
			//printf("%d %d -- ndx %d   (%d, %d, %d, %d)\n",
			//		i,j, ndx, data[ndx],data[ndx+1],data[ndx+2],data[ndx+3] );
		}
	}

	delete [] row;
	delete [] img;

    png_destroy_read_struct(&png_ptr, 0, 0);

    fclose (fp);
}
