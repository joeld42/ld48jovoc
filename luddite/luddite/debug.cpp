#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>

#include <dbghelp.h>
#pragma comment(linker, "/defaultlib:dbghelp.lib")

#else

// Linux 
#include <termios.h>
#include <unistd.h>

#include <execinfo.h>

#endif

#include <SDL.h>
#include <SDL_endian.h>

#include "debug.h"

using namespace DBG;

// globals;
int  DBG::g_verbose_level = DBG::kVerbose_On;
bool DBG::g_colorterm = true;

const int DBG::kVerbose_Off = 2;
const int DBG::kVerbose_On  = 5;
const int DBG::kVerbose_Dbg = 6;

#define MAX_SYMBOL_NAME (1024)

// SymGetSymFromAddr64 seem to already return undecorated names, 
//so this isn't needed. Define this if you start getting decorated names
#define UNDECORATE_NAMES (0)

/////////////////////////////////////////////////////////
void vmessage( int message_level, const char *fmt, va_list args) {
	static char *catname[8] = {
		"", "fatal: ", "error: ", "warn: ", 
		"progress: ", "info: ", "debug: ", "vdebug: "
	};
	
#ifndef WIN32
	static char *catcolor[8] = {
		"\e[0m", // reset
		"\e[1m\e[31m", // fatal - bright red
		"\e[0m\e[31m", // error - red
		"\e[1m\e[33m", // warn - bright yellow
		"\e[1m\e[37m", // info - bright white
		"\e[1m\e[36m", // progress - bright cyan
		"\e[1m\e[44m", // debug - bright blue
		"\e[1m\e[44m"  // vdebug - bright blue
	};
#else
	static WORD catcolor[8] = {
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, // reset
		FOREGROUND_RED | FOREGROUND_INTENSITY, // fatal - bright red
		FOREGROUND_RED | FOREGROUND_INTENSITY, // error - red
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, // warn - bright yellow
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, // info - bright white
		FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY, // progress - bright cyan
		FOREGROUND_BLUE | FOREGROUND_INTENSITY, // debug - bright blue
		FOREGROUND_BLUE | FOREGROUND_INTENSITY  // vdebug - bright blue
	};

	static HANDLE hStdOut = INVALID_HANDLE_VALUE;
#endif

	if (message_level <= g_verbose_level ) {

		if (g_colorterm) {			
			
#ifndef WIN32
			printf( catcolor[message_level] );
#else
			if (hStdOut==INVALID_HANDLE_VALUE) {
				hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
				if (hStdOut==INVALID_HANDLE_VALUE) {
					g_colorterm = false;
					goto message_part;
				}
			}

			SetConsoleTextAttribute( hStdOut, catcolor[message_level] ); 
#endif
		}

message_part:

		//output message
		printf(catname[message_level]);
		vprintf(fmt, args);  

		// reset color
		if (g_colorterm) {
#ifndef WIN32
			printf("\e[0m" );
#else
			// defautl??
			SetConsoleTextAttribute( hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE ); 
#endif
		}
	}	

	// non-fatal error
	if (message_level > 1) return;
	
#ifndef NDEBUG
	Assert( 0, "Fatal Error" ); // break into debugger
#else
	exit(1); // quit
#endif
}

#define def_output_fn( fname, level ) \
	void DBG::fname( const char *fmt, ... ) { \
		va_list args; \
		va_start(args,fmt);  \
		vmessage( level, fmt, args); \
		va_end(args); }

def_output_fn( fatal,    1 );
def_output_fn( error,    2 );
def_output_fn( warn,     3 );
def_output_fn( progress, 4 );
def_output_fn( info,     5 );
def_output_fn( debug,    6 );
def_output_fn( vdebug,   7 );

#undef def_output_fn

// Buffer to store stack trace text
#define STACKBUFF_SIZE (8192)
static char g_stackTraceBuff[STACKBUFF_SIZE];
static char *g_chStackBuff = NULL;

void stack_output_reset()
{
	g_chStackBuff = g_stackTraceBuff;
	*g_chStackBuff = 0;
}

// Send output to many places, and also accumulate
// it in g_stackTraceBuff for possibly copying to
// clipboard
void stack_outf( const char *fmt, ... )
{
	static char line[1024];

	va_list args; 
	va_start(args,fmt);  
	vsprintf(line, fmt, args);  

	// send to visual studio
	//OutputDebugString( (LPCWSTR)line );

	// send to console
	printf( line );

	// TODO: log file??

	// accumulate on g_stackTraceBuff, use c assert because we're
	// already inside app assert :)
	assert( (strlen(line)+ g_chStackBuff) - g_stackTraceBuff < STACKBUFF_SIZE );

	strcpy( g_chStackBuff, line );
	g_chStackBuff += strlen( line );	
	
	va_end(args);
}

#ifdef WIN32

void copyStackBuffToClipboard()
{
	if (OpenClipboard( NULL)) 
	{
		HGLOBAL hMem;
		hMem = GlobalAlloc( GHND|GMEM_DDESHARE, strlen( g_stackTraceBuff )+1 );
		if (hMem) {
			char *pMem  = (char *)GlobalLock( hMem );
			strcpy( pMem, g_stackTraceBuff );
			GlobalUnlock( hMem );
			EmptyClipboard();
			SetClipboardData( CF_TEXT, hMem );
			DBG::info( "Stack trace copied to clipboard.\n" );
		} else {
			DBG::warn( "Could not alloc clipboard memory\n" );
		}
		CloseClipboard();
	} else {
		DBG::warn( "Could not open clipboard\n" );
	}
	
}

#else

// http://cboard.cprogramming.com/faq-board/27714-faq-there-getch-conio-equivalent-linux-unix.html 
int dbgGetCh()
{
  struct termios oldt, newt;
  int ch;

  tcgetattr( STDIN_FILENO, &oldt );

  newt = oldt;
  newt.c_lflag &= ~( ICANON | ECHO );

  tcsetattr( STDIN_FILENO, TCSANOW, &newt );
  ch = getchar();
  tcsetattr( STDIN_FILENO, TCSANOW, &oldt );

  return ch;
}


#endif

// The uber-assert -- win32 style
#ifdef WIN32

bool DBG::AssertFunc( bool expr, char *desc, int line, char *file, bool *skip )
{	
	if (expr) return false; // don't break if expr is good
			
	// make sure assert will be seen
	if (g_verbose_level < 2) g_verbose_level = 2;

	// print the assert message
	stack_output_reset();
	stack_outf( "Assertion Failed: %s:%d\n", file, line );
	stack_outf( "Description: %s\n\n", desc );	 

	// Print a stack trace
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();
	SymSetOptions( SYMOPT_DEFERRED_LOADS);
    if ( SymInitialize( hProcess, 0, TRUE ) )
	{		
		
		IMAGEHLP_SYMBOL64  *pSymbol;
#if UNDECORATE_NAMES
		char undName[MAX_SYMBOL_NAME];
		char undNameFull[MAX_SYMBOL_NAME];
#endif

		CONTEXT dummyCtx; // not important on i386
		STACKFRAME64 stackFrame;
		DWORD64 offsetFromSymbol;
		DWORD_PTR    framepointer;
		DWORD_PTR    programcounter;		

		stack_outf( "Stack Trace: \n" );

		__asm {			
			mov eax, [esp + 4]
			mov [programcounter], eax			
			mov [framepointer], EBP
		}
		
		// Initialize the STACKFRAME64 structure.
		memset(&stackFrame, 0, sizeof(stackFrame));
		stackFrame.AddrPC.Offset    = programcounter;
		stackFrame.AddrPC.Mode      = AddrModeFlat;
		stackFrame.AddrFrame.Offset = framepointer;
		stackFrame.AddrFrame.Mode   = AddrModeFlat;

		// Initialize the SYMBOL64 structure		
		pSymbol = (IMAGEHLP_SYMBOL64 *)malloc( sizeof(IMAGEHLP_SYMBOL64) + MAX_SYMBOL_NAME );
		memset( pSymbol, 0, sizeof(IMAGEHLP_SYMBOL64) + MAX_SYMBOL_NAME );
		pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
		pSymbol->MaxNameLength = MAX_SYMBOL_NAME;		

		int frameNum = 0;
		//stack_outf( "\n--# FV EIP----- RetAddr- FramePtr StackPtr Symbol\n" );
		stack_outf( "\n--# FV EIP----- Symbol\n" );
		while(1) {
			if (!StackWalk64( IMAGE_FILE_MACHINE_I386, hProcess, hThread,
							  &stackFrame, 
							  &dummyCtx, // context 
							  NULL, // mem read func 
							  SymFunctionTableAccess64,
							  SymGetModuleBase64, 
							  NULL  // translateAddress
							  )) 
			{
				break;
			}
								
			// print this stack frame								
			if ( stackFrame.AddrPC.Offset == 0 )
			{
				stack_outf( "%3d %c%c %08lx %08lx %08lx %08lx (nosymbol)\n",
				frameNum, 
				stackFrame.Far? 'F': '.', 
				stackFrame.Virtual? 'V': '.',
				stackFrame.AddrPC.Offset, 
				stackFrame.AddrReturn.Offset,
				stackFrame.AddrFrame.Offset, 
				stackFrame.AddrStack.Offset );
			}
			else 
			{
				stack_outf( "%3d %c%c %08lx ", frameNum, 
					stackFrame.Far? 'F': '.', 
					stackFrame.Virtual? 'V': '.',
					stackFrame.AddrPC.Offset );

				if (!SymGetSymFromAddr64( hProcess, stackFrame.AddrPC.Offset,
					&offsetFromSymbol, pSymbol )) {
					stack_outf( "SymGetSymFromAddr(): err = %lu\n", GetLastError() );
				} else {
					stack_outf( "%s", pSymbol->Name );
					if ( offsetFromSymbol != 0 )
						stack_outf( " %+ld bytes\n", (long) offsetFromSymbol );
					else stack_outf( "\n" );
#if UNDECORATE_NAMES
					UnDecorateSymbolName( pSymbol->Name, undName, MAX_SYMBOL_NAME, UNDNAME_NAME_ONLY );
					UnDecorateSymbolName( pSymbol->Name, undNameFull, MAX_SYMBOL_NAME, UNDNAME_COMPLETE );
					
					stack_outf( "    Sig:  %s\n", undName );
					stack_outf( "    Decl: %s\n", undNameFull );
#endif

				}
			}
			
			frameNum++;

		}

		free( pSymbol );
		stack_outf("\n\n");
	}

	DBG::error("[B] Break, [S] Skip, [K] Skip Always, [Q] Quit\n" );
	DBG::error("Press [SPACE] to copy stack trace to clipboard.\n" );

	while (1) {
		int ch = _getch();	
		switch (ch) {
			case 'b':
			case 'B':
				return true; // should break
				break;
			case 's':
			case 'S':
				return false; // should skip
				break;
			case 'k':
			case 'K':
				*skip = true; // set skip always
				return false; // skip
				break;
			case 'q':
			case 'Q':
				exit(1);
				return false;
				break;
			case ' ':
				copyStackBuffToClipboard();
				break;
		}
	}

	// not reached
	return true; // should break;
}

#else

// uber-assert, linux style
bool DBG::AssertFunc( bool expr, char *desc, int line, char *file, bool *skip )
{
 	if (expr) return false; // don't break if expr is good

	// make sure assert will be seen
	if (g_verbose_level < 2) g_verbose_level = 2;

	// print the assert message
	stack_output_reset();
	stack_outf( "Assertion Failed: %s:%d\n", file, line );
	stack_outf( "Description: %s\n\n", desc );

    // print a stack trace    
    const int max_depth = 16;    
    void *trace[max_depth];
    char **messages = (char**)NULL;
    int trace_size;
    
    trace_size = backtrace( trace, max_depth );
    messages = backtrace_symbols( trace,  trace_size );

    for (int i=0; i < trace_size; i++)
    {
        stack_outf("    %s\n", messages[i] );        
    }
    stack_outf("\n\n");

	DBG::error("[B] Break, [S] Skip, [K] Skip Always, [Q] Quit\n" );

	while (1) {
		int ch = dbgGetCh();	
		switch (ch) {
			case 'b':
			case 'B':
				return true; // should break
				break;
			case 's':
			case 'S':
				return false; // should skip
				break;
			case 'k':
			case 'K':
				*skip = true; // set skip always
				return false; // skip
				break;
			case 'q':
			case 'Q':
				exit(1);
				return false;
				break;
		}
	}

	// not reached
	return true; // should break;   
}
#endif
