Small JPEG Decoder Library v0.93b
Dec. 28, 2001 
Copyright (C) 1994-2000 Rich Geldreich

web: http://www.voicenet.com/~richgel
email: richgel@voicenet.com

--- Introduction

This C++ library decodes JPEG images.  It's not the most full-featured
JPEG library available, but it's relatively fast, completely free (LGPL
license), uses little memory, and should be easy to understand and
further optimize (mostly because it's so small:  the entire library is
only a few thousand lines long).  The very common Huffman DCT-based
sequential and progressive modes of operation are supported.

This library only handles JPEG decompression, upsampling, and colorspace
conversion.  It does not handle pixel format conversion, quantization,
dithering, etc.

--- Who's this library for?

If your application's input set is limited in some way (say, you're
making a game or some other app that has prepared resources), this
library may suite your needs.

This library may also prove useful to those that want to read JPEG
images on very small 32-bit platforms or embedded devices.  See the
porting section, below.  (This code has been successfully ported to work
on SH3, ARM, and MIPS CPU's.)

I do not recommend using this library in a product that requires
extremely high image quality or is going to be reading many arbitrary
images supplied by the user, unless you know exactly what you're doing.
The library does not support the more rare sampling factors/colorspaces,
and only uses a box filter to upsample the chrominance components.

I have tested this particular implementation with over 175 megabytes of
JPEG images from a variety of sources.  Still, this is not a trivial
piece of software, so please e-mail me if you find any bugs.

--- Alternatives

Intel has a free JPEG Library that is also quite fast, but they don't
distribute the source:

http://developer.intel.com/vtune/PERFLIBST/ijl/index.htm

If you are creating a product that requires the highest quality and most
compatibility I recommend using the Independent JPEG Group's free
library (includes source):

http://www.ijg.org

--- Features

Simple (very little code, especially when compared to other JPEG libraries)

Free (LGPL license)

A lot of optional MMX specific optimizations:
    IDCT (using Intel's free MMX IDCT module)
    Merged H2V2 upsampler/YCbCr->RGB conversion
    Fast "getbits"

Supports the most common (widely used) image types:
    Grayscale
    YCbCr colorspace: H1V1, H1V2, H2V1, or H2V2 sampling factors

Progressive images

--- Instructions

Make sure you have Visual C++ 6.0 Service Pack 3 or higher.

Load the workspace file, located at jpgd/jpgd.dsw.  The example console
mode program, jpg2tga, requires two parameters for it to do anything:  a
source filename (a valid JPEG image), and a destination filename (TGA).
Example:

jpg2tga input.jpg output.tga

The image input.jpg will be written to ouput.tga.  You may then view
output.tga to verify that everything worked as expected.

There are a number of useful test images in the "test_images" folder
that can be used to help verify that the decoder is working properly.

Notes:  I hardcoded the "use_mmx" parameter to true in the jpeg_decoder
constructor call..  If you don't have an MMX capable processor you will
need to change this code.  (A production program should check if the
processor supports MMX at run-time and adjust this parameter
accordingly.)

You should be able to compile and link using VC++ 5.0 if you remake the
projects/workspace. 

--- Decoder Streams

To decode JPEG streams straight from memory, or from some other source,
study the jpeg_decoder_stream class which is defined in jpegdecoder.h.
All data input is handled through an object which has this interface as
its base class.

An example decoder stream class that reads JPEG files from standard FILE
streams is included (see jpeg_decoder_file_stream).  The jpg2tga example
program uses this decoder stream class.

--- Porting

I've modified the library so it's much easier to selectively remove the
inline x86 assembler and MMX specific code.  See the following macros in
jpegdecoder.h:

#define SUPPORT_X86ASM
#define SUPPORT_MMX

--- Intel C/C++ Compiler Notes:

To use the Intel C/C++ Compiler (v4.0 or higher):  Make sure the "Intel
C/C++ Compiler" checkbox is checked in the "Select Compiler" tool.  If
it's not, the h2v2.cpp module will not include the MMX implementation.

The h2v2.obj module in the included libs was compiled with the Intel
Compiler-- I hope this causes no trouble for those that don't have this
product.  It shouldn't, but if it does, just recompile.

For some reason, the get_bits_2_mmx() method isn't inlined properly
by ICC, causing a crash in Release builds.  This is why all modules
except h2v2.cpp have the _USE_NON_INTEL_COMPILER symbol defined.  For
now, if you must use ICC to compile the library, either disable inlining
or don't define the SUPPORT_MMX symbol (jpgdecoder.h).

--- Partial Update History
v.93a:

Fixed dumb bug in the decode_next_row functions that could cause bogus non-zero 
coefficients from leaking through in rare cases.

v.93:

prep_in_buffer() now saves/restores the MMX state before the
jpeg_decoder_stream's read() method is called.

Moved a lot of the inline asm/MMX code to jpegdecoder.inl to simplify
the main header file.

Added the SUPPORT_X86ASM and SUPPORT_MMX defines, to ease porting to
non-x86 platforms (see the top of jpegdecoder.h).

v.92:

Removed Intel's old MMX IDCT module, replaced with a newer, free module
from their web site that was specifically designed for JPEG.  I have
detected no overflow problems with this IDCT.

Rewrote the MMX intrinsic code in the h2v2.cpp module.  Should
be faster and more accurate now.

Added MMX specific getbits()/Huffman decoding methods.  Coefficient
decoding is the main bottleneck (approximately 50%!) when MMX is enabled
so it must be done quickly.

Changed main decoder so it does not clear the entire coefficient buffer
every row.  This was really slowing the entire decoder down!

Changed merged upsampling/colorspace conversion methods to write the
destination pixels to a different buffer.  Also changed the RGB output
format from 3 bytes/pixel to 4 bytes/pixel (more efficient for MMX code
to write).

--- History

Some years ago, a friend (Matt Pritchard, now with Ensemble
Studios) and I wrote a 16-bit real mode DOS image viewer named "PowerView"
(PV).  Matt convinced me that we needed to add JPEG support -- no small feat,
especially considering that at the time (1994) JPEG was still rather new.

The Independent JPEG Group's (www.ijg.org) library was definitely an
option, but its sheer bulk seemed like it would have broken PV.  (PV's
memory situation was bleak:  less than ~500k for all code/near
and far data.  It supported EMS/XMS, but only for special cases.  Things
got so bad that I had to use Borland's run-time code swapping/paging
system to free up low RAM.  Modifying the IJG code to handle this
reliably seemed like much more trouble than it was worth.)  Also,
incorporating so much of someone else's code was sure to make the
program less reliable, overall.  We also wanted to stand out from the
crowd, so I decided to create our own implementation.

Approximately one week later, the baseline decoder was working.  The
upsampling/colorspace conversion code, Huffman decoder, and IDCT modules
where written entirely in real-mode assembler.  The IDCT module
performed no multiplies, just lots of overlapped/pipelined, 32-bit adds
from EMS memory lookup tables.  (Each 32-bit add summed two individual
16-bit quantities.)  It also employed an interesting "fast path"
optimization:  using macros, an individual IDCT was created for each
possible input case because many AC coefficients are 0 after
quantization.  (I will probably port this IDCT to 32-bit code and
release it soon.  It performs a lot of table lookups to a 64k buffer, so
I wonder how well it will perform on today's machines.)

Much later on, after the web became very popular, I added support for
progressive images.

A short time after, I wrote a rather complete JPEG encoder from scratch.
This code will be much easier to port because I wrote it keeping
portability in mind.  I'm probably going to release this code within the
next few months.

--- Future Changes

Partial list:

I would like to replace the IJG's integer IDCT module (named IDCT.CPP in
this lib) with something else, because it's copyrighted by Thomas G.
Lane of the IJG.  I am not sure if including this small bit of code
requires the user to include the usual "Parts of this code where written
by the IJG...", etc. statement in their product documentation or "About"
dialog.

Possibly add AMD 3D-Now or P3 specific modules.

Add bilinear filtering to the upsampler/colorspace conversion code.
Right now, only a simple box filter is implemented, which can sometimes
cause noticeable artifacts.  

It would be interesting to see how practical it is to use Direct3D for
upsampling and colorspace conversion.  Upsampling could be done by using
SetRenderTarget() with filtered texture mapping, and various
blending operations for colorspace conversion.  I'm almost positive this
is doable, but I'm not sure if 8-bits per component is really enough.

--- License

Except for those modules which are copyrighted by others, this library
is covered under the LGPL license.  Please see license.txt.

idct.cpp is from the IJG.  It's copyrighted by Thomas G. Lane.  Please
see the top of this file for more information.

