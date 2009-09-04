//------------------------------------------------------------------------------
// jpg2tga.cpp
// JPEG to TGA file conversion example program.
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
#include "jpegdecoder.h"
#include "writetga.h"
//------------------------------------------------------------------------------
// Uncomment to disable output file writing (for VTune benchmarking)
//#define NO_OUTPUT
//------------------------------------------------------------------------------
const bool use_mmx = true;
//------------------------------------------------------------------------------
// For debugging: purposely clear MMX state, to break any 'bad' code that depends
// on the regs being preserved across calls.
static void empty_mmx(void)
{
#ifdef _DEBUG
  _asm
  {
    pxor mm0, mm0
    pxor mm1, mm1
    pxor mm2, mm2
    pxor mm3, mm3
    pxor mm4, mm4
    pxor mm5, mm5
    pxor mm6, mm6
    pxor mm7, mm7
    emms
  }
#endif
}
//------------------------------------------------------------------------------
int main(int arg_c, char *arg_v[])
{
  printf("JPEG to TGA file conversion example. Compiled %s %s\n", __DATE__, __TIME__);
  printf("Copyright (C) 1994-2000 Rich Geldreich\n");
  printf("MMX enabled: %i\n", use_mmx);

  if (arg_c != 3)
  {
    printf("Usage: jpg2tga <source_file> <dest_file>\n");
    printf("Outputs greyscale and truecolor 24-bit TGA files.\n");
    return (EXIT_FAILURE);
  }

  const char *Psrc_filename = arg_v[1];
  const char *Pdst_filename = arg_v[2];

  printf("Source file:      \"%s\"\n", Psrc_filename);
  printf("Destination file: \"%s\"\n", Pdst_filename);

  Pjpeg_decoder_file_stream Pinput_stream = new jpeg_decoder_file_stream();

  if (Pinput_stream->open(Psrc_filename))
  {
    delete Pinput_stream;
    printf("Error: Unable to open file \"%s\" for reading!\n", Psrc_filename);
    return (EXIT_FAILURE);
  }

  Pjpeg_decoder Pd = new jpeg_decoder(Pinput_stream, use_mmx);

  if (Pd->get_error_code() != 0)
  {
    printf("Error: Decoder failed! Error status: %i\n", Pd->get_error_code());

    // Always be sure to delete the input stream object _after_
    // the decoder is deleted. Reason: the decoder object calls the input
    // stream's detach() method.
    delete Pd;
    delete Pinput_stream;

    return (EXIT_FAILURE);
  }

#ifdef NO_OUTPUT
  Ptga_writer Pdst = NULL;
#else
  Ptga_writer Pdst = new tga_writer();

  bool status = Pdst->open(Pdst_filename,
    Pd->get_width(),
    Pd->get_height(),
    (Pd->get_num_components() == 1) ? TGA_IMAGE_TYPE_GREY : TGA_IMAGE_TYPE_BGR);

  if (status)
  {
    delete Pd;
    delete Pinput_stream;
    delete Pdst;

    printf("Error: Unable to open file \"%s\" for writing!\n", Pdst_filename);

    return (EXIT_FAILURE);
  }
#endif

  printf("Width: %i\n", Pd->get_width());
  printf("Height: %i\n", Pd->get_height());
  printf("Components: %i\n", Pd->get_num_components());

  // for debugging only!
  if (use_mmx)
    empty_mmx();

  if (Pd->begin())
  {
    printf("Error: Decoder failed! Error status: %i\n", Pd->get_error_code());

    delete Pd;
    delete Pinput_stream;
    delete Pdst;
    remove(Pdst_filename);

    return (EXIT_FAILURE);
  }

  uchar *Pbuf = NULL;
  if (Pd->get_num_components() == 3)
  {
    Pbuf = (uchar *)malloc(Pd->get_width() * 3);
    if (!Pbuf)
    {
      printf("Error: Out of memory!\n");

      delete Pd;
      delete Pinput_stream;
      delete Pdst;
      remove(Pdst_filename);

      return (EXIT_FAILURE);
    }
  }

  int lines_decoded = 0;

  for ( ; ; )
  {
    void *Pscan_line_ofs;
    uint scan_line_len;

    if (Pd->decode(&Pscan_line_ofs, &scan_line_len))
      break;

    // for debugging only!
    if (use_mmx)
      empty_mmx();

    lines_decoded++;

#ifndef NO_OUTPUT
    bool status;

    if (Pd->get_num_components() == 3)
    {
      uchar *Psb = (uchar *)Pscan_line_ofs;
      uchar *Pdb = Pbuf;
      int src_bpp = Pd->get_bytes_per_pixel();

      for (int x = Pd->get_width(); x > 0; x--, Psb += src_bpp, Pdb += 3)
      {
        Pdb[0] = Psb[2];
        Pdb[1] = Psb[1];
        Pdb[2] = Psb[0];
      }

      status = Pdst->write_line(Pbuf);
    }
    else
    {
      status = Pdst->write_line(Pscan_line_ofs);
    }

    if (status)
    {
      printf("Error: Unable to write to file \"%s\"!\n", Pdst_filename);

      free(Pbuf);
      delete Pd;
      delete Pinput_stream;
      delete Pdst;
      remove(Pdst_filename);

      return (EXIT_FAILURE);
    }
#endif
  }

  free(Pbuf);

#ifndef NO_OUTPUT
  if (Pdst->close())
  {
    printf("Error: Unable to write to file \"%s\"!\n", Pdst_filename);

    delete Pd;
    delete Pinput_stream;
    delete Pdst;
    remove(Pdst_filename);

    return (EXIT_FAILURE);
  }

  delete Pdst;
#endif

  if (Pd->get_error_code())
  {
    printf("Error: Decoder failed! Error status: %i\n", Pd->get_error_code());

    delete Pd;
    delete Pinput_stream;

    return (EXIT_FAILURE);
  }

  printf("Lines decoded: %i\n", lines_decoded);
  printf("Input file size:  %i\n", Pinput_stream->get_size());
  printf("Input bytes actually read: %i\n", Pd->get_total_bytes_read());

  delete Pd;
  delete Pinput_stream;

  return (EXIT_SUCCESS);
}
//------------------------------------------------------------------------------

