#ifndef SCRIPT_CONSOLE
#define SCRIPT_CONSOLE

#include <allegro.h>
#include <alfont.h>

void script_init( ALFONT_FONT *console_font );

void draw_console( BITMAP *targ );

void console_key( int k );

void conout( const char *s, int color );

// printflike
void conoutf( const char *s, ... );
void conoutfc( int color, const char *s, ... );

#endif