#include <list>
#include <ctype.h>
#include <string.h>

#include <python.h>


#include "ScriptConsole.h"
#include "Game.h"
#include "Map.h"

ALFONT_FONT *f_cons = NULL;

#define CONSOLE_BG_COLOR   107,142,35

#define CONSOLE_TEXT_COLOR 255,215,0
#define CONSOLE_ERR_COLOR 255,10,0
#define CONSOLE_PROMPT_COLOR 0, 255, 255

#define CONSOLE_MARG (10)
#define CONSOLE_MAX_LINES (15)
#define CONSOLE_PROMPT ">>> "

extern GameMode *g_mode;

class ConsoleText {
public:
	ConsoleText( std::string s, int c) : txt(s), color(c) {}
	std::string txt;
	int color;
};

std::list<ConsoleText> console_text;

/////////////////////////////////////////////////////////////////////////////////////////
// python interface
/////////////////////////////////////////////////////////////////////////////////////////

PyObject *pModule, *pGlobals;

PyObject *
py_conout(PyObject *self, PyObject *args)
{
    char *text;	
	PyObject *color=NULL;
	int c;

    if (!PyArg_ParseTuple(args, "s|O", &text, &color))
        return NULL;

	c = makecol(CONSOLE_TEXT_COLOR);

	if (color) {
		if (!PyTuple_Check( color ) || (PyTuple_Size(color)!=3) ) {
			PyErr_SetString(PyExc_ValueError, "Expected (r,g,b) tuple for color" );
			return 0;
		}
		int r,g,b;
		r = PyInt_AsLong(PyTuple_GetItem( color, 0 ));
		g = PyInt_AsLong(PyTuple_GetItem( color, 1 ));
		b = PyInt_AsLong(PyTuple_GetItem( color, 2 ));
		
		c = makecol( r,g,b );
	}
    
	conout( text, c );

    Py_INCREF( Py_None );
	return Py_None;
}

PyObject *
py_newmap(PyObject *self, PyObject *args)
{
	char *tileset;
	int hite;

	if (!PyArg_ParseTuple( args, "is", &hite, &tileset )) {
		return NULL;
	}
	
	Map *map = new Map( hite, tileset );

	return PyInt_FromLong( (long)map );
}

PyObject *
py_destroymap(PyObject *self, PyObject *args)
{
	Map *map;

	if (!PyArg_ParseTuple( args, "i", &map )) {
		return NULL;
	}
	
	delete map;
	
	Py_INCREF( Py_None );
	return Py_None;
}

PyObject *
py_mapget(PyObject *self, PyObject *args)
{
	Map *map;
	int x, y;

	if (!PyArg_ParseTuple( args, "iii", &map, &x, &y )) {
		return NULL;
	}
	
	return PyInt_FromLong( map->m_mapdata[x][y] );	
}

PyObject *
py_mapset(PyObject *self, PyObject *args)
{
	Map *map;
	int x, y, tile;

	if (!PyArg_ParseTuple( args, "iiii", &map, &x, &y, &tile )) {
		return NULL;
	}
	
	map->m_mapdata[x][y] = tile;

	Py_INCREF( Py_None );
	return Py_None;
}

PyObject *
py_mapsetactive(PyObject *self, PyObject *args)
{
	Map *map;	

	if (!PyArg_ParseTuple( args, "i", &map )) {
		return NULL;
	}
	
	map->g_activeMap = map;

	Py_INCREF( Py_None );
	return Py_None;
}


PyObject *
py_mapgetactive(PyObject *self, PyObject *args)
{
	
	if (!PyArg_ParseTuple( args, "")) {
		return NULL;
	}
	
	return PyInt_FromLong( (long)Map::g_activeMap );
}

PyObject *
py_addMainMenu(PyObject *self, PyObject *args)
{
	char *label;
	PyObject *cb;

	if (!PyArg_ParseTuple( args, "sO", &label, &cb)) {
		return NULL;
	}

	if ((cb!=Py_None) && (!PyCallable_Check(cb)) ) {
           PyErr_SetString(PyExc_TypeError, "parameter must be None or callable");
           return NULL;
      }

	Py_INCREF( cb );
	TitleScreen::theTitle->addMenuItem( label, cb );

	Py_INCREF( Py_None );
	return Py_None;
}

PyObject *
py_setgamemode(PyObject *self, PyObject *args)
{
	char *s;	

	if (!PyArg_ParseTuple( args, "s", &s )) {
		return NULL;
	}
	
	if (!strcmp( s, "title" )) {
		g_mode = TitleScreen::theTitle;
	} else if (!strcmp(s, "city" )) {
		g_mode = CityMode::theCityMode;
	}

	Py_INCREF( Py_None );
	return Py_None;
}

// methods
static PyMethodDef GameMethods[] = {  
    {"conout", py_conout, METH_VARARGS, "Print string to the console."}, 

	// map stuffs
	{"newMap", py_newmap, METH_VARARGS, "Create a new map."}, 
	{"destroyMap", py_destroymap, METH_VARARGS, "Delete a map."}, 
	{"mapGet", py_mapget, METH_VARARGS, "Get map cell."}, 
	{"mapSet", py_mapset, METH_VARARGS, "Get map cell."}, 
	{"setActiveMap", py_mapsetactive, METH_VARARGS, "Set Active Map"}, 
	{"getActiveMap", py_mapgetactive, METH_VARARGS, "Get Active Map"}, 

	// game mode access
	{"setGameMode", py_setgamemode, METH_VARARGS, "Set Game Mode."}, 

	// title screen mode
	{"addMainMenuItem", py_addMainMenu, METH_VARARGS, "Add Menu Item"}, 

	// city mode

    {NULL, NULL, 0, NULL } 

};



// module init
void  initGameModule(void)
{
    (void) Py_InitModule("game", GameMethods );

	
	char buff[1024];
	sprintf( buff, "Python %s", Py_GetVersion() );
	conout( buff,makecol(CONSOLE_PROMPT_COLOR) );
}

/*
void check_py_error()
{
//	if (Py_ErrOccurred() ) {

	}
}
*/

/////////////////////////////////////////////////////////////////////////////////////////
// console stuff
/////////////////////////////////////////////////////////////////////////////////////////

void script_init( ALFONT_FONT *console_font )
{
	f_cons = console_font;

	alfont_text_mode(-1);

	conout( "Welcome to BioWar: Outbreak on Somecolonynamehere", makecol(CONSOLE_PROMPT_COLOR) );

	// Init python module
	Py_Initialize();
	initGameModule();

	// run the startup script
#if 0 // this crashes for some reason...	
	FILE *fp = fopen( "startup.py", "r" );
	if (fp) {
		PyRun_SimpleFile( fp, "startup.py" );		
	} else {
		conout( "ERROR LOADING STARTUP SCRIPT" );
	}
#endif
	PyRun_SimpleString("execfile('./scripts/startup.py')" );

	//check_py_error();

	conout( CONSOLE_PROMPT, makecol( CONSOLE_PROMPT_COLOR) );

}

void draw_console( BITMAP *targ )
{
	rectfill( targ, CONSOLE_MARG, CONSOLE_MARG,
					640 - CONSOLE_MARG*2,
					CONSOLE_MARG*2 + alfont_text_height( f_cons ) * CONSOLE_MAX_LINES + 10,
					makecol( CONSOLE_BG_COLOR ) );

	int yval = CONSOLE_MARG + 5 + alfont_text_height( f_cons );
	for (std::list<ConsoleText>::iterator ti = console_text.begin();
		 ti != console_text.end(); ti++) 
		 {
			alfont_textout( targ, f_cons, (*ti).txt.c_str(), 
						    CONSOLE_MARG + 5, yval, (*ti).color );

			yval = yval + alfont_text_height( f_cons );
		 }


}

void console_key( int k )
{
	char kk = scancode_to_ascii( k>>8 );
	char buff[10];
	
	if (k>>8 == KEY_ENTER ) {
	
		// process line
		std::string l2 = console_text.back().txt;		

		// remove prompt
		l2 = l2.substr( strlen( CONSOLE_PROMPT ),
						l2.size() - strlen( CONSOLE_PROMPT ) );
		
		// execute l2... 
		char s[1024];
		strcpy( s, l2.c_str() );
		PyRun_SimpleString( s );

		conout( CONSOLE_PROMPT, makecol(CONSOLE_PROMPT_COLOR) );
	} else if (k>>8 == KEY_BACKSPACE ) {
		// backspace		
		std::string &last_line = console_text.back().txt;
		if (last_line.size() > strlen( CONSOLE_PROMPT)) {
			last_line.erase( last_line.end()-1 );
		}

	} else {
		// tack it onto the input line
		std::string &last_line = console_text.back().txt;		

		// Im sure theres an easier way to do this but
		// i dont know it right now..
		if (key_shifts && KB_SHIFT_FLAG) {
			if (isalpha(kk)) {
				kk = toupper( kk );
			} else switch(kk) {
				case '0': kk = ')'; break;
				case '1': kk = '!'; break;
				case '2': kk = '@'; break;
				case '3': kk = '#'; break;
				case '4': kk = '$'; break;
				case '5': kk = '%'; break;
				case '6': kk = '^'; break;
				case '7': kk = '&'; break;
				case '8': kk = '*'; break;
				case '9': kk = '('; break;
				case '-': kk = '_'; break;
				case '=': kk = '+'; break;
				case '[': kk = '{'; break;
				case ']': kk = '}'; break;
				case ';': kk = ':'; break;
				case ',': kk = '<'; break;
				case '.': kk = '>'; break;
				case '/': kk = '?'; break;
				case '\\': kk = '|'; break;
				case '`': kk = '~'; break;
				case '\'': kk = '"'; break;
			}
		}

		buff[0] = kk;
		buff[1] = 0;

		last_line.append( buff );
	}
	
}

void conout( const char *s, int color )
{
	static char s1[4096];
	char *tok;

	if (strchr( s, '\n')) {
		strcpy( s1, s );
		tok = strtok( s1, "\n" );
		while (tok ) {
			console_text.push_back( ConsoleText( tok, color ) );
			tok = strtok( NULL, "\n" );
		} 
	} else {
		console_text.push_back( ConsoleText( s, color ) );
	}

	// scroll
	while (console_text.size() > CONSOLE_MAX_LINES) {
		console_text.pop_front();
	}
}


void conoutfc( int color, const char *s, ...)
{
	char *buff;
	int bsz, sz;

	va_list args;
	va_start(args, s);
	
	// make space for the new string 
	sz = strlen( s ) + 100;

	// format the string
	while (1) {
		buff = (char *)malloc( sz );
		bsz = vsnprintf( buff, sz, s, args );

		if (bsz >= 0) {
			conout( buff, color );
			free(buff);
			break;
		}
				
		// try again with twice as big until it fits
		free( buff );
		sz <<= 1;
	}

	va_end(args);	
}

void conoutf(  const char *s, ...)
{
	char *buff;
	int bsz, sz;

	va_list args;
	va_start(args, s);
	
	// make space for the new string 
	sz = strlen( s ) + 100;

	// format the string
	while (1) {
		buff = (char *)malloc( sz );
		bsz = vsnprintf( buff, sz, s, args );

		if (bsz >= 0) {
			conout( buff, makecol( CONSOLE_TEXT_COLOR) );
			free(buff);
			break;
		}
				
		// try again with twice as big until it fits
		free( buff );
		sz <<= 1;
	}

	va_end(args);	
}