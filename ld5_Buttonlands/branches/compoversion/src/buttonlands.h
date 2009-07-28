#ifndef BUTTONLANDS_H
#define BUTTONLANDS_H

#include <stdio.h>

#define USE_CONSOLE

#include <allegro.h>
#include <alfont.h>

#include <jpegdecoder.h>


BITMAP *load_jpeg( char *filename, int alpha_thresh=0 );

extern ALFONT_FONT *title_font, *text_font;

extern float pulse, pulse2;

extern BITMAP *cursorNorm, *cursorX, *currCursor;

extern int allSides[];

#endif