//------------------------------------------------------------------------------
// writetga.cpp
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
#include "writetga.h"
//------------------------------------------------------------------------------
tga_writer::tga_writer()
{
  Pfile = NULL;
  width = height = bytes_per_pixel = bytes_per_line = 0;
  image_type = TGA_IMAGE_TYPE_NULL;
}
//------------------------------------------------------------------------------
tga_writer::~tga_writer()
{
  close();
}
//------------------------------------------------------------------------------
bool tga_writer::close(void)
{
  width = height = bytes_per_pixel = bytes_per_line = 0;
  image_type = TGA_IMAGE_TYPE_NULL;

  if (Pfile)
  {
    bool failed = (fclose(Pfile) == EOF);

    Pfile = NULL;

    return (failed);
  }

  return (false);
}
//------------------------------------------------------------------------------
bool tga_writer::open(
  const char *Pfilename,
  int width, int height,
  tga_image_type_t image_type)
{
  assert(Pfilename);

  close();

  Pfile = fopen(Pfilename, "wb");
  if (!Pfile)
    return (true);

  this->width = width;
  this->height = height;
  this->image_type = image_type;

  uchar tga_header[18];
  memset(tga_header, 0, sizeof(tga_header));

  bool backwards_flag = false;

  tga_header[12] = (uchar)(width & 0xFF);
  tga_header[13] = (uchar)((width >> 8) & 0xFF);
  tga_header[14] = (uchar)(height & 0xFF);
  tga_header[15] = (uchar)((height >> 8) & 0xFF);
  tga_header[17] = backwards_flag ? 0x00 : 0x20;

  switch (image_type)
  {
    case TGA_IMAGE_TYPE_BGR:
    {
      tga_header[2] = 2;
      tga_header[16] = 24;
      bytes_per_pixel = 3;
      break;
    }
    case TGA_IMAGE_TYPE_GREY:
    {
      tga_header[2] = 3;
      tga_header[16] = 8;
      bytes_per_pixel = 1;
      break;
    }
    default:
      assert(false);
  }

  bytes_per_line = width * bytes_per_pixel;

  if (fwrite(tga_header, sizeof(tga_header), 1, Pfile) != 1)
    return (true);

  return (false);
}
//------------------------------------------------------------------------------
bool tga_writer::write_line(const void *Pscan_line)
{
  if (!Pfile)
    return (true);

  if (fwrite(Pscan_line, bytes_per_line, 1, Pfile) != 1)
    return (true);

  return (false);
}
//------------------------------------------------------------------------------

