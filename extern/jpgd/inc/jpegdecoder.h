//------------------------------------------------------------------------------
// jpegdecoder.h
// Small JPEG Decoder Library v0.93b
// Last updated: Dec. 28, 2001 
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
#ifndef JPEG_DECODER_H
#define JPEG_DECODER_H
//------------------------------------------------------------------------------
#include "main.h"
//------------------------------------------------------------------------------
// Define SUPPORT_X86ASM to include the inline x86 assembler code.
#define SUPPORT_X86ASM
//------------------------------------------------------------------------------
// Define SUPPORT_MMX to include MMX support.
#define SUPPORT_MMX
//------------------------------------------------------------------------------
#define JPGD_INBUFSIZE       4096
//------------------------------------------------------------------------------
// May need to be adjusted if support for other colorspaces/sampling factors is added
#define JPGD_MAXBLOCKSPERMCU 10
//------------------------------------------------------------------------------
#define JPGD_MAXHUFFTABLES   8
#define JPGD_MAXQUANTTABLES  4
#define JPGD_MAXCOMPONENTS   4
#define JPGD_MAXCOMPSINSCAN  4
//------------------------------------------------------------------------------
// Increase this if you increase the max width!
#define JPGD_MAXBLOCKSPERROW 6144
//------------------------------------------------------------------------------
// Max. allocated blocks
#define JPGD_MAXBLOCKS    100
//------------------------------------------------------------------------------
#define JPGD_MAX_HEIGHT 8192
#define JPGD_MAX_WIDTH  8192
//------------------------------------------------------------------------------
/* JPEG specific errors */
#define JPGD_BAD_DHT_COUNTS              -200
#define JPGD_BAD_DHT_INDEX               -201
#define JPGD_BAD_DHT_MARKER              -202
#define JPGD_BAD_DQT_MARKER              -203
#define JPGD_BAD_DQT_TABLE               -204
#define JPGD_BAD_PRECISION               -205
#define JPGD_BAD_HEIGHT                  -206
#define JPGD_BAD_WIDTH                   -207
#define JPGD_TOO_MANY_COMPONENTS         -208
#define JPGD_BAD_SOF_LENGTH              -209
#define JPGD_BAD_VARIABLE_MARKER         -210
#define JPGD_BAD_DRI_LENGTH              -211
#define JPGD_BAD_SOS_LENGTH              -212
#define JPGD_BAD_SOS_COMP_ID             -213
#define JPGD_W_EXTRA_BYTES_BEFORE_MARKER -214
#define JPGD_NO_ARITHMITIC_SUPPORT       -215
#define JPGD_UNEXPECTED_MARKER           -216
#define JPGD_NOT_JPEG                    -217
#define JPGD_UNSUPPORTED_MARKER          -218
#define JPGD_BAD_DQT_LENGTH              -219
#define JPGD_TOO_MANY_BLOCKS             -221
#define JPGD_UNDEFINED_QUANT_TABLE       -222
#define JPGD_UNDEFINED_HUFF_TABLE        -223
#define JPGD_NOT_SINGLE_SCAN             -224
#define JPGD_UNSUPPORTED_COLORSPACE      -225
#define JPGD_UNSUPPORTED_SAMP_FACTORS    -226
#define JPGD_DECODE_ERROR                -227
#define JPGD_BAD_RESTART_MARKER          -228
#define JPGD_ASSERTION_ERROR             -229
#define JPGD_BAD_SOS_SPECTRAL            -230
#define JPGD_BAD_SOS_SUCCESSIVE          -231
#define JPGD_STREAM_READ                 -232
#define JPGD_NOTENOUGHMEM                -233
//------------------------------------------------------------------------------
#define JPGD_GRAYSCALE 0
#define JPGD_YH1V1     1
#define JPGD_YH2V1     2
#define JPGD_YH1V2     3
#define JPGD_YH2V2     4
//------------------------------------------------------------------------------
const int JPGD_FAILED = -1;
const int JPGD_DONE = 1;
const int JPGD_OKAY = 0;
//------------------------------------------------------------------------------
typedef enum
{
  M_SOF0  = 0xC0,
  M_SOF1  = 0xC1,
  M_SOF2  = 0xC2,
  M_SOF3  = 0xC3,

  M_SOF5  = 0xC5,
  M_SOF6  = 0xC6,
  M_SOF7  = 0xC7,

  M_JPG   = 0xC8,
  M_SOF9  = 0xC9,
  M_SOF10 = 0xCA,
  M_SOF11 = 0xCB,

  M_SOF13 = 0xCD,
  M_SOF14 = 0xCE,
  M_SOF15 = 0xCF,

  M_DHT   = 0xC4,

  M_DAC   = 0xCC,

  M_RST0  = 0xD0,
  M_RST1  = 0xD1,
  M_RST2  = 0xD2,
  M_RST3  = 0xD3,
  M_RST4  = 0xD4,
  M_RST5  = 0xD5,
  M_RST6  = 0xD6,
  M_RST7  = 0xD7,

  M_SOI   = 0xD8,
  M_EOI   = 0xD9,
  M_SOS   = 0xDA,
  M_DQT   = 0xDB,
  M_DNL   = 0xDC,
  M_DRI   = 0xDD,
  M_DHP   = 0xDE,
  M_EXP   = 0xDF,

  M_APP0  = 0xE0,
  M_APP15 = 0xEF,

  M_JPG0  = 0xF0,
  M_JPG13 = 0xFD,
  M_COM   = 0xFE,

  M_TEM   = 0x01,

  M_ERROR = 0x100
} JPEG_MARKER;
//------------------------------------------------------------------------------
#define RST0 0xD0
//------------------------------------------------------------------------------
typedef struct huff_tables_tag
{
  uint  look_up[256];
  uchar code_size[256];
  // FIXME: Is 512 tree entries really enough to handle _all_ possible
  // code sets? I think so but not 100% positive.
  uint  tree[512];
} huff_tables_t, *Phuff_tables_t;
//------------------------------------------------------------------------------
typedef struct coeff_buf_tag
{
  uchar *Pdata;

  int block_num_x, block_num_y;
  int block_len_x, block_len_y;

  int block_size;

} coeff_buf_t, *Pcoeff_buf_t;
//------------------------------------------------------------------------------
class jpeg_decoder;
typedef void (*Pdecode_block_func)(jpeg_decoder *, int, int, int);
//------------------------------------------------------------------------------
class progressive_block_decoder
{
public:
  static void decode_block_dc_first(
    jpeg_decoder *Pd,
    int component_id, int block_x, int block_y);
  static void decode_block_dc_refine(
    jpeg_decoder *Pd,
    int component_id, int block_x, int block_y);
  static void decode_block_ac_first(
    jpeg_decoder *Pd,
    int component_id, int block_x, int block_y);
  static void decode_block_ac_refine(
    jpeg_decoder *Pd,
    int component_id, int block_x, int block_y);
};
//------------------------------------------------------------------------------
// Input stream interface.
// Derive from this class to fetch input data from sources other than
// files. An important requirement is that you *must* set eof_flag to true
// when no more data is available to fetch!
// The decoder is rather "greedy": it will keep on calling this method until
// its internal input buffer is full, or until the EOF flag is set.
// It the input stream contains data after the JPEG stream's EOI (end of
// image) marker it will probably be pulled into the internal buffer.
// Call the get_total_bytes_read() method to determine the true
// size of the JPEG stream.
class jpeg_decoder_stream
{
public:

  jpeg_decoder_stream()
  {
  }

  virtual ~jpeg_decoder_stream()
  {
  }

  // The read() method is called when the internal input buffer is empty.
  // Pbuf - input buffer
  // max_bytes_to_read - maximum bytes that can be written to Pbuf
  // Peof_flag - set this to true if at end of stream (no more bytes remaining)
  // Return -1 on error, otherwise return the number of bytes actually
  // written to the buffer (which may be 0).
  // Notes: This method will be called in a loop until you set *Peof_flag to
  // true or the internal buffer is full.
  // The MMX state will be automatically saved/restored before this method is
  // called, unlike previous versions.
  virtual int read(uchar *Pbuf, int max_bytes_to_read, bool *Peof_flag) = 0;

  virtual void attach(void)
  {
  }

  virtual void detach(void)
  {
  }
};
//------------------------------------------------------------------------------
typedef jpeg_decoder_stream *Pjpeg_decoder_stream;
//------------------------------------------------------------------------------
// Here's an example FILE stream class.
class jpeg_decoder_file_stream : public jpeg_decoder_stream
{
  FILE *Pfile;
  bool eof_flag, error_flag;

public:

  jpeg_decoder_file_stream()
  {
    Pfile = NULL;
    eof_flag = false;
    error_flag = false;
  }

  void close(void)
  {
    if (Pfile)
    {
      fclose(Pfile);
      Pfile = NULL;
    }

    eof_flag = false;
    error_flag = false;
  }

  virtual ~jpeg_decoder_file_stream()
  {
    close();
  }

  bool open(const char *Pfilename)
  {
    close();

    eof_flag = false;
    error_flag = false;

    Pfile = fopen(Pfilename, "rb");
    if (!Pfile)
      return (true);

    return (false);
  }

  virtual int read(uchar *Pbuf, int max_bytes_to_read, bool *Peof_flag)
  {
#if 0
// Empty/clear MMX state: For testing purposes only!
#ifdef _DEBUG
#ifdef SUPPORT_MMX
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
#endif
#endif
    if (!Pfile)
      return (-1);

    if (eof_flag)
    {
      *Peof_flag = true;
      return (0);
    }

    if (error_flag)
      return (-1);

    int bytes_read = fread(Pbuf, 1, max_bytes_to_read, Pfile);

    if (bytes_read < max_bytes_to_read)
    {
      if (ferror(Pfile))
      {
        error_flag = true;
        return (-1);
      }

      eof_flag = true;
      *Peof_flag = true;
    }

    return (bytes_read);
  }

  bool get_error_status(void)
  {
    return (error_flag);
  }

  bool reset(void)
  {
    if (error_flag)
      return (true);

    fseek(Pfile, 0, SEEK_SET);

    eof_flag = false;

    return (false);
  }

  int get_size(void)
  {
    if (!Pfile)
      return (-1);

    int loc = ftell(Pfile);

    fseek(Pfile, 0, SEEK_END);

    int size = ftell(Pfile);

    fseek(Pfile, loc, SEEK_SET);

    return (size);
  }
};
//------------------------------------------------------------------------------
typedef jpeg_decoder_file_stream *Pjpeg_decoder_file_stream;
//------------------------------------------------------------------------------
#define QUANT_TYPE int16
#define BLOCK_TYPE int16
//------------------------------------------------------------------------------
// Disable no return value warning, for rol() method
#pragma warning(push)
#pragma warning( disable : 4035 4799 )
//------------------------------------------------------------------------------
class jpeg_decoder
{
  friend class progressive_block_decoder;

private:

  void free_all_blocks(void);

  void terminate(int status);

  void *alloc(int n);

  void word_clear(void *p, ushort c, uint n);

  void prep_in_buffer(void);

  void read_dht_marker(void);

  void read_dqt_marker(void);

  void read_sof_marker(void);

  void skip_variable_marker(void);

  void read_dri_marker(void);

  void read_sos_marker(void);

  int next_marker(void);

  int process_markers(void);

  void locate_soi_marker(void);

  void locate_sof_marker(void);

  int locate_sos_marker(void);

  void init(Pjpeg_decoder_stream Pstream, bool use_mmx);

  void create_look_ups(void);

  void fix_in_buffer(void);

  void transform_row(void);

  Pcoeff_buf_t coeff_buf_open(
    int block_num_x, int block_num_y,
    int block_len_x, int block_len_y);

  void coeff_buf_read(
    Pcoeff_buf_t cb,
    int block_x, int block_y,
    BLOCK_TYPE *buffer);

  void coeff_buf_write(
    Pcoeff_buf_t cb,
    int block_x, int block_y,
    BLOCK_TYPE *buffer);

  BLOCK_TYPE *coeff_buf_getp(
    Pcoeff_buf_t cb,
    int block_x, int block_y);

  void load_next_row(void);

  void decode_next_row(void);
#ifdef SUPPORT_MMX
  void decode_next_row_mmx(void);
#endif

  void make_huff_table(
    int index,
    Phuff_tables_t hs);

  void check_quant_tables(void);

  void check_huff_tables(void);

  void calc_mcu_block_order(void);

  int init_scan(void);

  void init_frame(void);

  void process_restart(void);

  void decode_scan(
    Pdecode_block_func decode_block_func);

  void init_progressive(void);

  void init_sequential(void);

  void decode_start(void);

  void decode_init(Pjpeg_decoder_stream Pstream, bool use_mmx);

  void H2V2Convert(void);
  void H2V1Convert(void);
  void H1V2Convert(void);
  void H1V1Convert(void);
  void GrayConvert(void);

  void find_eoi(void);
//------------------
  inline uint jpeg_decoder::rol(uint i, uchar j);
  inline uint jpeg_decoder::get_char(void);
  inline uint jpeg_decoder::get_char(bool *Ppadding_flag);
  inline void jpeg_decoder::stuff_char(uchar q);
  inline uchar jpeg_decoder::get_octet(void);
  inline uint jpeg_decoder::get_bits_1(int num_bits);
  inline uint jpeg_decoder::get_bits_2(int numbits);
  inline int jpeg_decoder::huff_decode(Phuff_tables_t Ph);
#ifdef SUPPORT_X86ASM
  inline uint jpeg_decoder::huff_extend(uint i, int c);
#endif
  inline uchar jpeg_decoder::clamp(int i);

#ifdef SUPPORT_MMX
  inline uint jpeg_decoder::get_high_byte_mmx(void);
  inline uint jpeg_decoder::get_high_word_mmx(void);
  inline void jpeg_decoder::get_bits_2_mmx_init(void);
  inline void jpeg_decoder::get_bits_2_mmx_deinit(void);
  inline uint jpeg_decoder::get_bits_2_mmx(int numbits);
  inline int jpeg_decoder::huff_decode_mmx(Phuff_tables_t Ph);
#endif
//------------------
  int   image_x_size;
  int   image_y_size;

  Pjpeg_decoder_stream Pstream;

  int   progressive_flag;

  uchar *huff_num[JPGD_MAXHUFFTABLES];  /* pointer to number of Huffman codes per bit size */
  uchar *huff_val[JPGD_MAXHUFFTABLES];  /* pointer to Huffman codes per bit size */

  QUANT_TYPE *quant[JPGD_MAXQUANTTABLES];    /* pointer to quantization tables */

  int   scan_type;                      /* Grey, Yh1v1, Yh1v2, Yh2v1, Yh2v2,
                                           CMYK111, CMYK4114 */

  int   comps_in_frame;                 /* # of components in frame */
  int   comp_h_samp[JPGD_MAXCOMPONENTS];     /* component's horizontal sampling factor */
  int   comp_v_samp[JPGD_MAXCOMPONENTS];     /* component's vertical sampling factor */
  int   comp_quant[JPGD_MAXCOMPONENTS];      /* component's quantization table selector */
  int   comp_ident[JPGD_MAXCOMPONENTS];      /* component's ID */

  int   comp_h_blocks[JPGD_MAXCOMPONENTS];
  int   comp_v_blocks[JPGD_MAXCOMPONENTS];

  int   comps_in_scan;                  /* # of components in scan */
  int   comp_list[JPGD_MAXCOMPSINSCAN];      /* components in this scan */
  int   comp_dc_tab[JPGD_MAXCOMPONENTS];     /* component's DC Huffman coding table selector */
  int   comp_ac_tab[JPGD_MAXCOMPONENTS];     /* component's AC Huffman coding table selector */

  int   spectral_start;                 /* spectral selection start */
  int   spectral_end;                   /* spectral selection end   */
  int   successive_low;                 /* successive approximation low */
  int   successive_high;                /* successive approximation high */

  int   max_mcu_x_size;                 /* MCU's max. X size in pixels */
  int   max_mcu_y_size;                 /* MCU's max. Y size in pixels */

  int   blocks_per_mcu;
  int   max_blocks_per_row;
  int   mcus_per_row, mcus_per_col;

  int   mcu_org[JPGD_MAXBLOCKSPERMCU];

  int   total_lines_left;               /* total # lines left in image */
  int   mcu_lines_left;                 /* total # lines left in this MCU */

  int   real_dest_bytes_per_scan_line;
  int   dest_bytes_per_scan_line;        /* rounded up */
  int   dest_bytes_per_pixel;            /* currently, 4 (RGB) or 1 (Y) */

  void  *blocks[JPGD_MAXBLOCKS];         /* list of all dynamically allocated blocks */

  Phuff_tables_t h[JPGD_MAXHUFFTABLES];

  Pcoeff_buf_t dc_coeffs[JPGD_MAXCOMPONENTS];
  Pcoeff_buf_t ac_coeffs[JPGD_MAXCOMPONENTS];

  int eob_run;

  int block_y_mcu[JPGD_MAXCOMPONENTS];

  uchar *Pin_buf_ofs;
  int in_buf_left;
  int tem_flag;
  bool eof_flag;

  uchar padd_1[128];
  uchar in_buf[JPGD_INBUFSIZE + 128];
  uchar padd_2[128];

  int   bits_left;
  union
  {
    uint bit_buf;
    uint bit_buf_64[2];
  };
  
  uint  saved_mm1[2];

  bool  use_mmx_getbits;

  int   restart_interval;
  int   restarts_left;
  int   next_restart_num;

  int   max_mcus_per_row;
  int   max_blocks_per_mcu;

  int   max_mcus_per_col;

  uint *component[JPGD_MAXBLOCKSPERMCU];   /* points into the lastdcvals table */
  uint  last_dc_val[JPGD_MAXCOMPONENTS];

  Phuff_tables_t dc_huff_seg[JPGD_MAXBLOCKSPERMCU];
  Phuff_tables_t ac_huff_seg[JPGD_MAXBLOCKSPERMCU];

  BLOCK_TYPE *block_seg[JPGD_MAXBLOCKSPERROW];
  int   block_max_zag_set[JPGD_MAXBLOCKSPERROW];

  uchar *Psample_buf;
  //int   block_num[JPGD_MAXBLOCKSPERROW];

  int   crr[256];
  int   cbb[256];
  int   padd;
  long  crg[256];
  long  cbg[256];

  uchar *scan_line_0;
  uchar *scan_line_1;

  BLOCK_TYPE temp_block[64];

  bool use_mmx;
  bool use_mmx_idct;
  bool mmx_active;

  int error_code;
  bool ready_flag;

  jmp_buf jmp_state;

  int total_bytes_read;

public:

  // If SUPPORT_MMX is not defined, the use_mmx flag is ignored.
  jpeg_decoder(Pjpeg_decoder_stream Pstream,
               bool use_mmx);

  int begin(void);

  int decode(void * *Pscan_line_ofs, uint *Pscan_line_len);

  ~jpeg_decoder();

  int get_error_code(void)
  {
    return (error_code);
  }

  int get_width(void)
  {
    return (image_x_size);
  }

  int get_height(void)
  {
    return (image_y_size);
  }

  int get_num_components(void)
  {
    return (comps_in_frame);
  }

  int get_bytes_per_pixel(void)
  {
    return (dest_bytes_per_pixel);
  }

  int get_bytes_per_scan_line(void)
  {
    return (image_x_size * get_bytes_per_pixel());
  }

  int get_total_bytes_read(void)
  {
    return (total_bytes_read);
  }
};
//------------------------------------------------------------------------------
#include "jpegdecoder.inl"
//------------------------------------------------------------------------------
#pragma warning(pop)
//------------------------------------------------------------------------------
typedef jpeg_decoder *Pjpeg_decoder;
//------------------------------------------------------------------------------
// idct.cpp
void idct(BLOCK_TYPE *data, uchar *Pdst_ptr);
//------------------------------------------------------------------------------
// fidctfst.cpp
void jpeg_idct_ifast (
  BLOCK_TYPE* inptr,
  short *quantptr,
  uchar * *outptr,
  int output_col);

void jpeg_idct_ifast_deinit(void);

bool jpeg_idct_ifast_avail(void);
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------

