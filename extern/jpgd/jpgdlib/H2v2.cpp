//------------------------------------------------------------------------------
// h2v2.cpp
// Upsampling/colorspace conversion (H2V2, YCbCr)
// Last updated: Nov. 16, 2000 
// Copyright (C) 1994-2000 Rich Geldreich
// richgel@voicenet.com
//
// This is a popular case, so it's worth seperating out and optimizing a bit.
// If you compile this module with the Intel Compiler, the MMX version will
// automatically be compiled in.
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
#ifdef __ICL
#include "mmintrin.h"
#endif
//------------------------------------------------------------------------------
#define FIX(x, b) ((long) ((x) * (1L<<(b)) + 0.5))
//------------------------------------------------------------------------------
// YCbCr H2V2 (2x2:1:1, 6 blocks per MCU) to 24-bit RGB
// This case is very popular, so it's important that it's fast.
// If this module is compiled with the Intel Compiler the faster
// MMX specific version will also be available.
// FIXME: Create all-asm version, so Intel Compiler isn't needed.
void jpeg_decoder::H2V2Convert(void)
{
  int row = max_mcu_y_size - mcu_lines_left;
  uchar *d0 = scan_line_0;
  uchar *d1 = scan_line_1;
  uchar *y;
  uchar *c;
#ifdef __ICL
  // Included here so ICC includes symbol info for these variables
  static bool init_flag = false;
  static __m64 CRR_MUL;
  static __m64 CRR_SUB;
  static __m64 CBB_MUL;
  static __m64 CBB_SUB;
  static __m64 CRG_MUL;
  static __m64 CBG_MUL;
  static __m64 GC_SUB;
  //__m64 rc, gc, bc;
  __m64 m0,m1,m2,m3,m4,m5;
  __m64 cr, cb, yy, y0, y1, r, g, b, r0, r1, g0, g1, b0, b1;
  __m64 rc0, rc1, gc0, gc1, bc0, bc1;
  static __m64 zero = 0;
#endif

  if (row < 8)
    y = Psample_buf + row * 8;
  else
    y = Psample_buf + 64*2 + (row & 7) * 8;

  c = Psample_buf + 64*4 + (row >> 1) * 8;

#ifdef __ICL
  if (use_mmx)
  {
    if (!init_flag)
    {
      init_flag = true;

      // FIXME: Hardcode these values
      CRR_MUL = _m_from_int(FIX(1.402/4, 16));
      CRR_SUB = _m_from_int((int)(127.5*1.402 + .5));

      CBB_MUL = _m_from_int(FIX(1.772/4, 16));
      CBB_SUB = _m_from_int((int)(127.5*1.772 + .5));

      CRG_MUL = _m_from_int(FIX(-0.71414/4, 16));
      CBG_MUL = _m_from_int(FIX(-0.34414/4, 16));
      GC_SUB = _m_from_int((int)(127.5*(-0.71414) + 127.5*(-0.34414) + .5));

      CRR_MUL = _m_punpcklwd(CRR_MUL, CRR_MUL);
      CRR_MUL = _m_punpckldq(CRR_MUL, CRR_MUL);

      CRR_SUB = _m_punpcklwd(CRR_SUB, CRR_SUB);
      CRR_SUB = _m_punpckldq(CRR_SUB, CRR_SUB);

      CBB_MUL = _m_punpcklwd(CBB_MUL, CBB_MUL);
      CBB_MUL = _m_punpckldq(CBB_MUL, CBB_MUL);

      CBB_SUB = _m_punpcklwd(CBB_SUB, CBB_SUB);
      CBB_SUB = _m_punpckldq(CBB_SUB, CBB_SUB);

      CRG_MUL = _m_punpcklwd(CRG_MUL, CRG_MUL);
      CRG_MUL = _m_punpckldq(CRG_MUL, CRG_MUL);

      CBG_MUL = _m_punpcklwd(CBG_MUL, CBG_MUL);
      CBG_MUL = _m_punpckldq(CBG_MUL, CBG_MUL);

      GC_SUB = _m_punpcklwd(GC_SUB, GC_SUB);
      GC_SUB = _m_punpckldq(GC_SUB, GC_SUB);
    }

    // Mind-bending MMX intrinsics follow...
    for (int i = max_mcus_per_row; i > 0; i--)
    {
      for (int l = 0; l < 2; l++)
      {
        m2 = _m_from_int(*(int *)(&c[0]));
        m3 = _m_from_int(*(int *)(&c[64]));

        cb = m2;
        cb = _m_punpcklbw(cb, cb);
        cb = _m_psllw(_m_punpcklbw(cb, zero), 2);

        cr = m3;
        cr = _m_punpcklbw(cr, cr);
        cr = _m_psllw(_m_punpcklbw(cr, zero), 2);

        rc0 = _m_pmulhw(cr, CRR_MUL);
        rc0 = _m_psubw(rc0, CRR_SUB);

        bc0 = _m_pmulhw(cb, CBB_MUL);
        bc0 = _m_psubw(bc0, CBB_SUB);

        gc0 = _m_pmulhw(cr, CRG_MUL);
        m0 = _m_pmulhw(cb, CBG_MUL);
        gc0 = _m_paddw(gc0, m0);
        gc0 = _m_psubw(gc0, GC_SUB);
        //-------
        cb = _m_psrlqi(m2, 16);
        cb = _m_punpcklbw(cb, cb);
        cb = _m_psllw(_m_punpcklbw(cb, zero), 2);

        cr = _m_psrlqi(m3, 16);
        cr = _m_punpcklbw(cr, cr);
        cr = _m_psllw(_m_punpcklbw(cr, zero), 2);

        rc1 = _m_pmulhw(cr, CRR_MUL);
        rc1 = _m_psubw(rc1, CRR_SUB);

        bc1 = _m_pmulhw(cb, CBB_MUL);
        bc1 = _m_psubw(bc1, CBB_SUB);

        gc1 = _m_pmulhw(cr, CRG_MUL);
        m0 = _m_pmulhw(cb, CBG_MUL);
        gc1 = _m_paddw(gc1, m0);
        gc1 = _m_psubw(gc1, GC_SUB);
        //------------
        yy = *(__m64 *)y;
        y0 = _m_punpcklbw(yy, zero);
        y1 = _m_punpcklbw(_m_psrlq(yy, 32), zero);

        r0 = _m_paddsw(y0, rc0);
        r1 = _m_paddsw(y1, rc1);
        g0 = _m_paddsw(y0, gc0);
        g1 = _m_paddsw(y1, gc1);
        b0 = _m_paddsw(y0, bc0);
        b1 = _m_paddsw(y1, bc1);

        r = _m_packuswb(r0, r1);
        g = _m_packuswb(g0, g1);
        b = _m_packuswb(b0, b1);
        //------------
        m0 = _m_punpcklbw(r, zero);
        m0 = _m_punpcklwd(m0, zero);

        m1 = _m_punpcklbw(g, zero);
        m1 = _m_punpcklwd(m1, zero);
        m1 = _m_psllqi(m1, 8);

        m2 = _m_punpcklbw(b, zero);
        m2 = _m_punpcklwd(m2, zero);
        m2 = _m_psllqi(m2, 16);

        *(__m64 *)d0 = _m_por(_m_por(m0, m1), m2);
        //------------
        m0 = _m_punpcklbw(r, zero);
        m0 = _m_punpckhwd(m0, zero);

        m1 = _m_punpcklbw(g, zero);
        m1 = _m_punpckhwd(m1, zero);
        m1 = _m_psllqi(m1, 8);

        m2 = _m_punpcklbw(b, zero);
        m2 = _m_punpckhwd(m2, zero);
        m2 = _m_psllqi(m2, 16);

        *(__m64 *)(d0 + 8) = _m_por(_m_por(m0, m1), m2);
        //------------
        r = _m_psrlqi(r, 32);
        g = _m_psrlqi(g, 32);
        b = _m_psrlqi(b, 32);

        m0 = _m_punpcklbw(r, zero);
        m0 = _m_punpcklwd(m0, zero);

        m1 = _m_punpcklbw(g, zero);
        m1 = _m_punpcklwd(m1, zero);
        m1 = _m_psllqi(m1, 8);

        m2 = _m_punpcklbw(b, zero);
        m2 = _m_punpcklwd(m2, zero);
        m2 = _m_psllqi(m2, 16);

        *(__m64 *)(d0 + 16) = _m_por(_m_por(m0, m1), m2);
        //------------
        m0 = _m_punpcklbw(r, zero);
        m0 = _m_punpckhwd(m0, zero);

        m1 = _m_punpcklbw(g, zero);
        m1 = _m_punpckhwd(m1, zero);
        m1 = _m_psllqi(m1, 8);

        m2 = _m_punpcklbw(b, zero);
        m2 = _m_punpckhwd(m2, zero);
        m2 = _m_psllqi(m2, 16);

        *(__m64 *)(d0 + 24) = _m_por(_m_por(m0, m1), m2);
        //------------

        //------------
        yy = *(__m64 *)(y + 8);
        y0 = _m_punpcklbw(yy, zero);
        y1 = _m_punpcklbw(_m_psrlq(yy, 32), zero);

        r0 = _m_paddsw(y0, rc0);
        r1 = _m_paddsw(y1, rc1);
        g0 = _m_paddsw(y0, gc0);
        g1 = _m_paddsw(y1, gc1);
        b0 = _m_paddsw(y0, bc0);
        b1 = _m_paddsw(y1, bc1);

        r = _m_packuswb(r0, r1);
        g = _m_packuswb(g0, g1);
        b = _m_packuswb(b0, b1);
        //------------
        m0 = _m_punpcklbw(r, zero);
        m0 = _m_punpcklwd(m0, zero);

        m1 = _m_punpcklbw(g, zero);
        m1 = _m_punpcklwd(m1, zero);
        m1 = _m_psllqi(m1, 8);

        m2 = _m_punpcklbw(b, zero);
        m2 = _m_punpcklwd(m2, zero);
        m2 = _m_psllqi(m2, 16);

        *(__m64 *)d1 = _m_por(_m_por(m0, m1), m2);
        //------------
        m0 = _m_punpcklbw(r, zero);
        m0 = _m_punpckhwd(m0, zero);

        m1 = _m_punpcklbw(g, zero);
        m1 = _m_punpckhwd(m1, zero);
        m1 = _m_psllqi(m1, 8);

        m2 = _m_punpcklbw(b, zero);
        m2 = _m_punpckhwd(m2, zero);
        m2 = _m_psllqi(m2, 16);

        *(__m64 *)(d1 + 8) = _m_por(_m_por(m0, m1), m2);
        //------------
        r = _m_psrlqi(r, 32);
        g = _m_psrlqi(g, 32);
        b = _m_psrlqi(b, 32);

        m0 = _m_punpcklbw(r, zero);
        m0 = _m_punpcklwd(m0, zero);

        m1 = _m_punpcklbw(g, zero);
        m1 = _m_punpcklwd(m1, zero);
        m1 = _m_psllqi(m1, 8);

        m2 = _m_punpcklbw(b, zero);
        m2 = _m_punpcklwd(m2, zero);
        m2 = _m_psllqi(m2, 16);

        *(__m64 *)(d1 + 16) = _m_por(_m_por(m0, m1), m2);
        //------------
        m0 = _m_punpcklbw(r, zero);
        m0 = _m_punpckhwd(m0, zero);

        m1 = _m_punpcklbw(g, zero);
        m1 = _m_punpckhwd(m1, zero);
        m1 = _m_psllqi(m1, 8);

        m2 = _m_punpcklbw(b, zero);
        m2 = _m_punpckhwd(m2, zero);
        m2 = _m_psllqi(m2, 16);

        *(__m64 *)(d1 + 24) = _m_por(_m_por(m0, m1), m2);
        //------------

        d0 += 8*4;
        d1 += 8*4;

        c += 4;

        y += 64;
      }

      y += 64*6 - 64*2;
      c += 64*6 - 8;
    }

    _m_empty();
  }
  else
  {
#endif

  for (int i = max_mcus_per_row; i > 0; i--)
  {
    for (int l = 0; l < 2; l++)
    {
      for (int j = 0; j < 8; j += 2)
      {
        int cb = c[0];
        int cr = c[64];

        int rc = crr[cr];
        int gc = ((crg[cr] + cbg[cb]) >> 16);
        int bc = cbb[cb];

        int yy = y[j];
        d0[0] = clamp(yy+rc);
        d0[1] = clamp(yy+gc);
        d0[2] = clamp(yy+bc);

        yy = y[j+1];
        d0[4] = clamp(yy+rc);
        d0[5] = clamp(yy+gc);
        d0[6] = clamp(yy+bc);

        yy = y[j+8];
        d1[0] = clamp(yy+rc);
        d1[1] = clamp(yy+gc);
        d1[2] = clamp(yy+bc);

        yy = y[j+8+1];
        d1[4] = clamp(yy+rc);
        d1[5] = clamp(yy+gc);
        d1[6] = clamp(yy+bc);

        d0 += 8;
        d1 += 8;

        c++;
      }
      y += 64;
    }

    y += 64*6 - 64*2;
    c += 64*6 - 8;
  }

#ifdef __ICL
  }
#endif

}

