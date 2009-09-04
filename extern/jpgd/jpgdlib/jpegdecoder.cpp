//------------------------------------------------------------------------------
// jpegdecoder.cpp
// Small JPEG Decoder Library v0.93b
// Last updated: Dec. 28, 2001 
// Copyright (C) 1994-2000 Rich Geldreich
// richgel@voicenet.com
// 
// Dec. 19, 2001 - fixed dumb bug in the decode_next_row functions that
// could cause bogus non-zero coefficients from leaking through in rare cases.
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
//------------------------------------------------------------------------------
// Coefficients are stored in this sequence in the data stream.
static int ZAG[64] =
{
  0,  1,  8, 16,  9,  2,  3, 10,
 17, 24, 32, 25, 18, 11,  4,  5,
 12, 19, 26, 33, 40, 48, 41, 34,
 27, 20, 13,  6,  7, 14, 21, 28,
 35, 42, 49, 56, 57, 50, 43, 36,
 29, 22, 15, 23, 30, 37, 44, 51,
 58, 59, 52, 45, 38, 31, 39, 46,
 53, 60, 61, 54, 47, 55, 62, 63,
};
//------------------------------------------------------------------------------
const int AAN_SCALE_BITS = 14;
const int IFAST_SCALE_BITS = 2; /* fractional bits in scale factors */
//------------------------------------------------------------------------------
static int16 aan_scales[64] =
{
  16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
  22725, 31521, 29692, 26722, 22725, 17855, 12299,  6270,
  21407, 29692, 27969, 25172, 21407, 16819, 11585,  5906,
  19266, 26722, 25172, 22654, 19266, 15137, 10426,  5315,
  16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
  12873, 17855, 16819, 15137, 12873, 10114,  6967,  3552,
   8867, 12299, 11585, 10426,  8867,  6967,  4799,  2446,
   4520,  6270,  5906,  5315,  4520,  3552,  2446,  1247
};
//------------------------------------------------------------------------------
// Unconditionally frees all allocated blocks.
void jpeg_decoder::free_all_blocks(void)
{
  if (Pstream)
  {
    Pstream->detach();
    Pstream = NULL;
  }

  for (int i = 0; i < JPGD_MAXBLOCKS; i++)
  {
    free(blocks[i]);
    blocks[i] = NULL;
  }
}
//------------------------------------------------------------------------------
// This method handles all errors.
// It could easily be changed to use C++ exceptions.
void jpeg_decoder::terminate(int status)
{
  error_code = status;

  free_all_blocks();

  longjmp(jmp_state, status);
}
//------------------------------------------------------------------------------
// Allocate a block of memory-- store block's address in list for
// later deallocation by free_all_blocks().
void *jpeg_decoder::alloc(int n)
{
  // Find a free slot. The number of allocated slots will
  // always be very low, so a linear search is good enough.
  for (int i = 0; i < JPGD_MAXBLOCKS; i++)
  {
    if (blocks[i] == NULL)
      break;
  }

  if (i == JPGD_MAXBLOCKS)
    terminate(JPGD_TOO_MANY_BLOCKS);

  void *q = malloc(n + 8);

  if (q == NULL)
    terminate(JPGD_NOTENOUGHMEM);

  memset(q, 0, n + 8);

  blocks[i] = q;

  // Round to qword boundry, to avoid misaligned accesses with MMX code
  return ((void *)(((uint)q + 7) & ~7));
}
//------------------------------------------------------------------------------
// Clear buffer to word values.
void jpeg_decoder::word_clear(void *p, ushort c, uint n)
{
  ushort *ps = (ushort *)p;
  while (n)
  {
    *ps++ = c;
    n--;
  }
}
//------------------------------------------------------------------------------
// Refill the input buffer.
// This method will sit in a loop until (A) the buffer is full or (B)
// the stream's read() method reports and end of file condition.
void jpeg_decoder::prep_in_buffer(void)
{
  in_buf_left = 0;
  Pin_buf_ofs = in_buf;

  if (eof_flag)
    return;

#ifdef SUPPORT_MMX
  bool was_mmx_active = mmx_active;
  if (mmx_active)
    get_bits_2_mmx_deinit();
#endif

  do
  {
    int bytes_read = Pstream->read(in_buf + in_buf_left,
                                   JPGD_INBUFSIZE - in_buf_left,
                                   &eof_flag);

    if (bytes_read == -1)
      terminate(JPGD_STREAM_READ);

    in_buf_left += bytes_read;

  } while ((in_buf_left < JPGD_INBUFSIZE) && (!eof_flag));

  total_bytes_read += in_buf_left;

  word_clear(Pin_buf_ofs + in_buf_left, 0xD9FF, 64);

#ifdef SUPPORT_MMX
  if (was_mmx_active)
    get_bits_2_mmx_init();
#endif
}
//------------------------------------------------------------------------------
// Read a Huffman code table.
void jpeg_decoder::read_dht_marker(void)
{
  int i, index, count;
  uint left;
  uchar huff_num[17];
  uchar huff_val[256];

  left = get_bits_1(16);

  if (left < 2)
    terminate(JPGD_BAD_DHT_MARKER);

  left -= 2;

  while (left)
  {
    index = get_bits_1(8);

    huff_num[0] = 0;

    count = 0;

    for (i = 1; i <= 16; i++)
    {
      huff_num[i] = get_bits_1(8);
      count += huff_num[i];
    }

    if (count > 255)
      terminate(JPGD_BAD_DHT_COUNTS);

    for (i = 0; i < count; i++)
      huff_val[i] = get_bits_1(8);

    i = 1 + 16 + count;

    if (left < (uint)i)
      terminate(JPGD_BAD_DHT_MARKER);

    left -= i;

    if ((index & 0x10) > 0x10)
      terminate(JPGD_BAD_DHT_INDEX);

    index = (index & 0x0F) + ((index & 0x10) >> 4) * (JPGD_MAXHUFFTABLES >> 1);

    if (index >= JPGD_MAXHUFFTABLES)
      terminate(JPGD_BAD_DHT_INDEX);

    if (!this->huff_num[index])
      this->huff_num[index] = (uchar *)alloc(17);

    if (!this->huff_val[index])
      this->huff_val[index] = (uchar *)alloc(256);

    memcpy(this->huff_num[index], huff_num, 17);
    memcpy(this->huff_val[index], huff_val, 256);
  }
}
//------------------------------------------------------------------------------
// Read a quantization table.
void jpeg_decoder::read_dqt_marker(void)
{
  int n, i, prec;
  uint left;
  uint temp;

  left = get_bits_1(16);

  if (left < 2)
    terminate(JPGD_BAD_DQT_MARKER);

  left -= 2;

  while (left)
  {
    n = get_bits_1(8);
    prec = n >> 4;
    n &= 0x0F;

    if (n >= JPGD_MAXQUANTTABLES)
      terminate(JPGD_BAD_DQT_TABLE);

    if (!quant[n])
      quant[n] = (QUANT_TYPE *)alloc(64 * sizeof(QUANT_TYPE));

    // read quantization entries, in zag order
    for (i = 0; i < 64; i++)
    {
      temp = get_bits_1(8);

      if (prec)
        temp = (temp << 8) + get_bits_1(8);

      if (use_mmx_idct)
        quant[n][ZAG[i]] = (temp * aan_scales[ZAG[i]] + (1 << (AAN_SCALE_BITS - IFAST_SCALE_BITS - 1))) >> (AAN_SCALE_BITS - IFAST_SCALE_BITS);
      else
        quant[n][i] = temp;
    }

    i = 64 + 1;

    if (prec)
      i += 64;

    if (left < (uint)i)
      terminate(JPGD_BAD_DQT_LENGTH);

    left -= i;
  }
}
//------------------------------------------------------------------------------
// Read the start of frame (SOF) marker.
void jpeg_decoder::read_sof_marker(void)
{
  int i;
  uint left;

  left = get_bits_1(16);

  if (get_bits_1(8) != 8)   /* precision: sorry, only 8-bit precision is supported right now */
    terminate(JPGD_BAD_PRECISION);

  image_y_size = get_bits_1(16);

  if ((image_y_size < 1) || (image_y_size > JPGD_MAX_HEIGHT))
    terminate(JPGD_BAD_HEIGHT);

  image_x_size = get_bits_1(16);

  if ((image_x_size < 1) || (image_x_size > JPGD_MAX_WIDTH))
    terminate(JPGD_BAD_WIDTH);

  comps_in_frame = get_bits_1(8);

  if (comps_in_frame > JPGD_MAXCOMPONENTS)
    terminate(JPGD_TOO_MANY_COMPONENTS);

  if (left != (uint)(comps_in_frame * 3 + 8))
    terminate(JPGD_BAD_SOF_LENGTH);

  for (i = 0; i < comps_in_frame; i++)
  {
    comp_ident[i]  = get_bits_1(8);
    comp_h_samp[i] = get_bits_1(4);
    comp_v_samp[i] = get_bits_1(4);
    comp_quant[i]  = get_bits_1(8);
  }
}
//------------------------------------------------------------------------------
// Used to skip unrecognized markers.
void jpeg_decoder::skip_variable_marker(void)
{
  uint left;

  left = get_bits_1(16);

  if (left < 2)
    terminate(JPGD_BAD_VARIABLE_MARKER);

  left -= 2;

  while (left)
  {
    get_bits_1(8);
    left--;
  }
}
//------------------------------------------------------------------------------
// Read a define restart interval (DRI) marker.
void jpeg_decoder::read_dri_marker(void)
{
  if (get_bits_1(16) != 4)
    terminate(JPGD_BAD_DRI_LENGTH);

  restart_interval = get_bits_1(16);
}
//------------------------------------------------------------------------------
// Read a start of scan (SOS) marker.
void jpeg_decoder::read_sos_marker(void)
{
  uint left;
  int i, ci, n, c, cc;

  left = get_bits_1(16);

  n = get_bits_1(8);

  comps_in_scan = n;

  left -= 3;

  if ( (left != (uint)(n * 2 + 3)) || (n < 1) || (n > JPGD_MAXCOMPSINSCAN) )
    terminate(JPGD_BAD_SOS_LENGTH);

  for (i = 0; i < n; i++)
  {
    cc = get_bits_1(8);
    c = get_bits_1(8);
    left -= 2;

    for (ci = 0; ci < comps_in_frame; ci++)
      if (cc == comp_ident[ci])
        break;

    if (ci >= comps_in_frame)
      terminate(JPGD_BAD_SOS_COMP_ID);

    comp_list[i]    = ci;
    comp_dc_tab[ci] = (c >> 4) & 15;
    comp_ac_tab[ci] = (c & 15) + (JPGD_MAXHUFFTABLES >> 1);
  }

  spectral_start  = get_bits_1(8);
  spectral_end    = get_bits_1(8);
  successive_high = get_bits_1(4);
  successive_low  = get_bits_1(4);

  if (!progressive_flag)
  {
    spectral_start = 0;
    spectral_end = 63;
  }

  left -= 3;

  while (left)                  /* read past whatever is left */
  {
    get_bits_1(8);
    left--;
  }
}
//------------------------------------------------------------------------------
// Finds the next marker.
int jpeg_decoder::next_marker(void)
{
  uint c, bytes;

  bytes = 0;

  do
  {
    do
    {
      bytes++;

      c = get_bits_1(8);

    } while (c != 0xFF);

    do
    {
      c = get_bits_1(8);

    } while (c == 0xFF);

  } while (c == 0);

  // If bytes > 0 here, there where extra bytes before the marker (not good).

  return c;
}
//------------------------------------------------------------------------------
// Process markers. Returns when an SOFx, SOI, EOI, or SOS marker is
// encountered.
int jpeg_decoder::process_markers(void)
{
  int c;

  for ( ; ; )
  {
    c = next_marker();

    switch (c)
    {
      case M_SOF0:
      case M_SOF1:
      case M_SOF2:
      case M_SOF3:
      case M_SOF5:
      case M_SOF6:
      case M_SOF7:
//      case M_JPG:
      case M_SOF9:
      case M_SOF10:
      case M_SOF11:
      case M_SOF13:
      case M_SOF14:
      case M_SOF15:
      case M_SOI:
      case M_EOI:
      case M_SOS:
      {
        return c;
      }
      case M_DHT:
      {
        read_dht_marker();
        break;
      }
      // Sorry, no arithmitic support at this time. Dumb patents!
      case M_DAC:
      {
        terminate(JPGD_NO_ARITHMITIC_SUPPORT);
        break;
      }
      case M_DQT:
      {
        read_dqt_marker();
        break;
      }
      case M_DRI:
      {
        read_dri_marker();
        break;
      }
      //case M_APP0:  /* no need to read the JFIF marker */

      case M_JPG:
      case M_RST0:    /* no parameters */
      case M_RST1:
      case M_RST2:
      case M_RST3:
      case M_RST4:
      case M_RST5:
      case M_RST6:
      case M_RST7:
      case M_TEM:
      {
        terminate(JPGD_UNEXPECTED_MARKER);
        break;
      }
      default:    /* must be DNL, DHP, EXP, APPn, JPGn, COM, or RESn or APP0 */
      {
        skip_variable_marker();
        break;
      }

    }
  }
}
//------------------------------------------------------------------------------
// Finds the start of image (SOI) marker.
// This code is rather defensive: it only checks the first 512 bytes to avoid
// false positives.
void jpeg_decoder::locate_soi_marker(void)
{
  uint lastchar, thischar;
  ulong bytesleft;

  lastchar = get_bits_1(8);

  thischar = get_bits_1(8);

  /* ok if it's a normal JPEG file without a special header */

  if ((lastchar == 0xFF) && (thischar == M_SOI))
    return;

  bytesleft = 512;

  for ( ; ; )
  {
    if (--bytesleft == 0)
      terminate(JPGD_NOT_JPEG);

    lastchar = thischar;

    thischar = get_bits_1(8);

    if ((lastchar == 0xFF) && (thischar == M_SOI))
      break;
  }

  /* Check the next character after marker: if it's not 0xFF, it can't
     be the start of the next marker, so it probably isn't a JPEG */

  thischar = (bit_buf >> 8) & 0xFF;

  if (thischar != 0xFF)
    terminate(JPGD_NOT_JPEG);
}
//------------------------------------------------------------------------------
// Find a start of frame (SOF) marker.
void jpeg_decoder::locate_sof_marker(void)
{
  int c;

  locate_soi_marker();

  c = process_markers();

  switch (c)
  {
    case M_SOF2:
      progressive_flag = TRUE;
    case M_SOF0:  /* baseline DCT */
    case M_SOF1:  /* extended sequential DCT */
    {
      read_sof_marker();
      break;
    }
    case M_SOF9:  /* Arithmitic coding */
    {
      terminate(JPGD_NO_ARITHMITIC_SUPPORT);
      break;
    }

    default:
    {
      terminate(JPGD_UNSUPPORTED_MARKER);
      break;
    }
  }
}
//------------------------------------------------------------------------------
// Find a start of scan (SOS) marker.
int jpeg_decoder::locate_sos_marker(void)
{
  int c;

  c = process_markers();

  if (c == M_EOI)
    return FALSE;
  else if (c != M_SOS)
    terminate(JPGD_UNEXPECTED_MARKER);

  read_sos_marker();

  return TRUE;
}
//------------------------------------------------------------------------------
// Reset everything to default/uninitialized state.
void jpeg_decoder::init(Pjpeg_decoder_stream Pstream, bool use_mmx)
{
  error_code = 0;

  ready_flag = false;

  image_x_size = image_y_size = 0;

  this->Pstream = Pstream;

#ifdef SUPPORT_MMX
  this->use_mmx = use_mmx;
  use_mmx_idct = (use_mmx) && (jpeg_idct_ifast_avail());
#else
  this->use_mmx = false;
  use_mmx_idct = false;
#endif

  progressive_flag = FALSE;

  memset(huff_num, 0, sizeof(huff_num));
  memset(huff_val, 0, sizeof(huff_val));
  memset(quant, 0, sizeof(quant));

  scan_type = 0;

  comps_in_frame = 0;

  memset(comp_h_samp, 0, sizeof(comp_h_samp));
  memset(comp_v_samp, 0, sizeof(comp_v_samp));
  memset(comp_quant, 0, sizeof(comp_quant));
  memset(comp_ident, 0, sizeof(comp_ident));
  memset(comp_h_blocks, 0, sizeof(comp_h_blocks));
  memset(comp_v_blocks, 0, sizeof(comp_v_blocks));

  comps_in_scan = 0;
  memset(comp_list, 0, sizeof(comp_list));
  memset(comp_dc_tab, 0, sizeof(comp_dc_tab));
  memset(comp_ac_tab, 0, sizeof(comp_ac_tab));

  spectral_start = 0;
  spectral_end = 0;
  successive_low = 0;
  successive_high = 0;

  max_mcu_x_size = 0;
  max_mcu_y_size = 0;

  blocks_per_mcu = 0;
  max_blocks_per_row = 0;
  mcus_per_row = 0;
  mcus_per_col = 0;

  memset(mcu_org, 0, sizeof(mcu_org));

  total_lines_left = 0;
  mcu_lines_left = 0;

  real_dest_bytes_per_scan_line = 0;
  dest_bytes_per_scan_line = 0;
  dest_bytes_per_pixel = 0;

  memset(blocks, 0, sizeof(blocks));

  memset(h, 0, sizeof(h));

  memset(dc_coeffs, 0, sizeof(dc_coeffs));
  memset(ac_coeffs, 0, sizeof(ac_coeffs));
  memset(block_y_mcu, 0, sizeof(block_y_mcu));

  eob_run = 0;

  memset(block_y_mcu, 0, sizeof(block_y_mcu));

  Pin_buf_ofs = in_buf;
  in_buf_left = 0;
  eof_flag = false;
  tem_flag = 0;

  memset(padd_1, 0, sizeof(padd_1));
  memset(in_buf, 0, sizeof(in_buf));
  memset(padd_2, 0, sizeof(padd_2));

  restart_interval = 0;
  restarts_left    = 0;
  next_restart_num = 0;

  max_mcus_per_row = 0;
  max_blocks_per_mcu = 0;
  max_mcus_per_col = 0;

  memset(component, 0, sizeof(component));
  memset(last_dc_val, 0, sizeof(last_dc_val));
  memset(dc_huff_seg, 0, sizeof(dc_huff_seg));
  memset(ac_huff_seg, 0, sizeof(ac_huff_seg));
  memset(block_seg, 0, sizeof(block_seg));
  Psample_buf = NULL;

  total_bytes_read = 0;

  // Tell the stream we're going to use it.
  Pstream->attach();

  use_mmx_getbits = false;
  mmx_active = false;

  // Ready the input buffer.
  prep_in_buffer();

  // Prime the bit buffer.
  bits_left = 16;
  bit_buf_64[0] = 0;
  bit_buf_64[1] = 0;

  get_bits_1(16);
  get_bits_1(16);

  for (int i = 0; i < JPGD_MAXBLOCKSPERROW; i++)
    block_max_zag_set[i] = 64;
}
//------------------------------------------------------------------------------
#define SCALEBITS 16
#define ONE_HALF ((long) 1 << (SCALEBITS-1))
#define FIX(x) ((long) ((x) * (1L<<SCALEBITS) + 0.5))
//------------------------------------------------------------------------------
// Create a few tables that allow us to quickly convert YCbCr to RGB.
void jpeg_decoder::create_look_ups(void)
{
  int i, k;

  for (i = 0; i <= 255; i++)
  {
    //k = (i * 2) - 255;
		k = (i * 2) - 256; // Dec. 28 2001- change so table[128] == 0

    crr[i] = ( FIX(1.40200/2)  * k + ONE_HALF) >> SCALEBITS;
    cbb[i] = ( FIX(1.77200/2)  * k + ONE_HALF) >> SCALEBITS;

    crg[i] = (-FIX(0.71414/2)) * k;
    cbg[i] = (-FIX(0.34414/2)) * k + ONE_HALF;
  }
}
//------------------------------------------------------------------------------
// This method throws back into the stream any bytes that where read
// into the bit buffer during initial marker scanning.
void jpeg_decoder::fix_in_buffer(void)
{
  /* In case any 0xFF's where pulled into the buffer during marker scanning */

  if (bits_left == 16)
    stuff_char( (uchar)((bit_buf >> 16) & 0xFF));

  if (bits_left >= 8)
    stuff_char( (uchar)((bit_buf >> 24) & 0xFF));

  stuff_char( (uchar)(bit_buf & 0xFF) );

  stuff_char( (uchar)((bit_buf >> 8) & 0xFF) );

#ifdef SUPPORT_MMX
  if (use_mmx_getbits)
  {
    bits_left = 48;
    get_bits_2_mmx_init();
    get_bits_2_mmx(16);
    get_bits_2_mmx(16);
    get_bits_2_mmx(16);
    get_bits_2_mmx(16);
    get_bits_2_mmx_deinit();
  }
  else
#endif
  {
    bits_left = 16;
    //bit_buf = 0;
    get_bits_2(16);
    get_bits_2(16);
  }
}
//------------------------------------------------------------------------------
// Performs a 2D IDCT over the entire row's coefficient buffer.
void jpeg_decoder::transform_row(void)
{
#ifdef SUPPORT_MMX
  if (use_mmx_idct)
  {
    BLOCK_TYPE *Psrc_ptr = block_seg[0];
    uchar *Pdst_ptr = Psample_buf - 8;

    for (int mcu_row = 0; mcu_row < mcus_per_row; mcu_row++)
    {
      for (int mcu_block = 0; mcu_block < blocks_per_mcu; mcu_block++)
      {
        int component_id = mcu_org[mcu_block];
        QUANT_TYPE *Pquant_ptr = quant[comp_quant[component_id]];

        uchar * outptr[8];
        outptr[0] = Pdst_ptr;
        outptr[1] = Pdst_ptr+8*1;
        outptr[2] = Pdst_ptr+8*2;
        outptr[3] = Pdst_ptr+8*3;
        outptr[4] = Pdst_ptr+8*4;
        outptr[5] = Pdst_ptr+8*5;
        outptr[6] = Pdst_ptr+8*6;
        outptr[7] = Pdst_ptr+8*7;

        jpeg_idct_ifast(
          Psrc_ptr,
          Pquant_ptr,
          outptr,
          8);

        Psrc_ptr += 64;
        Pdst_ptr += 64;
      }
    }

    jpeg_idct_ifast_deinit();
  }
  else
#endif
  {
    BLOCK_TYPE *Psrc_ptr = block_seg[0];
    uchar *Pdst_ptr = Psample_buf;

    for (int i = max_blocks_per_row; i > 0; i--)
    {
      // Copy the block to a temp. buffer to prevent the IDCT
      // from modifying the entire block.
      memcpy(temp_block, Psrc_ptr, 64 * sizeof(BLOCK_TYPE));
      idct(temp_block, Pdst_ptr);
      Psrc_ptr += 64;
      Pdst_ptr += 64;
    }
  }
}
//------------------------------------------------------------------------------
// The coeff_buf series of methods originally stored the coefficients
// into a "virtual" file which was located in EMS, XMS, or a disk file. A cache
// was used to make this process more efficient. Now, we can store the entire
// thing in RAM.
Pcoeff_buf_t jpeg_decoder::coeff_buf_open(
  int block_num_x, int block_num_y,
  int block_len_x, int block_len_y)
{
  Pcoeff_buf_t cb = (Pcoeff_buf_t)alloc(sizeof(coeff_buf_t));

  cb->block_num_x = block_num_x;
  cb->block_num_y = block_num_y;

  cb->block_len_x = block_len_x;
  cb->block_len_y = block_len_y;

  cb->block_size = (block_len_x * block_len_y) * sizeof(BLOCK_TYPE);

  cb->Pdata = (uchar *)alloc(cb->block_size * block_num_x * block_num_y);

  return cb;
}
//------------------------------------------------------------------------------
void jpeg_decoder::coeff_buf_read(
  Pcoeff_buf_t cb,
  int block_x, int block_y,
  BLOCK_TYPE *buffer)
{
  if (block_x >= cb->block_num_x)
    terminate(JPGD_ASSERTION_ERROR);

  if (block_y >= cb->block_num_y)
    terminate(JPGD_ASSERTION_ERROR);

  memcpy(buffer,
         cb->Pdata + block_x * cb->block_size + block_y * (cb->block_size * cb->block_num_x),
         cb->block_size);
}
//------------------------------------------------------------------------------
void jpeg_decoder::coeff_buf_write(
  Pcoeff_buf_t cb,
  int block_x, int block_y,
  BLOCK_TYPE *buffer)
{
  if (block_x >= cb->block_num_x)
    terminate(JPGD_ASSERTION_ERROR);

  if (block_y >= cb->block_num_y)
    terminate(JPGD_ASSERTION_ERROR);

  memcpy(cb->Pdata + block_x * cb->block_size + block_y * (cb->block_size * cb->block_num_x),
         buffer,
         cb->block_size);
}
//------------------------------------------------------------------------------
BLOCK_TYPE *jpeg_decoder::coeff_buf_getp(
  Pcoeff_buf_t cb,
  int block_x, int block_y)
{
  if (block_x >= cb->block_num_x)
    terminate(JPGD_ASSERTION_ERROR);

  if (block_y >= cb->block_num_y)
    terminate(JPGD_ASSERTION_ERROR);

  return (BLOCK_TYPE *)(cb->Pdata + block_x * cb->block_size + block_y * (cb->block_size * cb->block_num_x));
}
//------------------------------------------------------------------------------
// Loads and dequantizes the next row of (already decoded) coefficients.
// Progressive images only.
void jpeg_decoder::load_next_row(void)
{
  int i;
  BLOCK_TYPE *p;
  QUANT_TYPE *q;
  int mcu_row, mcu_block, row_block = 0;
  int component_num, component_id;
  int block_x_mcu[JPGD_MAXCOMPONENTS];

  memset(block_x_mcu, 0, JPGD_MAXCOMPONENTS * sizeof(int));

  for (mcu_row = 0; mcu_row < mcus_per_row; mcu_row++)
  {
    int block_x_mcu_ofs = 0, block_y_mcu_ofs = 0;

    for (mcu_block = 0; mcu_block < blocks_per_mcu; mcu_block++)
    {
      component_id = mcu_org[mcu_block];

      p = block_seg[row_block];
      q = quant[comp_quant[component_id]];

      BLOCK_TYPE *pAC = coeff_buf_getp(ac_coeffs[component_id],
                                  block_x_mcu[component_id] + block_x_mcu_ofs,
                                  block_y_mcu[component_id] + block_y_mcu_ofs);

      BLOCK_TYPE *pDC = coeff_buf_getp(dc_coeffs[component_id],
                                  block_x_mcu[component_id] + block_x_mcu_ofs,
                                  block_y_mcu[component_id] + block_y_mcu_ofs);
      p[0] = pDC[0];
      memcpy(&p[1], &pAC[1], 63 * sizeof(BLOCK_TYPE));

      if (!use_mmx_idct)
      {
        for (i = 63; i > 0; i--)
          if (p[ZAG[i]])
            break;

        //block_num[row_block++] = i + 1;

        for ( ; i >= 0; i--)
          if (p[ZAG[i]])
            p[ZAG[i]] *= q[i];
      }

      row_block++;

      if (comps_in_scan == 1)
        block_x_mcu[component_id]++;
      else
      {
        if (++block_x_mcu_ofs == comp_h_samp[component_id])
        {
          block_x_mcu_ofs = 0;

          if (++block_y_mcu_ofs == comp_v_samp[component_id])
          {
            block_y_mcu_ofs = 0;

            block_x_mcu[component_id] += comp_h_samp[component_id];
          }
        }
      }
    }
  }

  if (comps_in_scan == 1)
    block_y_mcu[comp_list[0]]++;
  else
  {
    for (component_num = 0; component_num < comps_in_scan; component_num++)
    {
      component_id = comp_list[component_num];

      block_y_mcu[component_id] += comp_v_samp[component_id];
    }
  }
}
//------------------------------------------------------------------------------
// Restart interval processing.
void jpeg_decoder::process_restart(void)
{
  int i, c;

  // Align to a byte boundry
  // FIXME: Is this really necessary? get_bits_2() never reads in markers!
  //get_bits_2(bits_left & 7);

  // Let's scan a little bit to find the marker, but not _too_ far.
  // 1536 is a "fudge factor" that determines how much to scan.
  for (i = 1536; i > 0; i--)
    if (get_char() == 0xFF)
      break;

  if (i == 0)
    terminate(JPGD_BAD_RESTART_MARKER);

  for ( ; i > 0; i--)
    if ((c = get_char()) != 0xFF)
      break;

  if (i == 0)
    terminate(JPGD_BAD_RESTART_MARKER);

  // Is it the expected marker? If not, something bad happened.
  if (c != (next_restart_num + M_RST0))
    terminate(JPGD_BAD_RESTART_MARKER);

  // Reset each component's DC prediction values.
  memset(&last_dc_val, 0, comps_in_frame * sizeof(uint));

  eob_run = 0;

  restarts_left = restart_interval;

  next_restart_num = (next_restart_num + 1) & 7;

  // Get the bit buffer going again...

#ifdef SUPPORT_MMX
  if (use_mmx_getbits)
  {
    bits_left = 48;
    get_bits_2_mmx(16);
    get_bits_2_mmx(16);
    get_bits_2_mmx(16);
    get_bits_2_mmx(16);
  }
  else
#endif
  {
    bits_left = 16;
    //bit_buf = 0;
    get_bits_2(16);
    get_bits_2(16);
  }
}
//------------------------------------------------------------------------------
// Decodes and dequantizes the next row of coefficients.
void jpeg_decoder::decode_next_row(void)
{
  int row_block = 0;

  // Clearing the entire row block buffer can take a lot of time!
  // Instead of clearing the entire buffer each row, keep track
  // of the number of nonzero entries written to each block and do
  // selective clears.
  //memset(block_seg[0], 0, mcus_per_row * blocks_per_mcu * 64 * sizeof(BLOCK_TYPE));

  for (int mcu_row = 0; mcu_row < mcus_per_row; mcu_row++)
  {
    if ((restart_interval) && (restarts_left == 0))
      process_restart();

    for (int mcu_block = 0; mcu_block < blocks_per_mcu; mcu_block++)
    {
      int component_id = mcu_org[mcu_block];

      BLOCK_TYPE *p = block_seg[row_block];
      QUANT_TYPE *q = quant[comp_quant[component_id]];
      int r, s;

      if ((s = huff_decode(h[comp_dc_tab[component_id]])) != 0)
      {
        r = get_bits_2(s);
        s = HUFF_EXTEND(r, s);
      }

      last_dc_val[component_id] = (s += last_dc_val[component_id]);

      if (use_mmx_idct)
        p[0] = s;
      else
        p[0] = s * q[0];

      int prev_num_set = block_max_zag_set[row_block];

      Phuff_tables_t Ph = h[comp_ac_tab[component_id]];

      for (int k = 1; k < 64; k++)
      {
        s = huff_decode(Ph);

        r = s >> 4;
        s &= 15;

        if (s)
        {
          if (r)
          {
            if ((k + r) > 63)
              terminate(JPGD_DECODE_ERROR);

            if (k < prev_num_set)
            {
              int n = min(r, prev_num_set - k);
              int kt = k;
              while (n--)
                p[ZAG[kt++]] = 0;
            }

            k += r;
          }

          r = get_bits_2(s);
          s = HUFF_EXTEND(r, s);

          //assert(k < 64);

          if (use_mmx_idct)
            p[ZAG[k]] = s;
          else
            p[ZAG[k]] = s * q[k];
        }
        else
        {
          if (r == 15)
          {
            if ((k + 15) > 63)
              terminate(JPGD_DECODE_ERROR);

            if (k < prev_num_set)
            {
              int n = min(16, prev_num_set - k);		//bugfix Dec. 19, 2001 - was 15!
              int kt = k;
              while (n--)
                p[ZAG[kt++]] = 0;
            }

            k += 15;
          }
          else
          {
            //while (k < 64)
            //  p[ZAG[k++]] = 0;

            break;
          }
        }
      }

      if (k < prev_num_set)
      {
        int kt = k;
        while (kt < prev_num_set)
          p[ZAG[kt++]] = 0;
      }

      block_max_zag_set[row_block] = k;

      //block_num[row_block++] = k;
      row_block++;
    }

    restarts_left--;
  }
}
//------------------------------------------------------------------------------
#ifdef SUPPORT_MMX
void jpeg_decoder::decode_next_row_mmx(void)
{
  int row_block = 0;

  // Clearing the entire row block buffer can take a lot of time!
  // Instead of clearing the entire buffer each row, keep track
  // of the number of nonzero entries written to each block and do
  // selective clears.
  //memset(block_seg[0], 0, mcus_per_row * blocks_per_mcu * 64 * sizeof(BLOCK_TYPE));

  get_bits_2_mmx_init();

  for (int mcu_row = 0; mcu_row < mcus_per_row; mcu_row++)
  {
    if ((restart_interval) && (restarts_left == 0))
      process_restart();

    for (int mcu_block = 0; mcu_block < blocks_per_mcu; mcu_block++)
    {
      int component_id = mcu_org[mcu_block];

      BLOCK_TYPE *p = block_seg[row_block];
      QUANT_TYPE *q = quant[comp_quant[component_id]];
      int r, s;

      if ((s = huff_decode_mmx(h[comp_dc_tab[component_id]])) != 0)
      {
        r = get_bits_2_mmx(s);
        s = HUFF_EXTEND(r, s);
      }

      last_dc_val[component_id] = (s += last_dc_val[component_id]);

      if (use_mmx_idct)
        p[0] = s;
      else
        p[0] = s * q[0];

      int prev_num_set = block_max_zag_set[row_block];

      Phuff_tables_t Ph = h[comp_ac_tab[component_id]];

      for (int k = 1; k < 64; k++)
      {
        s = huff_decode_mmx(Ph);

        r = s >> 4;
        s &= 15;

        if (s)
        {
          if (r)
          {
            if ((k + r) > 63)
              terminate(JPGD_DECODE_ERROR);

            if (k < prev_num_set)
            {
              int n = min(r, prev_num_set - k);
              int kt = k;
              while (n--)
                p[ZAG[kt++]] = 0;
            }

            k += r;
          }

          r = get_bits_2_mmx(s);
          s = HUFF_EXTEND(r, s);

          assert(k < 64);

          if (use_mmx_idct)
            p[ZAG[k]] = s;
          else
            p[ZAG[k]] = s * q[k];
        }
        else
        {
          if (r == 15)
          {
            if ((k + 15) > 63)
              terminate(JPGD_DECODE_ERROR);

            if (k < prev_num_set)
            {
              int n = min(16, prev_num_set - k);		//bugfix Dec. 19, 2001 - was 15!
              int kt = k;
              while (n--)
                p[ZAG[kt++]] = 0;
            }

            k += 15;
          }
          else
            break;
        }
      }

      if (k < prev_num_set)
      {
        int kt = k;
        while (kt < prev_num_set)
          p[ZAG[kt++]] = 0;
      }

      block_max_zag_set[row_block] = k;

      //block_num[row_block++] = k;
      row_block++;
    }

    restarts_left--;
  }

  get_bits_2_mmx_deinit();
}
#endif
//------------------------------------------------------------------------------
// YCbCr H1V1 (1x1:1:1, 3 blocks per MCU) to 24-bit RGB
void jpeg_decoder::H1V1Convert(void)
{
  int row = max_mcu_y_size - mcu_lines_left;
  uchar *d = scan_line_0;
  uchar *s = Psample_buf + row * 8;

  for (int i = max_mcus_per_row; i > 0; i--)
  {
    for (int j = 0; j < 8; j++)
    {
      int y = s[j];
      int cb = s[64+j];
      int cr = s[128+j];

      d[0] = clamp(y + crr[cr]);
      d[1] = clamp(y + ((crg[cr] + cbg[cb]) >> 16));
      d[2] = clamp(y + cbb[cb]);
      d += 4;
    }

    s += 64*3;
  }
}
//------------------------------------------------------------------------------
// YCbCr H2V1 (2x1:1:1, 4 blocks per MCU) to 24-bit RGB
void jpeg_decoder::H2V1Convert(void)
{
  int row = max_mcu_y_size - mcu_lines_left;
  uchar *d0 = scan_line_0;
  uchar *y = Psample_buf + row * 8;
  uchar *c = Psample_buf + 2*64 + row * 8;

  for (int i = max_mcus_per_row; i > 0; i--)
  {
    for (int l = 0; l < 2; l++)
    {
      for (int j = 0; j < 4; j++)
      {
        int cb = c[0];
        int cr = c[64];

        int rc = crr[cr];
        int gc = ((crg[cr] + cbg[cb]) >> 16);
        int bc = cbb[cb];

        int yy = y[j<<1];
        d0[0] = clamp(yy+rc);
        d0[1] = clamp(yy+gc);
        d0[2] = clamp(yy+bc);

        yy = y[(j<<1)+1];
        d0[4] = clamp(yy+rc);
        d0[5] = clamp(yy+gc);
        d0[6] = clamp(yy+bc);

        d0 += 8;

        c++;
      }
      y += 64;
    }

    y += 64*4 - 64*2;
    c += 64*4 - 8;
  }
}
//------------------------------------------------------------------------------
// YCbCr H2V1 (1x2:1:1, 4 blocks per MCU) to 24-bit RGB
void jpeg_decoder::H1V2Convert(void)
{
  int row = max_mcu_y_size - mcu_lines_left;
  uchar *d0 = scan_line_0;
  uchar *d1 = scan_line_1;
  uchar *y;
  uchar *c;

  if (row < 8)
    y = Psample_buf + row * 8;
  else
    y = Psample_buf + 64*1 + (row & 7) * 8;

  c = Psample_buf + 64*2 + (row >> 1) * 8;

  for (int i = max_mcus_per_row; i > 0; i--)
  {
    for (int j = 0; j < 8; j++)
    {
      int cb = c[0+j];
      int cr = c[64+j];

      int rc = crr[cr];
      int gc = ((crg[cr] + cbg[cb]) >> 16);
      int bc = cbb[cb];

      int yy = y[j];
      d0[0] = clamp(yy+rc);
      d0[1] = clamp(yy+gc);
      d0[2] = clamp(yy+bc);

      yy = y[8+j];
      d1[0] = clamp(yy+rc);
      d1[1] = clamp(yy+gc);
      d1[2] = clamp(yy+bc);

      d0 += 4;
      d1 += 4;
    }

    y += 64*4;
    c += 64*4;
  }
}
//------------------------------------------------------------------------------
// Y (1 block per MCU) to 8-bit greyscale
void jpeg_decoder::GrayConvert(void)
{
  int row = max_mcu_y_size - mcu_lines_left;
  uchar *d = scan_line_0;
  uchar *s = Psample_buf + row * 8;

  for (int i = max_mcus_per_row; i > 0; i--)
  {
#if 0
    d[0] = s[0];
    d[1] = s[1];
    d[2] = s[2];
    d[3] = s[3];
    d[4] = s[4];
    d[5] = s[5];
    d[6] = s[6];
    d[7] = s[7];
#endif
    *(uint *)d = *(uint *)s;
    *(uint *)(&d[4]) = *(uint *)(&s[4]);

    s += 64;
    d += 8;
  }
}
//------------------------------------------------------------------------------
// Find end of image (EOI) marker, so we can return to the user the
// exact size of the input stream.
void jpeg_decoder::find_eoi(void)
{
  if (!progressive_flag)
  {
    // Attempt to read the EOI marker.
    //get_bits_2(bits_left & 7);

    // Prime the bit buffer
    bits_left = 16;
    //bit_buf = 0;
    get_bits_1(16);
    get_bits_1(16);

    // The next marker _should_ be EOI
    process_markers();
  }

  total_bytes_read -= in_buf_left;
}
//------------------------------------------------------------------------------
// Returns the next scan line.
// Returns JPGD_DONE if all scan lines have been returned.
// Returns JPGD_OKAY if a scan line has been returned.
// Returns JPGD_FAILED if an error occured.
int jpeg_decoder::decode(
  void * *Pscan_line_ofs, uint *Pscan_line_len)
{
  if ((error_code) || (!ready_flag))
    return (JPGD_FAILED);

  if (total_lines_left == 0)
    return (JPGD_DONE);

  if (mcu_lines_left == 0)
  {
    if (setjmp(jmp_state))
      return (JPGD_DECODE_ERROR);

    if (progressive_flag)
      load_next_row();
    else
    {
#ifdef SUPPORT_MMX
      if (use_mmx_getbits)
        decode_next_row_mmx();
      else
#endif
        decode_next_row();
    }

    // Find the EOI marker if that was the last row.
    if (total_lines_left <= max_mcu_y_size)
      find_eoi();

    transform_row();

    mcu_lines_left = max_mcu_y_size;
  }

  switch (scan_type)
  {
    case JPGD_YH2V2:
    {
      if ((mcu_lines_left & 1) == 0)
      {
        H2V2Convert();
        *Pscan_line_ofs = scan_line_0;
      }
      else
        *Pscan_line_ofs = scan_line_1;

      break;
    }
    case JPGD_YH2V1:
    {
      H2V1Convert();
      *Pscan_line_ofs = scan_line_0;
      break;
    }
    case JPGD_YH1V2:
    {
      if ((mcu_lines_left & 1) == 0)
      {
        H1V2Convert();
        *Pscan_line_ofs = scan_line_0;
      }
      else
        *Pscan_line_ofs = scan_line_1;

      break;
    }
    case JPGD_YH1V1:
    {
      H1V1Convert();
      *Pscan_line_ofs = scan_line_0;
      break;
    }
    case JPGD_GRAYSCALE:
    {
      GrayConvert();
      *Pscan_line_ofs = scan_line_0;

      break;
    }
  }

  *Pscan_line_len = real_dest_bytes_per_scan_line;

  mcu_lines_left--;
  total_lines_left--;

  return (JPGD_OKAY);
}
//------------------------------------------------------------------------------
// Creates the tables needed for efficient Huffman decoding.
void jpeg_decoder::make_huff_table(
  int index,
  Phuff_tables_t hs)
{
  int p, i, l, si;
  uchar huffsize[257];
  uint huffcode[257];
  uint code;
  uint subtree;
  int code_size;
  int lastp;
  int nextfreeentry;
  int currententry;

  p = 0;

  for (l = 1; l <= 16; l++)
  {
    for (i = 1; i <= huff_num[index][l]; i++)
      huffsize[p++] = l;
  }

  huffsize[p] = 0;

  lastp = p;

  code = 0;
  si = huffsize[0];
  p = 0;

  while (huffsize[p])
  {
    while (huffsize[p] == si)
    {
      huffcode[p++] = code;
      code++;
    }

    code <<= 1;
    si++;
  }

  memset(hs->look_up, 0, sizeof(hs->look_up));
  memset(hs->tree, 0, sizeof(hs->tree));
  memset(hs->code_size, 0, sizeof(hs->code_size));

  nextfreeentry = -1;

  p = 0;

  while (p < lastp)
  {
    i = huff_val[index][p];
    code = huffcode[p];
    code_size = huffsize[p];

    hs->code_size[i] = code_size;

    if (code_size <= 8)
    {
      code <<= (8 - code_size);

      for (l = 1 << (8 - code_size); l > 0; l--)
      {
        hs->look_up[code] = i;
        code++;
      }
    }
    else
    {
      subtree = (code >> (code_size - 8)) & 0xFF;

      currententry = hs->look_up[subtree];

      if (currententry == 0)
      {
        hs->look_up[subtree] = currententry = nextfreeentry;

        nextfreeentry -= 2;
      }

      code <<= (16 - (code_size - 8));

      for (l = code_size; l > 9; l--)
      {
        if ((code & 0x8000) == 0)
          currententry--;

        if (hs->tree[-currententry - 1] == 0)
        {
          hs->tree[-currententry - 1] = nextfreeentry;

          currententry = nextfreeentry;

          nextfreeentry -= 2;
        }
        else
          currententry = hs->tree[-currententry - 1];

        code <<= 1;
      }

      if ((code & 0x8000) == 0)
        currententry--;

      hs->tree[-currententry - 1] = i;
    }

    p++;
  }
}
//------------------------------------------------------------------------------
// Verifies the quantization tables needed for this scan are available.
void jpeg_decoder::check_quant_tables(void)
{
  int i;

  for (i = 0; i < comps_in_scan; i++)
    if (quant[comp_quant[comp_list[i]]] == NULL)
      terminate(JPGD_UNDEFINED_QUANT_TABLE);
}
//------------------------------------------------------------------------------
// Verifies that all the Huffman tables needed for this scan are available.
void jpeg_decoder::check_huff_tables(void)
{
  int i;

  for (i = 0; i < comps_in_scan; i++)
  {
    if ((spectral_start == 0) && (huff_num[comp_dc_tab[comp_list[i]]] == NULL))
      terminate(JPGD_UNDEFINED_HUFF_TABLE);

    if ((spectral_end > 0) && (huff_num[comp_ac_tab[comp_list[i]]] == NULL))
      terminate(JPGD_UNDEFINED_HUFF_TABLE);
  }

  for (i = 0; i < JPGD_MAXHUFFTABLES; i++)
    if (huff_num[i])
    {
      if (!h[i])
        h[i] = (Phuff_tables_t)alloc(sizeof(huff_tables_t));

      make_huff_table(i, h[i]);
    }

  for (i = 0; i < blocks_per_mcu; i++)
  {
    dc_huff_seg[i] = h[comp_dc_tab[mcu_org[i]]];
    ac_huff_seg[i] = h[comp_ac_tab[mcu_org[i]]];
    component[i]   = &last_dc_val[mcu_org[i]];
  }
}
//------------------------------------------------------------------------------
// Determines the component order inside each MCU.
// Also calcs how many MCU's are on each row, etc.
void jpeg_decoder::calc_mcu_block_order(void)
{
  int component_num, component_id;
  int max_h_samp = 0, max_v_samp = 0;

  for (component_id = 0; component_id < comps_in_frame; component_id++)
  {
    if (comp_h_samp[component_id] > max_h_samp)
      max_h_samp = comp_h_samp[component_id];

    if (comp_v_samp[component_id] > max_v_samp)
      max_v_samp = comp_v_samp[component_id];
  }

  for (component_id = 0; component_id < comps_in_frame; component_id++)
  {
    comp_h_blocks[component_id] = ((((image_x_size * comp_h_samp[component_id]) + (max_h_samp - 1)) / max_h_samp) + 7) / 8;
    comp_v_blocks[component_id] = ((((image_y_size * comp_v_samp[component_id]) + (max_v_samp - 1)) / max_v_samp) + 7) / 8;
  }

  if (comps_in_scan == 1)
  {
    mcus_per_row = comp_h_blocks[comp_list[0]];
    mcus_per_col = comp_v_blocks[comp_list[0]];
  }
  else
  {
    mcus_per_row = (((image_x_size + 7) / 8) + (max_h_samp - 1)) / max_h_samp;
    mcus_per_col = (((image_y_size + 7) / 8) + (max_v_samp - 1)) / max_v_samp;
  }

  if (comps_in_scan == 1)
  {
    mcu_org[0] = comp_list[0];

    blocks_per_mcu = 1;
  }
  else
  {
    blocks_per_mcu = 0;

    for (component_num = 0; component_num < comps_in_scan; component_num++)
    {
      int num_blocks;

      component_id = comp_list[component_num];

      num_blocks = comp_h_samp[component_id] * comp_v_samp[component_id];

      while (num_blocks--)
        mcu_org[blocks_per_mcu++] = component_id;
    }
  }
}
//------------------------------------------------------------------------------
// Starts a new scan.
int jpeg_decoder::init_scan(void)
{
  if (!locate_sos_marker())
    return FALSE;

  calc_mcu_block_order();

  check_huff_tables();

  check_quant_tables();

  memset(last_dc_val, 0, comps_in_frame * sizeof(uint));

  eob_run = 0;

  if (restart_interval)
  {
    restarts_left = restart_interval;
    next_restart_num = 0;
  }

  if ((!progressive_flag) && (use_mmx))
    use_mmx_getbits = true;

  fix_in_buffer();

  return TRUE;
}
//------------------------------------------------------------------------------
// Starts a frame. Determines if the number of components or sampling factors
// are supported.
void jpeg_decoder::init_frame(void)
{
  int i;
  uchar *q;

  if (comps_in_frame == 1)
  {
    scan_type          = JPGD_GRAYSCALE;

    max_blocks_per_mcu = 1;

    max_mcu_x_size     = 8;
    max_mcu_y_size     = 8;
  }
  else if (comps_in_frame == 3)
  {
    if ( ((comp_h_samp[1] != 1) || (comp_v_samp[1] != 1)) ||
         ((comp_h_samp[2] != 1) || (comp_v_samp[2] != 1)) )
      terminate(JPGD_UNSUPPORTED_SAMP_FACTORS);

    if ((comp_h_samp[0] == 1) && (comp_v_samp[0] == 1))
    {
      scan_type          = JPGD_YH1V1;

      max_blocks_per_mcu = 3;

      max_mcu_x_size     = 8;
      max_mcu_y_size     = 8;
    }
    else if ((comp_h_samp[0] == 2) && (comp_v_samp[0] == 1))
    {
      scan_type          = JPGD_YH2V1;

      max_blocks_per_mcu = 4;

      max_mcu_x_size     = 16;
      max_mcu_y_size     = 8;
    }
    else if ((comp_h_samp[0] == 1) && (comp_v_samp[0] == 2))
    {
      scan_type          = JPGD_YH1V2;

      max_blocks_per_mcu = 4;

      max_mcu_x_size     = 8;
      max_mcu_y_size     = 16;
    }
    else if ((comp_h_samp[0] == 2) && (comp_v_samp[0] == 2))
    {
      scan_type          = JPGD_YH2V2;

      max_blocks_per_mcu = 6;

      max_mcu_x_size     = 16;
      max_mcu_y_size     = 16;
    }
    else
      terminate(JPGD_UNSUPPORTED_SAMP_FACTORS);
  }
  else
    terminate(JPGD_UNSUPPORTED_COLORSPACE);

  max_mcus_per_row = (image_x_size + (max_mcu_x_size - 1)) / max_mcu_x_size;
  max_mcus_per_col = (image_y_size + (max_mcu_y_size - 1)) / max_mcu_y_size;

  /* these values are for the *destination* pixels: after conversion */

  if (scan_type == JPGD_GRAYSCALE)
    dest_bytes_per_pixel = 1;
  else
    dest_bytes_per_pixel = 4;

  dest_bytes_per_scan_line = ((image_x_size + 15) & 0xFFF0) * dest_bytes_per_pixel;

  real_dest_bytes_per_scan_line = (image_x_size * dest_bytes_per_pixel);

  // Initialize two scan line buffers.
  // FIXME: Only the V2 sampling factors need two buffers.
  scan_line_0         = (uchar *)alloc(dest_bytes_per_scan_line + 8);
  memset(scan_line_0, 0, dest_bytes_per_scan_line);

  scan_line_1         = (uchar *)alloc(dest_bytes_per_scan_line + 8);
  memset(scan_line_1, 0, dest_bytes_per_scan_line);

  max_blocks_per_row = max_mcus_per_row * max_blocks_per_mcu;

  // Should never happen
  if (max_blocks_per_row > JPGD_MAXBLOCKSPERROW)
    terminate(JPGD_ASSERTION_ERROR);

  // Allocate the coefficient buffer, enough for one row's worth of MCU's
  q = (uchar *)alloc(max_blocks_per_row * 64 * sizeof(BLOCK_TYPE) + 8);

  // Align to 8-byte boundry, for MMX code
  q = (uchar *)(((uint)q + 7) & ~7);

  // The block_seg[] array's name dates back to the
  // 16-bit assembler implementation. "seg" stood for "segment".
  for (i = 0; i < max_blocks_per_row; i++)
    block_seg[i] = (BLOCK_TYPE *)(q + i * 64 * sizeof(BLOCK_TYPE));

  for (i = 0; i < max_blocks_per_row; i++)
    block_max_zag_set[i] = 64;

  Psample_buf = (uchar *)(((uint)alloc(max_blocks_per_row * 64 + 8) + 7) & ~7);

  total_lines_left = image_y_size;

  mcu_lines_left = 0;

  create_look_ups();
}
//------------------------------------------------------------------------------
// The following methods decode the various types of blocks encountered
// in progressively encoded images.
void progressive_block_decoder::decode_block_dc_first(
  jpeg_decoder *Pd,
  int component_id, int block_x, int block_y)
{
  int s, r;
  BLOCK_TYPE *p = Pd->coeff_buf_getp(Pd->dc_coeffs[component_id], block_x, block_y);

  if ((s = Pd->huff_decode(Pd->h[Pd->comp_dc_tab[component_id]])) != 0)
  {
    r = Pd->get_bits_2(s);
    s = HUFF_EXTEND_P(r, s);
  }

  Pd->last_dc_val[component_id] = (s += Pd->last_dc_val[component_id]);

  p[0] = s << Pd->successive_low;
}
//------------------------------------------------------------------------------
void progressive_block_decoder::decode_block_dc_refine(
  jpeg_decoder *Pd,
  int component_id, int block_x, int block_y)
{
  if (Pd->get_bits_2(1))
  {
    BLOCK_TYPE *p = Pd->coeff_buf_getp(Pd->dc_coeffs[component_id], block_x, block_y);

    p[0] |= (1 << Pd->successive_low);
  }
}
//------------------------------------------------------------------------------
void progressive_block_decoder::decode_block_ac_first(
  jpeg_decoder *Pd,
  int component_id, int block_x, int block_y)
{
  int k, s, r;

  if (Pd->eob_run)
  {
    Pd->eob_run--;
    return;
  }

  BLOCK_TYPE *p = Pd->coeff_buf_getp(Pd->ac_coeffs[component_id], block_x, block_y);

  for (k = Pd->spectral_start; k <= Pd->spectral_end; k++)
  {
    s = Pd->huff_decode(Pd->h[Pd->comp_ac_tab[component_id]]);

    r = s >> 4;
    s &= 15;

    if (s)
    {
      if ((k += r) > 63)
        Pd->terminate(JPGD_DECODE_ERROR);

      r = Pd->get_bits_2(s);
      s = HUFF_EXTEND_P(r, s);

      p[ZAG[k]] = s << Pd->successive_low;
    }
    else
    {
      if (r == 15)
      {
        if ((k += 15) > 63)
          Pd->terminate(JPGD_DECODE_ERROR);
      }
      else
      {
        Pd->eob_run = 1 << r;

        if (r)
          Pd->eob_run += Pd->get_bits_2(r);

        Pd->eob_run--;

        break;
      }
    }
  }
}
//------------------------------------------------------------------------------
void progressive_block_decoder::decode_block_ac_refine(
  jpeg_decoder *Pd,
  int component_id, int block_x, int block_y)
{
  int s, k, r;
  int p1 = 1 << Pd->successive_low;
  int m1 = (-1) << Pd->successive_low;
  BLOCK_TYPE *p = Pd->coeff_buf_getp(Pd->ac_coeffs[component_id], block_x, block_y);

  k = Pd->spectral_start;

  if (Pd->eob_run == 0)
  {
    for ( ; k <= Pd->spectral_end; k++)
    {
      s = Pd->huff_decode(Pd->h[Pd->comp_ac_tab[component_id]]);

      r = s >> 4;
      s &= 15;

      if (s)
      {
        if (s != 1)
          Pd->terminate(JPGD_DECODE_ERROR);

        if (Pd->get_bits_2(1))
          s = p1;
        else
          s = m1;
      }
      else
      {
        if (r != 15)
        {
          Pd->eob_run = 1 << r;

          if (r)
            Pd->eob_run += Pd->get_bits_2(r);

          break;
        }
      }

      do
      {
        BLOCK_TYPE *this_coef = p + ZAG[k];

        if (*this_coef != 0)
        {
          if (Pd->get_bits_2(1))
          {
            if ((*this_coef & p1) == 0)
            {
              if (*this_coef >= 0)
                *this_coef += p1;
              else
                *this_coef += m1;
            }
          }
        }
        else
        {
          if (--r < 0)
            break;
        }

        k++;

      } while (k <= Pd->spectral_end);

      if ((s) && (k < 64))
      {
        p[ZAG[k]] = s;
      }
    }
  }

  if (Pd->eob_run > 0)
  {
    for ( ; k <= Pd->spectral_end; k++)
    {
      BLOCK_TYPE *this_coef = p + ZAG[k];

      if (*this_coef != 0)
      {
        if (Pd->get_bits_2(1))
        {
          if ((*this_coef & p1) == 0)
          {
            if (*this_coef >= 0)
              *this_coef += p1;
            else
              *this_coef += m1;
          }
        }
      }
    }

    Pd->eob_run--;
  }
}
//------------------------------------------------------------------------------
// Decode a scan in a progressively encoded image.
void jpeg_decoder::decode_scan(
  Pdecode_block_func decode_block_func)
{
  int mcu_row, mcu_col, mcu_block;
  int block_x_mcu[JPGD_MAXCOMPONENTS], block_y_mcu[JPGD_MAXCOMPONENTS];

  memset(block_y_mcu, 0, sizeof(block_y_mcu));

  for (mcu_col = 0; mcu_col < mcus_per_col; mcu_col++)
  {
    int component_num, component_id;

    memset(block_x_mcu, 0, sizeof(block_x_mcu));

    for (mcu_row = 0; mcu_row < mcus_per_row; mcu_row++)
    {
      int block_x_mcu_ofs = 0, block_y_mcu_ofs = 0;

      if ((restart_interval) && (restarts_left == 0))
        process_restart();

      for (mcu_block = 0; mcu_block < blocks_per_mcu; mcu_block++)
      {
        component_id = mcu_org[mcu_block];

        decode_block_func(this, component_id,
                          block_x_mcu[component_id] + block_x_mcu_ofs,
                          block_y_mcu[component_id] + block_y_mcu_ofs);

        if (comps_in_scan == 1)
          block_x_mcu[component_id]++;
        else
        {
          if (++block_x_mcu_ofs == comp_h_samp[component_id])
          {
            block_x_mcu_ofs = 0;

            if (++block_y_mcu_ofs == comp_v_samp[component_id])
            {
              block_y_mcu_ofs = 0;

              block_x_mcu[component_id] += comp_h_samp[component_id];
            }
          }
        }
      }

      restarts_left--;
    }

    if (comps_in_scan == 1)
      block_y_mcu[comp_list[0]]++;
    else
    {
      for (component_num = 0; component_num < comps_in_scan; component_num++)
      {
        component_id = comp_list[component_num];

        block_y_mcu[component_id] += comp_v_samp[component_id];
      }
    }
  }
}
//------------------------------------------------------------------------------
// Decode a progressively encoded image.
void jpeg_decoder::init_progressive(void)
{
  int i;

  if (comps_in_frame == 4)
    terminate(JPGD_UNSUPPORTED_COLORSPACE);

  // Allocate the coefficient buffers.
  for (i = 0; i < comps_in_frame; i++)
  {
    dc_coeffs[i] = coeff_buf_open(max_mcus_per_row * comp_h_samp[i],
                                  max_mcus_per_col * comp_v_samp[i], 1, 1);
    ac_coeffs[i] = coeff_buf_open(max_mcus_per_row * comp_h_samp[i],
                                  max_mcus_per_col * comp_v_samp[i], 8, 8);
  }

  for ( ; ; )
  {
    int dc_only_scan, refinement_scan;
    Pdecode_block_func decode_block_func;

    if (!init_scan())
      break;

    dc_only_scan    = (spectral_start == 0);
    refinement_scan = (successive_high != 0);

    if ((spectral_start > spectral_end) || (spectral_end > 63))
      terminate(JPGD_BAD_SOS_SPECTRAL);

    if (dc_only_scan)
    {
      if (spectral_end)
        terminate(JPGD_BAD_SOS_SPECTRAL);
    }
    else if (comps_in_scan != 1)  /* AC scans can only contain one component */
      terminate(JPGD_BAD_SOS_SPECTRAL);

    if ((refinement_scan) && (successive_low != successive_high - 1))
      terminate(JPGD_BAD_SOS_SUCCESSIVE);

    if (dc_only_scan)
    {
      if (refinement_scan)
        decode_block_func = progressive_block_decoder::decode_block_dc_refine;
      else
        decode_block_func = progressive_block_decoder::decode_block_dc_first;
    }
    else
    {
      if (refinement_scan)
        decode_block_func = progressive_block_decoder::decode_block_ac_refine;
      else
        decode_block_func = progressive_block_decoder::decode_block_ac_first;
    }

    decode_scan(decode_block_func);

    //get_bits_2(bits_left & 7);

    bits_left = 16;
    //bit_buf = 0;
    get_bits_1(16);
    get_bits_1(16);
  }

  comps_in_scan = comps_in_frame;

  for (i = 0; i < comps_in_frame; i++)
    comp_list[i] = i;

  calc_mcu_block_order();
}
//------------------------------------------------------------------------------
void jpeg_decoder::init_sequential(void)
{
  if (!init_scan())
    terminate(JPGD_UNEXPECTED_MARKER);
}
//------------------------------------------------------------------------------
void jpeg_decoder::decode_start(void)
{
  init_frame();

  if (progressive_flag)
    init_progressive();
  else
    init_sequential();
}
//------------------------------------------------------------------------------
// Find the start of the JPEG file and reads enough data to determine
// its size, number of components, etc.
void jpeg_decoder::decode_init(Pjpeg_decoder_stream Pstream, bool use_mmx)
{
  init(Pstream, use_mmx);

  locate_sof_marker();
}
//------------------------------------------------------------------------------
// Call get_error_code() after constructing to determine if the stream
// was valid or not. You may call the get_width(), get_height(), etc.
// methods after the constructor is called.
// You may then either destruct the object, or begin decoding the image
// by calling begin(), then decode().
jpeg_decoder::jpeg_decoder(Pjpeg_decoder_stream Pstream, bool use_mmx)
{
  if (setjmp(jmp_state))
    return;

  decode_init(Pstream, use_mmx);
}
//------------------------------------------------------------------------------
// If you wish to decompress the image, call this method after constructing
// the object. If JPGD_OKAY is returned you may then call decode() to
// fetch the scan lines.
int jpeg_decoder::begin(void)
{
  if (ready_flag)
    return (JPGD_OKAY);

  if (error_code)
    return (JPGD_FAILED);

  if (setjmp(jmp_state))
    return (JPGD_FAILED);

  decode_start();

  ready_flag = true;

  return (JPGD_OKAY);
}
//------------------------------------------------------------------------------
// Completely destroys the decoder object. May be called at any time.
jpeg_decoder::~jpeg_decoder()
{
  free_all_blocks();
}
//------------------------------------------------------------------------------

