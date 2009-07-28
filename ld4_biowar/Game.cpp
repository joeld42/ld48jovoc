#include <allegro.h>
#include <alfont.h>

#include "Game.h"
#include "ScriptConsole.h"

// fonts
extern ALFONT_FONT *font_console, *font_title, *font_game;

TitleScreen *TitleScreen::theTitle = NULL;
CityMode *CityMode::theCityMode = NULL;

GameMode::GameMode()
{

}

void GameMode::update() 
{
	// TODO:
}

/////////////////////////////////////////////
// Title Screen Mode
/////////////////////////////////////////////

TitleScreen::TitleScreen() 
{
	theTitle = this;
	menu_ndx = 0;

	metal = NULL;
}

void TitleScreen::addMenuItem( std::string nm, PyObject *callback )
{
	MainMenuItem item;
	item.label = nm; 
	item.callback = callback;

	menu.push_back( item );


}

void TitleScreen::draw( BITMAP *targ )
{
	//rectfill( targ, 0,0, 640, 480, 0x0 );

	if (!metal) {
		metal = load_bitmap( "./Graphics/metal1.bmp", NULL );	
	}
	blit( metal, targ, 0,0, 0,0, 640, 480 );	
	
	// titles
	alfont_set_font_size( font_title, 72 );
	alfont_textout_centre( targ, font_title, "Profiteer:",  320, 50, makecol(255, 0, 0) );

	alfont_set_font_size( font_title, 48);
	alfont_textout_centre( targ, font_title, "Germ Warfare",  320, 50+70, makecol(255, 0, 0) );

	// menu items
	alfont_set_font_size( font_title, 30);
	int yval = 50+140;
	int i=0;
	for (std::vector<MainMenuItem>::iterator item = menu.begin();
		item != menu.end(); item++) {
			if (i!=menu_ndx) {
				alfont_textout_centre( targ, font_title, (*item).label.c_str(),  
					320, yval, makecol(60, 60, 255) );
			} else {
				alfont_textout_centre( targ, font_title, (*item).label.c_str(),  
					320, yval, makecol(255, 255, 60) );
			}
			yval += alfont_text_height( font_title );
			i++;
	}
}

void TitleScreen::key_pressed( int k ) {
	switch (k>>8) {
	case KEY_DOWN:
		menu_ndx++; 
		if (menu_ndx == menu.size()) {
			menu_ndx = 0;
		}
		break;
	case KEY_UP:
		menu_ndx--;
		if  (menu_ndx < 0) {
			menu_ndx = menu.size() - 1;
		}
		break;
	case KEY_ENTER:
		menuActivate();
		break;
	}
}

void TitleScreen::menuActivate()
{
	PyObject *cb = menu[menu_ndx].callback;

	if (cb == Py_None) {
		conoutf("main menu item %s has no callback!", menu[menu_ndx].label.c_str() );
	} else {
		PyObject *arglist;
		arglist = Py_BuildValue("()" );
		PyEval_CallObject(cb, arglist);
	}
}

/////////////////////////////////////////////
// City Mode
/////////////////////////////////////////////

// a city's icon on the map
struct MapCity {
	int x, y;

};

CityMode::CityMode()
{
	assert( theCityMode== NULL );
	theCityMode = this;

	citybg = NULL;
}

void CityMode::draw( BITMAP *targ )
{
	if (!citybg) {
		citybg = load_bitmap( "./Graphics/cityscreen.bmp", NULL );	
	}
	blit( citybg, targ, 0,0, 0,0, 640, 480 );
}

void CityMode::key_pressed( int k )
{
}