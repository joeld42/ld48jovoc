//------------------------------------------------------------------------------
// writetga.h
// Simple TGA writer -- handles 24-bit truecolor, 8-bit greyscale
// Last updated: Nov. 16, 2000 
// Copyright (C) 1994-2000 Rich Geldreich
// richgel@voicenet.com
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//------------------------------------------------------------------------------
// writetga.h
//------------------------------------------------------------------------------
#ifndef WRITETGA_H
#define WRITETGA_H
//------------------------------------------------------------------------------
#include "main.h"
//------------------------------------------------------------------------------
typedef enum
{
  TGA_IMAGE_TYPE_NULL = 0,
  TGA_IMAGE_TYPE_BGR,
  TGA_IMAGE_TYPE_GREY,
} tga_image_type_t;
//------------------------------------------------------------------------------
class tga_writer
{
  FILE *Pfile;
  int width, height;
  int bytes_per_pixel, bytes_per_line;
  tga_image_type_t image_type;

public:

  tga_writer();

  ~tga_writer();

  bool open(const char *Pfilename,
            int width, int height,
            tga_image_type_t image_type);

  bool close(void);

  bool write_line(const void *Pscan_line);
};
//------------------------------------------------------------------------------
typedef tga_writer *Ptga_writer;
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------

