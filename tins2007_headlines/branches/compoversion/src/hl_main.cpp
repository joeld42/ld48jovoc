#include <iostream>
#include <vector>

#include <math.h>

#ifndef NDEBUG
#define USE_CONSOLE
#endif

#include <allegro.h>

#include <alfont.h>

#include "panel.h"
#include "recycle_bin.h"
#include "clipping.h"
#include "headline.h"
#include "furnace.h"
#include "folder.h"
#include "wire_news.h"
#include "layout.h"

using namespace std;

// game state
enum {
	STATE_TITLE,
	STATE_GAME,
	STATE_GAMEOVER
};

// silly smoke
struct Smoke
{
	float x, y;
	float age;	
	float speed, drift;
};

// game data
std::vector<Panel*> g_panels;
std::vector<Clipping*> g_pasteboard;
Clipping *g_activeClip = NULL;

// when user cancels drop (or tries to drop
// on an invalid target), the clips snap back to
// where they came from
std::vector<Clipping*> g_goingHome;

BITMAP *smoke;
std::vector<Smoke> smokelist;

// desk accessories
BITMAP *desklamp;
BITMAP *btn_publish;

// timer
Panel *p_deadline;
BITMAP *dl_hurry;
BITMAP *deadline;

// furnace
Furnace *p_furnace;

// cutting tools
BITMAP *currTool = NULL;
BITMAP *tool_xacto, *tool_scissor;
int xacto_pos;

// pattern
BITMAP *pasteboard = NULL;

// timer
float TOT_TIME = (60.0f * 5);
float timeLeft;

bool winner;
bool cheats_unlocked = false;

WireNews *p_wirenews;

enum {
	LEVEL_EASY,
	LEVEL_MEDIUM,
	LEVEL_HARD,
	LEVEL_INSANE
};
int level = LEVEL_EASY;

// Fonts
ALFONT_FONT *g_fontWireTiny;
ALFONT_FONT *g_fontWireHeadline; 
ALFONT_FONT *g_fontTitleScreen;
ALFONT_FONT *g_fontNews;

BITMAP *logo;




//=========================================================
// draw title screen
//=========================================================
void draw_title( BITMAP *dest, bool fetchmode )
{
	// clear backbuf
	//rectfill( dest, 0, 0, 800, 600, makecol32( 230, 245, 255 ) );
	static int ax;
	drawing_mode( DRAW_MODE_COPY_PATTERN, pasteboard, ax, 0 );
	rectfill( dest, 0, 0, 800, 600, makecol32( 100, 0, 100 ) );
	drawing_mode( DRAW_MODE_SOLID, NULL, 0, 0 );
	ax++; if (ax > pasteboard->w) ax = 0;

	masked_blit( logo, dest, 0, 0, 400, 50, logo->w, logo->h );
	
	alfont_set_font_size( g_fontTitleScreen, 72 );

	for (int i=-2; i <= 2; i++)  {
		for (int j=-2; j <= 2; j++) {

			alfont_textout_centre( dest, g_fontTitleScreen, "Headline", 250+i, 50+j, 
						0);
			alfont_textout_centre( dest, g_fontTitleScreen, "Recycler", 250+i, 115+j, 0);
		}
	}
	alfont_textout_centre( dest, g_fontTitleScreen, "Headline", 250, 50, 
						makecol32( 255, 188, 83 ) );
	alfont_textout_centre( dest, g_fontTitleScreen, "Recycler", 250, 115, 
						makecol32( 255, 188, 83 ) );

	if (!fetchmode)
	{
		alfont_set_font_size( g_fontTitleScreen, 24 );

		// difficulty level
		alfont_textout_centre( dest, g_fontTitleScreen, "Easy", 
			250, 200, ((level==LEVEL_EASY)?makecol( 100, 255, 100 ):makecol( 0, 128, 0) ) );

		alfont_textout_centre( dest, g_fontTitleScreen, "Medium", 
			250, 230, ((level==LEVEL_MEDIUM)?makecol( 100, 255, 100 ):makecol( 0, 128, 0) ) );

		alfont_textout_centre( dest, g_fontTitleScreen, "Hard", 
			250, 260, ((level==LEVEL_HARD)?makecol( 100, 255, 100 ):makecol( 0, 128, 0) ) );

		alfont_textout_centre( dest, g_fontTitleScreen, "Insane", 
			250, 290, ((level==LEVEL_INSANE)?makecol( 100, 255, 100 ):makecol( 0, 128, 0) ) );


		for (int i=-2; i <= 2; i++)  {
			for (int j=-2; j <= 2; j++) {	
				alfont_textout_centre( dest, 
					g_fontTitleScreen, "Press SPACE to start", 
					250+i, 350+j, 0);
			}
		}
		alfont_textout_centre( dest, 
					g_fontTitleScreen, "Press SPACE to start", 
					250, 350, makecol32( 255, 188, 83 ) );
	}


	alfont_set_font_size( g_fontWireHeadline, 12 );
	alfont_textout_centre( dest, g_fontWireHeadline, 
		"Created for TINS 2007 -- by Joel Davis (joeld42@yahoo.com)",
		400, 560, makecol32( 30, 65, 100 ) );

}

//=========================================================
// draw game screen
//=========================================================
void draw_game( BITMAP *dest )
{
	// clear backbuf
	drawing_mode( DRAW_MODE_COPY_PATTERN, pasteboard, 0, 0 );
	rectfill( dest, 0, 0, 800, 600, makecol32( 100, 0, 100 ) );
	drawing_mode( DRAW_MODE_SOLID, NULL, 0, 0 );
	
	// draw pasteboard clippings
	for (std::vector<Clipping*>::iterator ci = g_pasteboard.begin();
		 ci != g_pasteboard.end(); ci++)
	{
		Clipping *c = (*ci);

		// TODO: draw BIGSPRITE
		//draw_sprite( dest, c->m_sprite, c->m_x, c->m_y );
		masked_stretch_blit( c->m_sprite, dest,
					0, 0, 
					c->m_sprite->w, c->m_sprite->h, 
					c->m_x, c->m_y, 
					c->m_sprite->w * 2, c->m_sprite->h * 2  );

	}

	// draw panels
	for (std::vector<Panel*>::iterator pi = g_panels.begin(); pi != g_panels.end(); pi++)
	{
		// tell the panel to redraw itself
		(*pi)->draw();

		// now blit the panel to backbuf			
		if ((*pi) == p_furnace)
		{
			// cheat on the panel			
			draw_trans_sprite( dest, p_furnace->m_surf, (*pi)->m_x, (*pi)->m_y );
		}
		else
		{
			masked_blit( (*pi)->m_surf, dest, 0,0, 
					   (*pi)->m_x, (*pi)->m_y, 
					   (*pi)->m_w, (*pi)->m_h );
		}
	}

	// timer line
	for (int i=0; i < 3; i++)
	{
		hline( dest, p_deadline->m_x + 8, 
			   p_deadline->m_y + (1.0f-(timeLeft / TOT_TIME)) * 210 + 32 + i,
			   p_deadline->m_x + 42, 
			   i==1?makecol32( 0, 255, 255 ):makecol32( 0, 128,128) );				
	}

	// draw the silly desklamp
	masked_blit( desklamp, dest, 0,0, 680, -15, desklamp->w, desklamp->h );

	// draw the "publish" button
	draw_sprite( dest, btn_publish, 593, 230 );


	// draw active clippings
	if (g_activeClip)
	{
		// todo: draw BIGSPRITE
		//draw_sprite( dest, g_activeClip->m_sprite, 
		//				g_activeClip->m_x, g_activeClip->m_y );
		masked_stretch_blit( g_activeClip->m_sprite, dest,
					0, 0, 
					g_activeClip->m_sprite->w, g_activeClip->m_sprite->h, 
					g_activeClip->m_x, g_activeClip->m_y, 
					g_activeClip->m_sprite->w * 2, g_activeClip->m_sprite->h * 2  );
	}

	// draw clippings heading home
	for (std::vector<Clipping*>::iterator ci = g_goingHome.begin();
		 ci != g_goingHome.end(); ci++)
	{
		Clipping *c = (*ci);		
		draw_sprite( dest, c->m_sprite, c->m_x, c->m_y );
	}

	// draw smoke	
	for (int i=0; i < smokelist.size(); i++ )
	{
		Smoke &s = smokelist[i];
		if (s.age < 0.0) continue;

		int f = int(s.age * 7.0f); // frame;
		int sz = 5 + s.age*20.0f;

		masked_stretch_blit( smoke, dest,
			f*32, 0, 32, 32,
			s.x - sz, s.y - sz, sz*2, sz*2);		
	}	

	// draw tools in toolbox
	if (currTool != tool_xacto)
	{
		draw_sprite( dest, tool_xacto, 0, 400 );
	}
	if (currTool != tool_scissor)
	{
		draw_sprite( dest, tool_scissor, 0, 464 );
	}

}



//=========================================================
// Main Program
//=========================================================
int main( int argc, char *argv[] )
{
	BITMAP *backbuf; // main screen
	int state = STATE_TITLE; // game state
	HeadlinePool hpool;
	bool init_headlines = false;	

	

	// gameplay stuff
	int clip_grab_x, clip_grab_y;

	// initialize allegro
	allegro_init();
	install_keyboard();
	install_mouse();
	install_timer();

	set_window_title( "Headline Recycler" );

	alfont_text_mode( -1 );
	alfont_init();
	g_fontWireTiny = alfont_load_font( "gamedata/slkscr.ttf" );
	g_fontWireHeadline = alfont_load_font( "gamedata/04B_25__.ttf" );

	g_fontNews = alfont_load_font( "gamedata/X_BAMBI.ttf" );

	g_fontTitleScreen = alfont_load_font( "gamedata/CREABBB_.TTF" );
	
	if ((!g_fontWireTiny) || (!g_fontWireHeadline))
	{
		set_gfx_mode( GFX_TEXT, 0,0,0,0 );
		allegro_message( "Unable to load font.\n" );
		return 1;
	}
	
	// Set up graphics mode
	set_color_depth( 32 );
	if (set_gfx_mode( GFX_AUTODETECT_WINDOWED, 800, 600, 0, 0 ) )
	{
		set_gfx_mode( GFX_TEXT, 0,0,0,0 );
		allegro_message( "Unable to set graphic mode\n%s\n", allegro_error );
		return 1;
	}

	// enable hardware cursor
	enable_hardware_cursor();
	select_mouse_cursor( MOUSE_CURSOR_ARROW );
	show_mouse( screen );

	// set up back buffer
	backbuf = create_bitmap( 800, 600 );

	//===============================
	// Set up game objects
	//===============================

	logo = load_bitmap( "gamedata/logo.bmp", NULL );

	// Recycle Bin
	RecycleBin *p_recycle = new RecycleBin( "Recycle Bin", &hpool,
						makecol32( 50, 50, 255 ), 0, 0, 250, 250 );
	p_recycle->m_background = load_bitmap( "gamedata/recycle.bmp", NULL );
	if (!p_recycle->m_background)
	{
		cout << allegro_error << endl;
	}
	g_panels.push_back( p_recycle );


	// wire news
	p_wirenews = new WireNews( "Wire News", &hpool,
						makecol32( 100, 255, 100 ), 800-550, 0, 250, 250 );
	p_wirenews->m_background = load_bitmap( "gamedata/layout_table.bmp", NULL );
	g_panels.push_back( p_wirenews );


	// page layout
	pasteboard = load_bitmap( "gamedata/pasteboard_pat.bmp", NULL );	
	NewsLayout *p_layout = new NewsLayout( "Layout", 
						makecol32( 255, 180, 100 ), 800-300, 0, 250, 250 );
	p_layout->m_background = p_wirenews->m_background;
	desklamp = load_bitmap( "gamedata/desklamp.bmp", NULL );
	btn_publish = load_bitmap( "gamedata/btn_publish.bmp", NULL );
	p_layout->setNCols( 5 );
	g_panels.push_back( p_layout );

	// deadline
	// should be a separate deadline class but out of time...
	dl_hurry = load_bitmap( "gamedata/dl_hurry.bmp", NULL );
	deadline = load_bitmap( "gamedata/deadline.bmp", NULL );
	p_deadline = new Panel( "DL", makecol32( 164, 164, 164 ), 800-50, 0, 50, 250 );
	p_deadline->m_background = deadline;
	g_panels.push_back( p_deadline);	


	// Buttons	
	//g_panels.push_back( new Panel( "BTNS", 
	//					makecol32( 255, 255, 0 ), 0, 600 - 200, 64, 200 ));
	tool_xacto = load_bitmap( "gamedata/tool_xacto.bmp", NULL );
	tool_scissor = load_bitmap( "gamedata/tool_scissor.bmp", NULL );
	
	ClipFolder *p_folder = new ClipFolder( "Folder", 
						makecol32( 255, 255, 0 ), 64, 600 - 200, 700-64, 200 );
	p_folder->m_background = load_bitmap( "gamedata/folder.bmp", NULL );

	g_panels.push_back( p_folder );

	// Furnace
	p_furnace = new Furnace( "Furnace", makecol32( 255, 0, 0 ), 700, 600 - 200, 100, 200 );
	g_panels.push_back( p_furnace );
	
	// load gfx
	smoke = load_bitmap( "gamedata/smoke_anim.bmp", NULL );

	// set blend mode for fire
	set_screen_blender(0, 0, 0, 255);

	// mainloop
	bool done = false;
	while (!done) {

		switch( state )
		{
		case STATE_TITLE:
			draw_title( backbuf, false );
			break;

		case STATE_GAME:
			draw_game( backbuf );
			break;

		case STATE_GAMEOVER:
						
			// draw cool fire effect over the paper
			blit( p_layout->m_newsImg, p_layout->m_newsImg2,
				  0, 0, 0, 0, 
				  p_layout->m_newsImg->w,
				  p_layout->m_newsImg->h );

			draw_trans_sprite( p_layout->m_newsImg2, p_furnace->m_surf, 
				   (p_layout->m_newsImg->w / 2) - (p_furnace->m_surf->w / 2), 
							  0);

			stretch_blit( p_layout->m_newsImg2, backbuf,
						0, 0, 
								  p_layout->m_newsImg->w, 
								  p_layout->m_newsImg->h,

								  400 - p_layout->m_newsImg->w, 
								  300 - p_layout->m_newsImg->h, 
								  p_layout->m_newsImg->w *2, 
								  p_layout->m_newsImg->h *2 );									
			break;
		}

		// Initialize headlines
		if (!init_headlines)
		{	

			// draw something to look at
			draw_title( screen, true );

#if 1
			if (!hpool.populate( "gamedata/feeds.xml" ))
			{
				done = true;
			}
#endif

#if 0
			for (HeadlinePool::WordInfoMap::iterator wi = hpool.m_wordmap.begin();
					wi != hpool.m_wordmap.end(); wi++) 
			{
				g_pasteboard.push_back( new Clipping( (*wi).second.m_word ) );
			}


			// fake to put these in the folder, should be in folder panel
			for (std::vector<Clipping*>::iterator ci = g_pasteboard.begin();
				ci != g_pasteboard.end(); ci++)
			{
				(*ci)->m_x = (rand() % 700) + 50;
				(*ci)->m_y = (rand() % 150) + 400;
			}
#endif


			init_headlines = true;
		}

		// Handle events
		if (keypressed()) {
			int k = readkey();
			
			if (state==STATE_TITLE) {

				switch (k>>8)
				{
					case KEY_SPACE:
						// reset game state based on difficulty
						// level
						switch (level) {
							case LEVEL_EASY:
								p_layout->setNCols( 3 );
								TOT_TIME = 60.0f * 10.0f;
								break;
							case LEVEL_MEDIUM:
								p_layout->setNCols( 5 );
								TOT_TIME = 60.0f * 10.0f;
								break;
							case LEVEL_HARD:
								p_layout->setNCols( 8 );
								TOT_TIME = 60.0f * 10.0f;
								break;
							case LEVEL_INSANE:
								p_layout->setNCols( 10 );
								TOT_TIME = 60.0f * 15.0f;
								break;
						}				

						// start the game
						state = STATE_GAME;
						timeLeft = TOT_TIME;
						break;

					case KEY_UP:
						level--; if (level < LEVEL_EASY) level = LEVEL_INSANE;
						break;

					case KEY_DOWN:
						level++; if (level > LEVEL_INSANE) level = LEVEL_EASY;
						break;

					case KEY_F11:
						cheats_unlocked = true;
						break;
				}
			}
			else if (state==STATE_GAMEOVER) 
			{
				// reset game state and go to title screen
				state = STATE_TITLE;
				timeLeft = TOT_TIME;
				p_layout->setNCols( 5 );
				p_folder->m_clips.clear();
				p_recycle->m_clips.clear();
				p_wirenews->m_heads.clear();
				
				// memleak -- delete these clippings.
				g_goingHome.clear();

				p_wirenews->m_nextHeadlineTime = HEADLINE_TIME;
				g_pasteboard.clear();
			}
			else if (state==STATE_GAME) {
				switch (k>>8)
				{
				// DBG: Cheat codes
				case KEY_P:
					if (cheats_unlocked) {
						// print story
						std::string hcheat = "I am a cheating bastard";
						p_layout->printStory( hcheat );

						cout << "Stories: ";
						for (int i=0; i < p_layout->m_storiesLeft.size(); i++)
						{
							cout << " " << p_layout->m_storiesLeft[i]; 
						}
						cout << endl;

						// kick the panel
						p_layout->m_dirty = true;
					}
					break;
				case KEY_3:
					if (cheats_unlocked) {
						p_layout->setNCols( 3 );
					}
					break;
				case KEY_5:
					if (cheats_unlocked) {
						p_layout->setNCols( 5 );
					}
					break;
				case KEY_8:
					if (cheats_unlocked) {
						p_layout->setNCols( 8 );
					}
					break;
				case KEY_0:
					if (cheats_unlocked) {
						p_layout->setNCols( 10 );
					}
					break;
				case KEY_EQUALS:
					if (cheats_unlocked) {
						timeLeft = TOT_TIME;
					}
					break;
				}
			}
		}

#if 0
		for (std::vector<Clipping*>::iterator ci = g_pasteboard.begin();
						ci != g_pasteboard.end(); ci++)
		{
			rect( backbuf,  (*ci)->m_x, (*ci)->m_y,
				(*ci)->m_x + (*ci)->m_sprite->w,
				(*ci)->m_y + (*ci)->m_sprite->h,
				makecol32( 255,0,0 ) );
		}
#endif

		// Get the panel that the mouse is over 
		// or NULL for pasteboard
		Panel *currPanel = NULL;
		for (std::vector<Panel*>::iterator pi = g_panels.begin(); 
							pi != g_panels.end(); pi++)		
		{
			if ((*pi)->hit( mouse_x, mouse_y ) )
			{
				currPanel = (*pi);
				break;
			}
		}			

		//cout << "Curr panel " << (currPanel?currPanel->m_name:"None") << endl;

		// mouse events
		static int old_mouse_b = 0;
		if (mouse_b != old_mouse_b)
		{
			// click or release
			if (!(old_mouse_b & 1) && (mouse_b & 1))
			{
				printf("%d %d click %d %d\n", old_mouse_b, mouse_b, mouse_x, mouse_y );

				bool check_pasteboard = true;

				// Check for the mini-delete button
				if (currPanel == p_wirenews)
				{
					p_wirenews->click( mouse_x - p_wirenews->m_x,
									   mouse_y - p_wirenews->m_y );
					check_pasteboard = false;
				}


				// If we're not holding a clip, check if we've picked up a tool
				if ( (!g_activeClip) && (mouse_x < 64) && (mouse_y > 400) && 
						(mouse_y < 464 ) )
				{
					currTool = tool_xacto;
				}
				else if ( (!g_activeClip) && (mouse_x < 64) && (mouse_y > 464) && 
						(mouse_y < 464+64 ) )
				{
					currTool = tool_scissor;
				}

				// If we're not holding a clip, check if this hits the publish button
				if ( (!g_activeClip) && (mouse_x >=593) && (mouse_y >= 230) &&
						(mouse_x < 593+64) && (mouse_y < 230+35) )
				{
					//==================
					// Publish
					//==================
					printf(" Publish !! " );

					std::string hltext;
					if (p_wirenews->checkHeadline( g_pasteboard, hltext ))
					{															
						int sx, sy;
						p_layout->printStory( hltext,  &sx, &sy );

						// tell all the clips on the pasteboard to go home to 
						// the layout
						for (std::vector<Clipping*>::iterator ci = g_pasteboard.begin();
							ci != g_pasteboard.end(); ci++)
						{
							Clipping *c = (*ci);
							c->m_published = true;
							c->m_origX = sx;
							c->m_origY = sy;
							c->m_dragSrc = p_layout;

							g_goingHome.push_back( c );
						}

						g_pasteboard.clear();
					}

				}

				// Check if this picks up a clip from recycle panel				
				for (std::vector<Clipping*>::iterator ci = p_recycle->m_clips.begin();
					 ci != p_recycle->m_clips.end(); ci++)
				{
					if ((*ci)->hit( mouse_x - p_recycle->m_x, mouse_y - p_recycle->m_y, 1 ))
					{
						std::cout << "Hit " << (*ci)->m_text << std::endl;
						g_activeClip = (*ci);
						clip_grab_x = mouse_x - (*ci)->m_x;
						clip_grab_y = mouse_y - (*ci)->m_y;
						p_recycle->m_clips.erase( ci );
						check_pasteboard = false;

						// remember clip src
						g_activeClip->m_origX = g_activeClip->m_x;
						g_activeClip->m_origY = g_activeClip->m_y;
						g_activeClip->m_dragSrc = p_recycle;

						break;
					}
				}

				// Check if this picks up a clip from the folder
				for (std::vector<Clipping*>::iterator ci = p_folder->m_clips.begin();
					 ci != p_folder->m_clips.end(); ci++)
				{
					if ((*ci)->hit( mouse_x - p_folder->m_x, mouse_y - p_folder->m_y, 1 ))
					{
						std::cout << "Hit " << (*ci)->m_text << std::endl;
						g_activeClip = (*ci);
						clip_grab_x = (mouse_x - p_folder->m_x) - (*ci)->m_x;
						clip_grab_y = (mouse_y - p_folder->m_y) - (*ci)->m_y;
						p_folder->m_clips.erase( ci );
						check_pasteboard = false;

						// remember clip src
						g_activeClip->m_origX = g_activeClip->m_x;
						g_activeClip->m_origY = g_activeClip->m_y;
						g_activeClip->m_dragSrc = p_folder;

						break;
					}
				}
				

				// check if this picks up a clip from the pasteboard
				if ((!currPanel) && (check_pasteboard))
				{
					for (std::vector<Clipping*>::iterator ci = g_pasteboard.begin();
						ci != g_pasteboard.end(); ci++)
					{					
						if ( (*ci)->hit( mouse_x, mouse_y, 2 ) )
						{
							std::cout << "Hit " << (*ci)->m_text << std::endl;
							g_activeClip = (*ci);
							clip_grab_x = mouse_x - (*ci)->m_x;
							clip_grab_y = mouse_y - (*ci)->m_y;
							g_pasteboard.erase( ci );
							break;
						}
					}
				}
			}
			else if ((old_mouse_b & 1) && !(mouse_b & 1))
			{
				// btn release -- drop clip
				if (g_activeClip)
				{
					if (!currPanel)
					{
						// drop on pasteboard
						g_pasteboard.push_back( g_activeClip );
						g_activeClip = NULL;
					}
					else
					{
						// see if panel wants it
						if (currPanel->dropClip( g_activeClip, mouse_x - currPanel->m_x, 
													mouse_y - currPanel->m_y ) )
						{
							// let the panel deal with it
							g_activeClip = NULL;

							if (currPanel->m_name == "Furnace" )
							{
								for (int i = 0; i < 15; i++)
								{
									Smoke s;
									s.x = mouse_x;
									s.y = mouse_y;
									s.age  = -0.7f;									
									s.speed = ((float)rand() / (float)RAND_MAX) + 0.5;
									s.drift = (((float)rand() / (float)RAND_MAX) - 0.5f) * 30.0f;
									smokelist.push_back( s );
								}
							}
						}
						else
						{
							// Do something with released clip:
							// when user cancels drop (or tries to drop
							// on an invalid target), the clips snap back to
							// where they came from
							g_goingHome.push_back( g_activeClip );
							g_activeClip = NULL;
						}

					}
				}
				else if (currTool)
				{
					// Apply tool
					if ((currTool == tool_scissor) ||
						(currTool == tool_xacto) )
					{
						Clipping *choppy = NULL;
						for (std::vector<Clipping*>::iterator ci = g_pasteboard.begin();
							ci != g_pasteboard.end(); ci++)
						{					
							if ( (*ci)->hit( mouse_x, mouse_y, 2 ) )
							{
								std::cout << "Hit " << (*ci)->m_text << std::endl;							
								choppy = (*ci);
								g_pasteboard.erase( ci );
								break;
							}
						}

						if ((choppy) && (currTool == tool_scissor))
						{
							// Make clips for each letter
							for (int i=0; i < choppy->m_text.size(); i++)
							{
								g_pasteboard.push_back( new Clipping( 
									choppy->m_text.substr( i, 1 ),
									(choppy->m_x - 5) + i*25 + (rand() % 5), 
									choppy->m_y + (rand() % 5) ) );
							}
							delete(choppy);
						}
						else if ((choppy) && (currTool == tool_xacto) &&
							
								// bugfix: xacto_pos >= strlen
								(xacto_pos < choppy->m_text.size())
									)
						{					
							// cut clipping into two halves
							g_pasteboard.push_back( new Clipping( 
								choppy->m_text.substr( 0, xacto_pos ),
								choppy->m_x -5, choppy->m_y -2 ) );

							g_pasteboard.push_back( new Clipping( 
								choppy->m_text.substr( xacto_pos, std::string::npos ),
								choppy->m_x +15 + 16* xacto_pos, choppy->m_y +2 ) );

							// delete the origional
							delete(choppy);
						}
					}

					// drop tool 
					currTool = NULL;					
				}
			}


			// remember state
			old_mouse_b = mouse_b;
		}

		// Update the active clip if the user is holding one
		if (g_activeClip)
		{
			g_activeClip->m_x = mouse_x - clip_grab_x;
			g_activeClip->m_y = mouse_y - clip_grab_y;
		}

		// draw tool
		if (currTool)
		{
			draw_sprite( backbuf, currTool, mouse_x - 20, mouse_y-32 );

			// draw preview of cut
			Clipping *choppy = NULL;
			for (std::vector<Clipping*>::iterator ci = g_pasteboard.begin();
				  ci != g_pasteboard.end(); ci++)
			{					
				if ( (*ci)->hit( mouse_x, mouse_y, 2 ) )
				{								
					choppy = (*ci);			
					break;
				}			
			}

			if (choppy)
			{
				for (int i=1; i < choppy->m_text.size(); i++)
				{				
					int cx = choppy->m_x + i*(CHAR_W*2) + 10;
					if ((currTool==tool_scissor) ||
						((currTool==tool_xacto) && (abs( mouse_x - cx ) < CHAR_W ) )
						) 
					{
						vline( backbuf, cx,
							  choppy->m_y, choppy->m_y + 40,
							makecol32( 255, 0, 0 ) );				

						xacto_pos = i;
					}
				}
			}

		}
				

		// DBG:
#if 0
		char buff[1024];
		sprintf( buff, "Next headline in %f", p_wirenews->m_nextHeadlineTime );
		textout( backbuf, font, buff, 10, 10, makecol( 255,255,255) );
#endif 

		// "flip" backbuffer -- just blit
		vsync();
		acquire_screen();
		blit( backbuf, screen, 0,0,0,0, 800, 600 );
		release_screen();

		// update game state
		static int last_retrace_count = 0;
		static int leftover = 0;
		if (last_retrace_count == 0)
		{
			last_retrace_count = retrace_count;
		}

		// add diff to leftover
		int diff = retrace_count - last_retrace_count;
		leftover += diff;
		last_retrace_count = retrace_count;
		
		// do updates every 3 retraces
		const int retrace_step = 3;
		while ( leftover >= retrace_step )
		{
			leftover -= retrace_step;
			
			// allegro simulates fixed 70 hz retrace
			float dt = (1.0f / 70.0f) * retrace_step;

			//========================
			// update ourselves
			//========================
			if (state == STATE_GAME )
			{
				// check for win
				if ( ((!p_layout->m_dirty) && (p_layout->m_storiesLeft.size() == 0) ) ||
					  (timeLeft <= 0.0f) ) 
				{
					winner = timeLeft > 0.0f;

					// TODO: Alfont and winner graphic
					textout_centre( p_layout->m_newsImg, font,
						winner?"***** YOU WIN *****":"***** YOU LOSE *****",
						p_layout->m_newsImg->w / 2, 
						p_layout->m_newsImg->h / 2,
						winner?makecol32( 0, 255, 0 ):makecol32( 255, 0, 0 ) );

					// display their handiwork
					rectfill( backbuf, 
								  400 - (p_layout->m_newsImg->w + 10), 
								  300 - (p_layout->m_newsImg->h + 10), 
								  400 + (p_layout->m_newsImg->w + 10), 
								  300 + (p_layout->m_newsImg->h + 10), 
								  0);	

					stretch_blit( p_layout->m_newsImg, backbuf,
								  0, 0, 
								  p_layout->m_newsImg->w, 
								  p_layout->m_newsImg->h,

								  400 - p_layout->m_newsImg->w, 
								  300 - p_layout->m_newsImg->h, 
								  p_layout->m_newsImg->w *2, 
								  p_layout->m_newsImg->h *2 );						

					// game ovar
					state = STATE_GAMEOVER;					
				}

				// update timeleft
				timeLeft -= dt;
				if ((timeLeft < 60.0f) && ((int)(timeLeft * 3) & 1) )
				{
					p_deadline->m_background = dl_hurry;
				}
				else
				{
					p_deadline->m_background = deadline;
				}				

				// update smoke			
				std::vector<Smoke> smoke2;
				for (std::vector<Smoke>::iterator si = smokelist.begin();
						 si != smokelist.end(); si++ )
				{				
					(*si).age += dt * (*si).speed;
					
					if ((*si).age < 1.0f)				
					{
						(*si).y -= 50.0 * dt;
						if ((*si).age > 0.0f) 
						{
							(*si).x += (*si).drift * dt;
						}

						if ( (*si).y > 0 )
						{
							smoke2.push_back( *si );
						}
					}
				}
				smokelist = smoke2;

				// Update clips heading home
				std::vector<Clipping*> gh2;
				for (std::vector<Clipping*>::iterator ci = g_goingHome.begin();
					ci != g_goingHome.end(); ci++)
				{
					Clipping *c = (*ci);
					int destX, destY;
					destX = c->m_dragSrc->m_x + c->m_origX;
					destY = c->m_dragSrc->m_y + c->m_origY;

					float dirX = destX - c->m_x;
					float dirY = destY - c->m_y;
					float l = sqrt( dirX * dirX + dirY * dirY );

					// are we close enuf?
					if ( l < 10.0f )
					{
						c->m_dragSrc->dropClip( c, c->m_origX, c->m_origY );
					}
					else
					{					
						dirX /= l;
						dirY /= l;
						float speed = 1600.0f;
						if ( l < 100.0f ) speed = 400.0f;
						c->m_x += dirX * dt * speed;
						c->m_y += dirY * dt * speed;
						gh2.push_back( c );					
					}
				}
				g_goingHome = gh2;

				// update all panels
				for (std::vector<Panel*>::iterator pi = g_panels.begin(); 
					pi != g_panels.end(); pi++)
				{				
					(*pi)->update( dt );
				}

			}
			else if (state==STATE_GAMEOVER)
			{
				// just update furnace :)
				p_furnace->update( dt );
				p_furnace->draw();				
			}
		}
		
		// check for escape or quit
		if (key[KEY_ESC])
		{
			done = true;
		}


		// don't hog cpu
		yield_timeslice();
	}

	return 0;	
}
END_OF_MAIN()