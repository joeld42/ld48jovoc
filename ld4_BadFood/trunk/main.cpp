#ifdef WIN32
# include <crtdbg.h>
#endif

#include <allegro.h>
#include <alfont.h>

#include <math.h>
#include <string.h>

#include <assert.h>

#include <algorithm>
#include <vector>
#include <deque>

#include <jpegdecoder.h>

// stupid msvc
//#ifdef WIN32
//#define max(a,b) ((a>b)?(a):(b))
//#define min(a,b) ((a<b)?(a):(b))
//#endif


ALFONT_FONT *gamefont, *titlefont;  

// tileset
std::vector<BITMAP*> tileset;
std::vector<BITMAP*> walltileset;


BITMAP *pukegfx;
BITMAP *discardBmp;
BITMAP *cashtiles;
BITMAP *titlebg;
BITMAP *docs;

SAMPLE *sfxGetCard, *sfxBleep, *sfxSwish, *sfxSiren,
       *sfxWinner, *sfxLoser;

MIDI *music, *titlemusic;

// levels
extern int leveldata_level1[40*30];
// level 2 is empty
extern int leveldata_level3[40*30];
extern int leveldata_level4[40*30];
// level 5 is empty
extern int leveldata_level6[40*30];
extern int leveldata_level7[40*30];
// level 8 is empty
extern int leveldata_level9[40*30];
extern int leveldata_level10[40*30];
#define MAX_LEVEL 11

// game mode
enum {
	MODE_TITLE,
	MODE_DOCS,
	MODE_GAME,
};

// direction
enum {
	DIR_UP, 
	DIR_DOWN,
	DIR_LEFT, 
	DIR_RIGHT
};

// puke sprites
enum {
	PNDX_VERT,
	PNDX_HORIZ,
	PNDX_END_N,
	PNDX_END_E,
	PNDX_END_S,
	PNDX_END_W,
	PNDX_SE,
	PNDX_SW,
	PNDX_NE,
	PNDX_NW,
	PNDX_VERT_ALT,
	PNDX_HORIZ_ALT,
};

bool editor = false; // in edit mode??
BITMAP *editCursor = NULL;
int lasttile, lastwall;
int markX, markY;

const char *menu[] = {
	"Normal",
	"Expert",
	"Insane",
	"Instructions",
	"Quit",
	NULL
};
int menuNdx = 0;
int docPage = 0;

const char *docText[] = {
	"Chuck and his friend Ralph\n"
	"have made a very unwise bet.\n"
	" \n"
	"They bet on who can get the\n"
	"most ill between them. But\n"
	"they aren't just going for\n"
	"the worst disease...they are\n"
	"after the most unpleasent\n"
	"*combination* of diseases.",

	"Here is a reminder of poker\n"
	"hands, from best to worst:\n \n"
	"    Hand                       Example\n"
	" Royal Flush\n"
	" Straight Flush\n"
	" Four of a Kind\n"
	" Full House\n"
	" Flush\n"
	" Straight\n"
	" Three of a Kind\n"
	" Two Pair\n"
	" Pair\n",

	NULL
};

#define DISCARD_TIME (20)

struct MapSquare
{
	int tile;
	int wall; // 0= no wall, N = wall tile n-1
	bool puke;
};

struct Pos {
	Pos() { x = 0; y = 0; } 
	Pos( int xx, int yy ) { x = xx; y = yy; }
	int x, y;
};

class Sprite {
public:
	Sprite( Sprite &other );
	Sprite( char *filename );

	void draw( BITMAP *target );
	void frame();	

	int x,  y;
	int f, frate, fcnt; // rate is in # frames per sprite frame

	std::vector<BITMAP*> frames;
};

// suits
enum {
	GREEN,
	BROWN,
	PURPLE,
	RED
};



class Card {
public:
	Card( int s, int v ) {
		suit = s;
		value = v;
	}

	Card() {}

	int suit, value; // values are 1..14 (ace==14)	
	bool used;

	static BITMAP *cardsm, *cardGreen, *cardBrown, *cardRed, *cardPurple;
	static void initCardBmps();

	void drawSmall( BITMAP *targ, int x, int y, bool faceup = true );
	void drawBig( BITMAP *targ, int x, int y );

	// returns the number as a string
	char *cardStr();
	std::string cardStrLong();

	bool operator < (const Card &other );
};

BITMAP *Card::cardsm = NULL;
BITMAP *Card::cardGreen = NULL;
BITMAP *Card::cardBrown = NULL;
BITMAP *Card::cardRed = NULL;
BITMAP *Card::cardPurple = NULL;


// this is a card dealt to the table
class TableCard {
public:
	Card card;
	int x, y; // map coords
	int age;
};

// represents a raise
class TableCoin {
public:
	int value; // index
	int x,y;
	int age;
};

class PokerHand
{
public:
	PokerHand();

	int getValue();

	std::vector<Card> card;
	int value;
	std::string name;
	std::string details;
};

// phases
enum {
	GAME_PLAY,	
	GAME_FOLD, 
	GAME_CALL
};


class Game {
public:
	int level;

	Game();

	void initLevel( int level );
	void initNextLevel();

	void newGame();

	void draw( BITMAP *targ );
	void drawCall( BITMAP *targ );

	void update();	

	void moveTo( int x, int y );

	void Call();

	MapSquare map[40][30];
	int dir,  nextDir;

	Sprite *plr, *plrPuke;
	Sprite *cleaner;
	Pos playerPos;  // position on map
	
	Pos cleanerGoal;

	int pukeLen;
	int discardCursorPos;
	int discardTimeLeft;

	std::deque<Pos> path;	
	
	// game stuff
	std::deque<Card> deck;
	void shuffle();
	void deal();
	
	std::vector<TableCard> table;
	std::vector<Card> shared, hand, ophand;
	int updateTableCard;

	std::vector<TableCoin> coin;
	int updateTableCoin;

	int phase;

	PokerHand bestPlrHand, bestDlrHand;

	// Best Poker Hand Calculating stuff
	PokerHand bestHand;	
	std::vector<Card> pool;
	void calcBestHand(); // from pool
	void makeDealerHand( int difficulty );

	int cash, // your money
		pot,  // the pot
		fakecash; // animated money 

	int updaterate;

	int callAnim, callAnimX;

	// animated message
	std::string msg;
	int messageAge;
	void message( char *);

protected:
	void doCalcBestHand( int ndx, int numUsed, bool dlrMode = false );

};
Game *theGame = NULL;




/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
int irand( int range ) 
{
	return (int)( ((float)rand()/(float)RAND_MAX) * range );
}


void draw_shadow_text( BITMAP *targ, ALFONT_FONT *thefont, 
					  const char *message, int x, int y, int col, int offs=3) 
{
	alfont_textout( targ, thefont, message, x+offs,y+offs, makecol( 10, 50, 10)  );
	alfont_textout( targ, thefont,  message, x, y, col );
}

void draw_shadow_text_centre( BITMAP *targ, ALFONT_FONT *thefont, 
					  const char *message, int x, int y, int col, int offs=3) 
{
	alfont_textout_centre( targ, thefont, message, x+offs,y+offs, makecol( 10, 50, 10)  );
	alfont_textout_centre( targ, thefont,  message, x, y, col );
}

PokerHand::PokerHand()
{
	value = -1;
}

BITMAP *load_jpeg( char *filename ) 
{
	int w,h;
	BITMAP *img;

	Pjpeg_decoder_file_stream inputStream = new jpeg_decoder_file_stream();	
	inputStream->open(filename);
	Pjpeg_decoder pd = new jpeg_decoder(inputStream, 0);
	w = pd->get_width();
	h = pd->get_height();
	assert(pd->get_num_components() == 3);
	unsigned char *pbuf = (unsigned char *)malloc(pd->get_width() * 3);
	
	img = create_bitmap( w, h );

	pd->begin();

	int lines_decoded = 0;
	unsigned y = 0;	
	for ( ; ; y++)
	{
		void *Pscan_line_ofs;
		uint scan_line_len;

		if (pd->decode(&Pscan_line_ofs, &scan_line_len)) {			
		  break;
		}		

		lines_decoded++;

		int srcBpp = pd->get_bytes_per_pixel();
		assert(srcBpp == 4 || srcBpp == 3);
				
		unsigned char* scanLine = (unsigned char*)Pscan_line_ofs;
		for (int x = 0; x < pd->get_width(); x ++)
		{
			putpixel( img, x, y, makecol( scanLine[x*srcBpp],
										  scanLine[x*srcBpp+1],
										  scanLine[x*srcBpp+2] ) );			
		}
	}

	free(pbuf);
	delete pd;
	delete inputStream;

	return img;

}

// TODO:
//  -- tiebreaks are limited to only the high card currently
int PokerHand::getValue()
{
	// don't getValue an incomplete hand
	assert( card.size() == 5);	


	if (value!=-1) {
		return value;
	}

	name = "error naming hand";
	
	bool straight, flush, pair, three, four, twopair;	
	Card hicard, threecard, fourcard, paircard, twopaircard;


	// is it a flush?
	flush = true;
	for (int i=1; i < card.size(); i++) {
		if (card[i].suit != card[0].suit) flush=false;
	}
	
	// is it a straight?
	std::sort( card.begin(), card.end() );
	straight = true;
	bool hasAce = false;
	for (i=1; i < card.size(); i++) {
		if (card[i].value==14) hasAce = true;
		if (card[i].value != card[0].value + i) straight=false;
	}

	if ((hasAce) && (!straight)) {
		// check for ace-low straight
		for (i=0; i < card.size(); i++) {
			if (card[i].value==14) card[i].value = 1;
		}

		straight = true;
		std::sort( card.begin(), card.end() );
		for (i=1; i < card.size(); i++) {		
			if (card[i].value != card[0].value + i) straight=false;
		}
		
		// change aces back if not a straight
		if (!straight) {
			for (i=0; i < card.size(); i++) {
				if (card[i].value==1) card[i].value = 14;
			}
		}
		std::sort( card.begin(), card.end() );
	}

	// count card groups
	pair = false;
	three = false;
	four = false;
	twopair = false;
	hicard.value = 0;
	for (i=0; i < card.size(); i++) {
		if (card[i].value > hicard.value) {
			hicard = card[i];
		}

		int count = 0;
		for (int j=0; j < card.size(); j++) {
			if (card[j].value == card[i].value) {
				count++;
			}
		}

		if ((count==2) && (pair) && (card[i].value!=paircard.value)) {
			twopair = true;
			twopaircard = card[i];			
		} else if (count==2) {
			pair = true;
			paircard = card[i];			
		} else if (count==3) {
			three = true;
			threecard = card[i];			
		} else if (count==4) {
			four = true;
			fourcard = card[i];			
		} 
	}	

	// now combine this into a magic number
	char buff[1024];
	if ((flush) && (straight) && (card[0].value==10)) {
		name = "Royal Flush";
		value = 10000;
	} else if (flush && straight) {
		name = "Straight Flush";
		sprintf( buff, "%s High", card[4].cardStrLong().c_str() );
		details = buff;
		value = 9000 + card[4].value;
	
	} else if (four) {
		name = "Four of a Kind";
		sprintf( buff, "%ss", fourcard.cardStrLong().c_str() );
		details = buff;
		value = 8000 + fourcard.value;
	} else if ((three) && (pair)) {
		name = "Full House";
		sprintf( buff, "%ss over %ss", 
				threecard.cardStrLong().c_str(), paircard.cardStrLong().c_str() );
		details = buff;
		value = 7000 + threecard.value * 20 + paircard.value;
	} else if (flush) {
		name = "Flush";
		sprintf( buff, "%s High", hicard.cardStrLong().c_str() );
		details = buff;
		value = 6000 + hicard.value;		
	} else if (straight) {
		name = "Straight";
		sprintf( buff, "%s High", hicard.cardStrLong().c_str() );
		details = buff;
		value = 5000 + hicard.value;		
	} else if (three) {
		name = "Three of a Kind";
		sprintf( buff, "%ss", threecard.cardStrLong().c_str() );
		details = buff;
		value = 4000 + threecard.value;
	} else if (twopair) {
		name = "Two Pair";
		sprintf( buff, "%ss and %ss", 
			paircard.cardStrLong().c_str(), twopaircard.cardStrLong().c_str() );
		details = buff;
		value = 3000 + max(twopaircard.value, paircard.value) * 20 + min(twopaircard.value, paircard.value);
	} else if (pair) {
		sprintf( buff, "Pair of %ss", paircard.cardStrLong().c_str() );
		name = buff;
		details = "";
		value = 2000 + paircard.value;
	} else {
		sprintf( buff, "High Card %s", hicard.cardStrLong().c_str() );		
		name = buff;
		details = "";
		value = hicard.value;
	}

	return value;
}

// from pool
void Game::calcBestHand()
{
	bestHand.card.erase( bestHand.card.begin(), bestHand.card.end() );

	for (int i=0; i < pool.size(); i++) {
		pool[i].used = false;
	}
	
	doCalcBestHand( pool.size()-1, 0 );
}

// Dealer's hand
void Game::makeDealerHand( int difficulty )
{
	assert( difficulty >=3 );

	bestHand.card.erase( bestHand.card.begin(), bestHand.card.end() );

	// shared pool cards (these must be first because of dlrMode)
	pool.erase( pool.begin(), pool.end() );
	for (int i=0; i < shared.size(); i++) {
		pool.push_back( shared[i] );
	}

	// dealer cards
	std::vector<Card> leftover;
	for (i=0; i < difficulty; i++) {
		pool.push_back( deck.front() );
		leftover.push_back( deck.front() );
		deck.pop_front();
	}

	for ( i=0; i < pool.size(); i++) {
		pool[i].used = false;
	}
	
	doCalcBestHand( pool.size()-1, 0, true );
	
	_RPT1( _CRT_WARN, "Besthand size is %d\n", bestHand.card.size() );
	_RPT2( _CRT_WARN, "Besthand is %s %s\n", bestHand.name.c_str(), bestHand.details.c_str() );	
	
	for (i=0; i < bestHand.card.size(); i++) {

		_RPT2( _CRT_WARN, "Looking at card %d-%d\n", 
					bestHand.card[i].value, bestHand.card[i].suit ); 

		// see if the card came from shared or from
		// the dealer's hand
		bool inShared = false;
		for (int j=0; j < shared.size(); j++) {
			if ( (shared[j].value == bestHand.card[i].value) &&
				(shared[j].suit == bestHand.card[i].suit) ) {

				inShared = true;
				break;
			}
		}

		// if it's not from the common pool, put it in the dealer's hand
		if (!inShared) {			
			ophand.push_back( bestHand.card[i] );

			_RPT2( _CRT_WARN, "Dealer card %d-%d is not from shared\n", 
					bestHand.card[i].value, bestHand.card[i].suit ); 

			// and take it out of leftover
			for (int j=0; j < leftover.size(); j++) {
				if ( (leftover[j].value == bestHand.card[i].value) &&
					 (leftover[j].suit == bestHand.card[i].suit) ) {
					
					leftover[j] = leftover.back();
					leftover.pop_back();
				}
			}
		}
	}

	// If the dealer's hand needed less than three cards from
	// the deck, fill it up with leftovers
	while (ophand.size() < 3) {
		ophand.push_back( leftover.back() );
		leftover.pop_back();
	}

	// now put the leftovers on the bottom of the deck
	// TODO: should probably reshuffle
	for (i=0; i < leftover.size(); i++) {
		deck.push_back( leftover[i] );
	}
}

void Game::doCalcBestHand( int ndx, int numUsed, bool dlrMode )
{

	if (numUsed==5) {

		//_RPT0( _CRT_WARN, "Looking at hand: " );
		// evaluate this hand
		PokerHand curr;
		int dlrCount=0;
		for (int i=0; i < pool.size(); i++) {
			if (pool[i].used) {
				curr.card.push_back( pool[i] );
				
				// count how many non-shared cards are used
				if (i >= 4) {
					dlrCount++;
				}
			}
		}

		// dealer can only use 3 of the non-shared pool cards
		if ( (dlrMode) && (dlrCount > 3) ){
			_RPT1( _CRT_WARN, "Dealer hand count %d\n", dlrCount );
			return;
		}

		if ( (bestHand.card.size()==0) ||
		 	(curr.getValue() > bestHand.getValue() ) ) {
			bestHand = curr;
			//_RPT0( _CRT_WARN, "***" );
		}
		//_RPT0(_CRT_WARN,  "\n" );

	} else if (ndx>=0) {
		pool[ndx].used = true;
		doCalcBestHand( ndx-1, numUsed+1, dlrMode );
		pool[ndx].used = false;
		doCalcBestHand( ndx-1, numUsed, dlrMode
			);
	}
}

Sprite::Sprite( Sprite &other )
{
	frames = other.frames;
	x = other.x;
	y = other.y;
	frate = other.frate;
	f = 0; fcnt = 0;
}

void Sprite::draw( BITMAP *target )
{
	draw_sprite( target, frames[f], x, y );
}

void Sprite::frame()
{
	fcnt++;
	if (fcnt == frate) {
		fcnt = 0;
		f++;
		if (f==frames.size()) f=0;
	}
}

Sprite::Sprite( char *filename )
{
	BITMAP *rawFrames = load_bitmap( filename, NULL );

	for (int i = 0; i < rawFrames->w / rawFrames->h; i++ ) {
		BITMAP *frame = create_bitmap( rawFrames->h, rawFrames->h );
		blit( rawFrames, frame, i*rawFrames->h, 0,   0,0, rawFrames->h, rawFrames->h );

		frames.push_back( frame );
	}
	destroy_bitmap( rawFrames );

	x=0; y=0; f =0;
	frate = 10; fcnt = 0;
}

void Card::initCardBmps()
{
	cardsm = load_bitmap( "smallcards.bmp", NULL );
	cardGreen  = load_bitmap( "bigcard-green.bmp", NULL );
	cardBrown  = load_bitmap( "bigcard-brown.bmp", NULL );
	cardPurple = load_bitmap( "bigcard-purple.bmp", NULL );
	cardRed    = load_bitmap( "bigcard-red.bmp", NULL );
}

char *Card::cardStr()
{
	static char s[5];
	assert( (value > 0) && (value <= 14) );

	if ((value<=10)&&(value>1)) {
		sprintf( s, "%d", value );
	} else {
		switch(value) {
		case 11: strcpy( s, "J" ); break;
		case 12: strcpy( s, "Q" ); break;
		case 13: strcpy( s, "K" ); break;
		case 14: strcpy( s, "A" ); break;
		case  1: strcpy( s, "A" ); break;
		}
	}

	

	return s;
}

std::string Card::cardStrLong()
{
	static char s[30];
	assert( (value > 0) && (value <= 14) );

	switch(value) {
		case 2: strcpy( s, "Two" ); break;
		case 3: strcpy( s, "Three" ); break;
		case 4: strcpy( s, "Four" ); break;
		case 5: strcpy( s, "Five" ); break;
		case 6: strcpy( s, "Six" ); break;
		case 7: strcpy( s, "Seven" ); break;
		case 8: strcpy( s, "Eight" ); break;
		case 9: strcpy( s, "Nine" ); break;
		case 10: strcpy( s, "Ten" ); break;
		case 11: strcpy( s, "Jack" ); break;
		case 12: strcpy( s, "Queen" ); break;
		case 13: strcpy( s, "King" ); break;
		case 14: strcpy( s, "Ace" ); break;		
	}

	return std::string(s);
}

void Card::drawSmall( BITMAP *targ, int x, int y, bool faceup )
{
	int ndx = value-1;
	if (ndx==13) ndx = 0; // ace

	if (!faceup) ndx = 13;
	masked_blit( cardsm, targ, ndx * 16, 0, x, y, 16, 16 );

	ndx = suit + 15;
	if (!faceup) ndx = 14;

	masked_blit( cardsm, targ, ndx * 16, 0, x, y+16, 16, 16 );
}

void Card::drawBig( BITMAP *targ, int x, int y )
{
	BITMAP *c;	

	assert( (suit >= 0) && (suit < 4) );
	assert( (value > 0) && (value <= 14) );
	
	switch (suit) {
		case GREEN: c = cardGreen; break;
		case BROWN: c = cardBrown; break;
		case PURPLE: c = cardPurple; break;
		case RED: c = cardRed; break;
	}

	draw_sprite( targ, c, x, y );

	// draw the number
	alfont_set_font_size( gamefont, 24 );
	alfont_textout_centre( targ, gamefont, cardStr(), x + 24, y + 4, makecol( 83, 101, 240) );

}

bool 
Card::operator < (const Card &other )
{	
	return value < other.value;
}

Game::Game() 
{
	theGame = this;

	plr = new Sprite( "player.bmp" );
	plrPuke = new Sprite( "pukeanim.bmp" );
	plrPuke->frate = 4;

	cleaner = new Sprite( "cleaner.bmp" );
	cleaner->frate = 8;
	cleaner->x = 640;
	cleaner->y = 200;

	newGame();
	
}

void Game::newGame()
{
	messageAge = 0;
	cash = 1100;

	level = 1;
	initLevel( level );
	dir = DIR_UP;
	nextDir = dir;

	phase = GAME_PLAY;
	updateTableCard = 15;
	updateTableCoin = 5;
}

void Game::message( char *the_msg ) 
{
	msg = std::string(the_msg);
	messageAge = 120;
}

void Game::initNextLevel()
{
	int l = level+1;
	if (l == MAX_LEVEL) l = 1;
	initLevel( l );

	
}

void Game::initLevel( int Level ) {

	level = Level;

	// init floor
	for (int j=0; j < 30; j++) {
		for (int i=0; i < 40; i++) {
			map[i][j].puke = false;			
			map[i][j].wall = 0;

			if (level==1) {
				int ndx = (j*40) + i;
				map[i][j].wall = leveldata_level1[ndx] >> 8;
				map[i][j].tile = leveldata_level1[ndx] & 0xff;			
			} else if (level==3) {
				int ndx = (j*40) + i;
				map[i][j].wall = leveldata_level3[ndx] >> 8;
				map[i][j].tile = leveldata_level3[ndx] & 0xff;
			} else if (level==4) {
				int ndx = (j*40) + i;
				map[i][j].wall = leveldata_level4[ndx] >> 8;
				map[i][j].tile = leveldata_level4[ndx] & 0xff;
			} else if (level==6) {
				int ndx = (j*40) + i;
				map[i][j].wall = leveldata_level6[ndx] >> 8;
				map[i][j].tile = leveldata_level6[ndx] & 0xff;
			} else if (level==7) {
				int ndx = (j*40) + i;
				map[i][j].wall = leveldata_level7[ndx] >> 8;
				map[i][j].tile = leveldata_level7[ndx] & 0xff;
			} else if (level==9) {
				int ndx = (j*40) + i;
				map[i][j].wall = leveldata_level9[ndx] >> 8;
				map[i][j].tile = leveldata_level9[ndx] & 0xff;
			} else if (level==10) {
				int ndx = (j*40) + i;
				map[i][j].wall = leveldata_level10[ndx] >> 8;
				map[i][j].tile = leveldata_level10[ndx] & 0xff;
			} else {
				// Use default (empty) level for 
				// level 2, 5 and 8

				if ((i==0)||(j==0)||(i==39)||(j==29)) {
					map[i][j].tile = 0; 
				} else {
					if ( i & 1) { 
						map[i][j].tile = 18; 
					} else {
						map[i][j].tile = 13; 
					}
				}
			}

			//
			
		}
	}



	// init path
	pukeLen = 20;

	path.erase( path.begin(), path.end() );	
	moveTo( 20, 15 );

	cleaner->x = 640;
	cleaner->y = 200;

	// init card stuff	
	bestPlrHand = PokerHand();
	bestDlrHand = PokerHand();
	table.erase( table.begin(), table.end() );
	coin.erase( coin.begin(), coin.end() );

	deal();

	// really shouldn't reach into the menu here but...
	int diff;
	if (menuNdx==0){
		diff = 5;
		updaterate = 10;
	} else if (menuNdx==1) {
		diff = 6;
		updaterate = 8;
	} else if (menuNdx==2) {
		diff = 7;
		updaterate = 5;
	}

	makeDealerHand( diff );


	// Ante Up
	if (cash >=  100) {
		cash -= 100;
		pot = 200;
		message( "Ante up $100" );
	} else {
		pot = cash * 2;
		cash = 0;
		message("Can't afford ante! You bet it all!");
	}
}

void Game::moveTo( int x, int y )
{
	// check for collisions
	if ((x <0) || (y < 0) ||
		(x >= 40) || (y >= 30) ||
		(map[x][y].wall != 0) ||
		(map[x][y].puke != 0) ) {
		phase = GAME_FOLD;
		return;
	}

	// move chuck
	playerPos.x = x;
	playerPos.y = y;
	path.push_back( Pos(x,y) );
	map[x][y].puke = true;

	plr->x = x * 16 - 24;
	plr->y = y * 16 - 40;

	plrPuke->x = x * 16 - 24;
	plrPuke->y = y * 16 - 40;	


	// clean up old puke
	if (path.size() > pukeLen) {
		map[path.front().x][path.front().y].puke = false;
		path.pop_front();
	}
	cleanerGoal.x = path[0].x * 16 - 24;
	cleanerGoal.y = path[0].y * 16 - 40;
}

void Game::update( ) 
{
	int x,y;
	x = playerPos.x;
	y = playerPos.y;

	dir = nextDir;

	switch (dir) {
	case DIR_UP:
		moveTo( x, y-1 );
		break;
	case DIR_DOWN:
		moveTo( x, y+1 );
		break;
	case DIR_LEFT:
		moveTo( x-1, y );
		break;
	case DIR_RIGHT:
		moveTo( x+1, y );
		break;
	}

	// pick up cards
	for (int i=0; i < table.size(); i++) {
		if ((table[i].x == playerPos.x) &&
			((table[i].y == playerPos.y)||
			(table[i].y == playerPos.y-1)) ) {

			hand.push_back( table[i].card );
			table[i] = table.back();
			table.pop_back();

			// in case we need t
			discardCursorPos = hand.size() - 1;
			theGame->discardTimeLeft = DISCARD_TIME;

			// make the puke trail longer
			pukeLen += 10 + irand(5);

			// play 
			play_sample(sfxGetCard, 255, 128, 900 + irand(200), FALSE );

			if (hand.size() > 3) {
				// going to discard
				play_sample(sfxSiren, 255, 128, 1000, TRUE );
			}
			break;
		}

	}

	// pick up coins
	for (i=0; i < coin.size(); i++) {
		if ( (coin[i].x == playerPos.x) &&
		 	 (coin[i].y == playerPos.y)) {
			int raiseAmt;
			char raiseMsg[100];

			switch (coin[i].value) {
			case 0: raiseAmt = 10; break;
			case 1: raiseAmt = 50; break;
			case 2: raiseAmt = 100; break;
			case 3: raiseAmt = 500; break;
			}

			if (raiseAmt > cash) {
				raiseAmt = cash;
				sprintf( raiseMsg, "All In!! (raising $%d)", cash );
				play_sample(sfxBleep, 255, 128, 800, FALSE );
				play_sample(sfxBleep, 255, 128, 900, FALSE );
				play_sample(sfxBleep, 255, 128, 1000, FALSE );
			} else {
				sprintf( raiseMsg, "You raise $%d", raiseAmt );
				play_sample(sfxBleep, 255, 128, 900 + irand(200), FALSE );
			}

			message( raiseMsg );
			pot += raiseAmt * 2;
			cash -= raiseAmt;

			// clear the coin
			if (cash==0) {
				coin.erase( coin.begin(), coin.end() );
				break;
			} else {
				coin[i] = coin.back();
				coin.pop_back();
			}
		}
	}

	// deal new cards
	if ((table.size() < 5) && (updateTableCard == 0)) {
		
		updateTableCard = irand( 10 ) + 20;

		TableCard tc;
		tc.card = deck[0];
		deck.pop_front();

		// handle when deck runs out of cards
		if (deck.size() == 0) {
			// call (or fold if player hasn't picked up three yet..)
			if (hand.size() < 3) {
				phase = GAME_FOLD;
			} else {				
				Call();
			}
		}

		// make a new card
		tc.age = 0;
		bool hitsCoin = false;
		do {
			tc.x = irand(40);
			tc.y = irand(29);

			hitsCoin = false;
			for (int j=0; j < coin.size(); j++) {
				if ( (coin[j].x==tc.x) &&
					 ((coin[j].y==tc.y) ||
					  (coin[j].y==tc.y+1)) ) {
					hitsCoin = true;
					break;
				}
			}
		} while (((tc.x<15)&&(tc.y<5)) ||
				 ((tc.x<15)&&(tc.y>25)) ||
				 (map[tc.x][tc.y].wall) ||
				 (map[tc.x][tc.y+1].wall)||
				 (hitsCoin) );

		table.push_back(tc);
	
	} else {
		if (updateTableCard > 0) {
			updateTableCard--;
		}
	}

	// put out new coins
	if ((cash > 0) && (coin.size() < 5) && (updateTableCoin == 0)) {		
		updateTableCoin = irand( 10 ) + 15;

		TableCoin c;
		c.value = irand( 4 );		
		c.age = 0;
		bool hitsCard = false;
		do {
			c.x = irand(40);
			c.y = irand(30);

			hitsCard = false;
			for (int j=0; j < table.size(); j++) {
				if ( (table[j].x==c.x) &&
					 ((table[j].y==c.y) ||
					  (table[j].y+1==c.y)) ) {
					hitsCard = true;
					break;
				}
			}

		} while (((c.x<15)&&(c.y<5)) ||
				 ((c.x<15)&&(c.y>25)) ||
				 (map[c.x][c.y].wall) ||				 
				 (hitsCard) );

		coin.push_back( c );

	} else {
		if (updateTableCoin> 0) {
			updateTableCoin--;
		}
	}
}

void Game::draw( BITMAP *targ ) {


	if (phase == GAME_CALL) {
		drawCall( targ );
		return;
	}

	// draw floor
	for (int j=0; j < 30; j++) {
		for (int i=0; i < 40; i++) {
			int tndx = map[i][j].tile;
			assert (tndx >= 0 );
			assert (tndx < tileset.size() );

			blit( tileset[tndx], targ, 0, 0, i*16, j*16, 16, 16 );
		}
	}

	// draw walls
	for (j=0; j < 30; j++) {
		for (int i=0; i < 40; i++) {
			int tndx = map[i][j].wall;
			if (tndx > 0) {
				masked_blit( walltileset[tndx-1], targ, 0, 0, i*16, j*16, 16, 16 );
			}
		}
	}

	if (editor) return;

	// draw puke
	for (int i=0; i < path.size(); i++) {
		int x = path[i].x, y = path[i].y;
		int pndx = 0;

		if ((i==0) || (i== path.size()-1)) {
			
			// end piece
			int x, y;
			if (path.size() >=2) {
				if (i==0) {
					x = path[i+1].x;
					y = path[i+1].y;
				} else {
					x = path[i-1].x;
					y = path[i-1].y;
				}

				if (x == path[i].x+1) {
					pndx = PNDX_END_W;
				} else if (x == path[i].x-1) {
					pndx = PNDX_END_E;
				}  else if (y == path[i].y-1) {
					pndx = PNDX_END_S;
				}  else if (y == path[i].y+1) {
					pndx = PNDX_END_N;
				}

			} else {
				pndx = PNDX_END_S;
			}

		} else {
			// middle piece
			int n=0,e=0,w=0,s=0;

			if ((path[i-1].x==path[i].x+1) || (path[i+1].x==path[i].x+1) ) {
				e = 1;
			}

			if ((path[i-1].x==path[i].x-1) || (path[i+1].x==path[i].x-1) ) {
				w=1;
			}

			if ((path[i-1].y==path[i].y-1) || (path[i+1].y==path[i].y-1) ) {
				n=1;
			}

			if ((path[i-1].y==path[i].y+1) || (path[i+1].y==path[i].y+1) ) {
				s=1;
			}

			if (n && s) {
				pndx = PNDX_VERT;
				if ( ((path[i].x + path[i].y)% 7)==0 ) {
					pndx = PNDX_VERT_ALT;
				}
			} else if (e &&w) {
				pndx = PNDX_HORIZ;
				if ( ((path[i].x + path[i].y)% 5)==0 ) {
					pndx = PNDX_HORIZ_ALT;
				}
			} else if (n && e) {
				pndx = PNDX_NE;
			} else if (n && w) {
				pndx = PNDX_NW;
			} else if (s && e) {
				pndx = PNDX_SE;
			} else if (s && w) {
				pndx = PNDX_SW;
			}
		}

		masked_blit( pukegfx, targ, pndx*16, 0, x*16, y*16, 16, 16 );
	}

	// Draw player
	plr->draw( targ );
	plr->frame();

	plrPuke->draw( targ );
	plrPuke->frame();

	cleaner->draw( targ );
	cleaner->frame();

	// update cleaner pos
	if (cleaner->x > cleanerGoal.x) cleaner->x -= 2; 
	if (cleaner->x < cleanerGoal.x) cleaner->x += 2; 
	if (cleaner->y > cleanerGoal.y) cleaner->y -= 2; 
	if (cleaner->y < cleanerGoal.y) cleaner->y += 2; 

	// draw shared hand
	for ( i = 0; i < 4; i++) {
		shared[i].drawBig( targ, 10 + i*55, 10 );
	}

	// draw player's hand
	for ( i =0; i < 4; i++) {
		if (i < hand.size()) {
			hand[i].drawBig( targ, 10+i*55, 410 );
		} else if (i < 3) {
			for (j=0; j < 5; j++) {
				int xx = 10+i*55;
				int yy = 410;
				
				rect( targ, xx+j, yy+j, xx + 48 - j, 410 + 64 - j, makecol( 255-(j*40), 255-(j*40), 255 ) );
			}
			
		}
	}


	// draw Table cards
	std::vector<TableCard> keepOnTable;
	for ( i=0; i < table.size(); i++) {

		//table[i].card.drawBig( targ, table[i].x*16, table[i].y*16 + 16 );

		table[i].card.drawSmall( targ, 
			table[i].x*16, table[i].y*16, !(table[i].age < 70*5) );

		// don't age during discard phase
		if ((hand.size()>3) || (phase==GAME_FOLD)) continue;
		table[i].age ++;

		if (table[i].age < 250*5) {
			keepOnTable.push_back( table[i] );
		}
	}

	if (hand.size()<=3) {
		table = keepOnTable;
	}

	// draw Coins
	std::vector<TableCoin> keepCoin;
	for ( i=0; i < coin.size(); i++) {		

		//table[i].card.drawSmall( targ, 
		//	table[i].x*16, table[i].y*16, !(table[i].age < 70*5) );
		masked_blit( cashtiles, targ, coin[i].value*16, 0, coin[i].x*16, coin[i].y*16, 16, 16 );
					

		// don't age during discard phase
		if ((hand.size()>3) || (phase==GAME_FOLD)) continue;
		coin[i].age ++;

		if (coin[i].age < 200*5) {
			keepCoin.push_back( coin[i] );
		}
	}

	if (hand.size()<=3) {
		coin = keepCoin;
	}
	
	// draw discard cursor
	if (hand.size() > 3) {		
		char buff[10];
		sprintf( buff, "%d", discardTimeLeft );
		alfont_set_font_size( gamefont, 20 );
		draw_shadow_text( targ, gamefont, buff, 15 + discardCursorPos*55, 360, makecol(107,200,35) );
		draw_sprite( targ, discardBmp, 8 + discardCursorPos*55, 380 );
	}


	// Draw titles
	if (phase == GAME_FOLD) {
		alfont_set_font_size( gamefont, 26 );

		if (deck.size()==0) {
			alfont_textout_centre( targ, gamefont, 
						"You're supposed to PICK UP the cards, dumbass!",  320, 250, makecol(100, 255, 100) );
		} else {
			alfont_textout_centre( targ, gamefont, 
						"You stepped in it!",  320, 250, makecol(100, 255, 100) );
		}

		alfont_set_font_size( titlefont, 56 );
		
		alfont_textout_centre( targ, titlefont, "Fold",  320, 200-5, makecol(50, 100, 50) );
		alfont_textout_centre( targ, titlefont, "Fold",  320, 200+5, makecol(50, 100, 50) );
		alfont_textout_centre( targ, titlefont, "Fold",  320, 200, makecol(100, 255, 100) );

	} else if (hand.size()==3) {
		alfont_set_font_size( gamefont, 20 );		
		
		draw_shadow_text( targ, gamefont, "Press ENTER to call!", 400, 450, makecol( 100, 255,255) );		
	}

	char money[100];
	alfont_set_font_size( gamefont, 20 );		
	sprintf( money, "Cash $%d", cash );
	draw_shadow_text( targ, gamefont, money, 280, 20, makecol( 255,255,100) );
	sprintf( money, "Pot $%d", pot );
	draw_shadow_text( targ, gamefont, money, 500, 20, makecol( 255,255,100) );

	if (messageAge) {
		draw_shadow_text( targ, gamefont, msg.c_str(), 250, 45, makecol( 135+messageAge,135+messageAge,messageAge) );
		messageAge--;
	}

	if (deck.size() <= 10) {
		char buff[100];
		sprintf(buff, "Cards Left: %d\n", deck.size() );
		alfont_set_font_size( gamefont, 20 );		
		draw_shadow_text( targ, gamefont, buff, 400, 430, makecol( 255,0, 0) );		
	}
}

void Game::Call()
{
	phase = GAME_CALL;
	callAnim = 0;
	callAnimX = 640;	
}

void Game::drawCall( BITMAP *targ ) {
	blit( titlebg, targ, 0,0,0,0, 640, 480 );		

	alfont_set_font_size( gamefont, 20 );
	//alfont_textout( targ, gamefont, "Shared Cards", 20, 350, makecol( 100, 255, 255 ) );
	draw_shadow_text( targ, gamefont, "Shared Cards", 20, 370, makecol( 100, 255, 255 ) );
	
	for (int i=0; i < 4; i++) {
		shared[i].drawSmall( targ, 20 + i*20, 395, true );
	}

	draw_shadow_text( targ, gamefont, "Your Hand", 260, 370, makecol( 100, 255, 255 ) );
	
	for ( i=0; i < hand.size(); i++) {
		hand[i].drawSmall( targ, 260 + i*20, 395, true );
	}

	draw_shadow_text( targ, gamefont, "Dealer", 440, 370, makecol( 100, 255, 255 ) );
	
	for ( i=0; i < ophand.size(); i++) {
		ophand[i].drawSmall( targ, 440 + i*20, 395, true );
	}

	// figure out the hands
	if (bestPlrHand.card.size()==0) {		

		pool.erase( pool.begin(), pool.end() );
		for ( i=0; i < 3; i++) {
			pool.push_back( hand[i] );
		}
		for ( i=0; i < 4; i++) {
			pool.push_back( shared[i] );
		}
		
		calcBestHand();
		bestPlrHand = bestHand;

		// now do dealer's hand
		pool.erase( pool.begin(), pool.end() );
		for ( i=0; i < 3; i++) {
			pool.push_back( ophand[i] );
		}
		for ( i=0; i < 4; i++) {
			pool.push_back( shared[i] );
		}
		
		calcBestHand();
		bestDlrHand = bestHand;

		if (bestPlrHand.getValue() >= bestDlrHand.getValue() ) {
			// collect money					
			cash += pot;
		}
		fakecash = cash; // for animation purposes only

		play_sample(sfxSwish, 255, 128, 900 + irand(200), FALSE );
	}

	alfont_set_font_size( gamefont, 40 );
	draw_shadow_text( targ, gamefont, "Your Best Poker Hand", 20, 100, makecol( 100, 255, 255 ) );
		
	if (callAnim >= 5) {
		alfont_set_font_size( gamefont, 35 );
		draw_shadow_text( targ, gamefont, bestPlrHand.name.c_str(), 20+(5*55), 145, makecol( 255, 100, 255 ) );

		alfont_set_font_size( gamefont, 25 );
		draw_shadow_text( targ, gamefont, bestPlrHand.details.c_str(), 20+(5*55), 185, makecol( 255, 100, 255 ) );
	}

	int xpos;
	for ( i=0; i < bestPlrHand.card.size(); i++) {
		if (i > callAnim) break;
		xpos = 20 + i*55;
		if (i==callAnim) {
			if (xpos < callAnimX) {
				xpos = callAnimX;
			} else {
				callAnimX = 640;
				callAnim++;

				
				play_sample(sfxSwish, 255, 128, 900 + irand(200), FALSE );
			}
		}

		bestPlrHand.card[i].drawBig( targ, xpos, 145);
	}


	
	alfont_set_font_size( gamefont, 40 );
	draw_shadow_text( targ, gamefont, "Dealer's Best Poker Hand", 20, 240, makecol( 100, 255, 255 ) );
	
	if (callAnim >= 10) {

		alfont_set_font_size( gamefont, 35 );
		draw_shadow_text( targ, gamefont, bestDlrHand.name.c_str(), 20+(5*55), 285, makecol( 255, 100, 255 ) );

		alfont_set_font_size( gamefont, 25 );
		draw_shadow_text( targ, gamefont, bestDlrHand.details.c_str(), 20+(5*55), 325, makecol( 255, 100, 255 ) );
	}

	for ( i=0; i < bestDlrHand.card.size(); i++) {

		if (i+5 > callAnim) break;
		xpos = 20 + i*55;
		if (i+5==callAnim) {
			if (xpos < callAnimX) {
				xpos = callAnimX;
			} else {
				callAnimX = 640;
				callAnim++;

				if (callAnim < 10) {
					play_sample(sfxSwish, 255, 128, 900 + irand(200), FALSE );
				} else {
					if (bestDlrHand.getValue() > bestPlrHand.getValue() ) {
						play_sample( sfxLoser, 255, 128, 1000, TRUE );
					} else {
						play_sample( sfxWinner, 255, 128, 1000, TRUE );
					}
				}
			}
		}

		bestDlrHand.card[i].drawBig( targ, xpos, 275);
	}

	// do winning
	if (callAnim >= 10) {
		alfont_set_font_size( gamefont, 60 );
		if (bestDlrHand.getValue() > bestPlrHand.getValue() ) {
			
			draw_shadow_text( targ, gamefont, "Dealer Wins.", 20, 20, makecol( 255,255,100) );

			if (pot > 0) {
				if (pot > 1000) pot -= 100;
				else if (pot > 10) pot -= 10;
				pot -= 1;

				if (pot==0) {					
					stop_sample( sfxLoser);
				}
			}

		} else {
			draw_shadow_text( targ, gamefont, "You Win!", 20, 20, makecol( 255,255,100) );
			if (pot > 0) {				
				if (pot > 1000) { pot -= 100; fakecash += 100; }
				else if (pot > 10) { pot -= 10; fakecash += 10; }
				else { pot--; fakecash++; }

				if (pot==0) {					
					stop_sample( sfxWinner );
				}
			}
		}
	}

	char money[100];
	alfont_set_font_size( gamefont, 20 );		
	sprintf( money, "Cash $%d", fakecash );
	draw_shadow_text( targ, gamefont, money, 500, 20, makecol( 255,255,100) );
	sprintf( money, "Pot $%d", pot );
	draw_shadow_text( targ, gamefont, money, 500, 45, makecol( 255,255,100) );

}

void Game::shuffle() 
{
	deck.erase( deck.begin(), deck.end() );

	// make deck
	for (int suit=0; suit < 4; suit++) {
		for (int v = 2; v <= 14; v++) {
			deck.push_back( Card( suit, v ) );
		}
	}

	// shuffle
	for (int i=0; i < 52; i++) {
		Card c(0,0);
		int ndx = irand(52);		
		
		c = deck[ndx];
		deck[ndx] = deck[i];
		deck[i] = c;

	}

}

void Game::deal() 
{
	shuffle();

	shared.erase( shared.begin(), shared.end() );
	for (int i=0; i < 4; i++) {
		shared.push_back( deck[0] );
		deck.pop_front();
	}

	// clear hands
	ophand.erase( ophand.begin(), ophand.end() );
	hand.erase( hand.begin(), hand.end() );	

	// todo.. fake opponent thing

	// table cards
	//for (i=0; i < 5; i++) {
	//	table[i].card = deck[0];
	//	deck.pop_front();

	//	table[i].age = 0;
	//	table[i].x = irand(40);
	//	table[i].y = irand(30);
	//}
}

void load_tileset( char *filename, std::vector<BITMAP*> &targtiles ) {
	BITMAP *tiles = load_bitmap( filename, NULL );
	for (int j=0; j < tiles->h / 16; j++) {
		for (int i=0; i < tiles->w / 16; i++) {
			BITMAP *t = create_bitmap( 16, 16 );
			blit( tiles, t, i*16, j*16, 0,0, 16, 16 );
			targtiles.push_back( t );
		}
	}

	destroy_bitmap( tiles );
}

int main( int argc, char *argv[] ) 
{
	int screenMode = GFX_AUTODETECT_WINDOWED;
	BITMAP *backbuf;
	int mode = MODE_TITLE;
	int last_retrace = 0;


	bool showConsole = false;

	srand( time(0) );
	//srand( 131 ); // don't use time for dbg 

	//args
	for (int i=1; i < argc; i++) {
		if (!strcmp(argv[i],"-fullscreen") ) {
			screenMode = GFX_AUTODETECT_FULLSCREEN;
		}
	}

	// Init alleg
	allegro_init();
	install_keyboard();
	install_mouse();
	set_color_depth( 16 );

	set_keyboard_rate( 500,10 );

	/* install a digital sound driver */
   if (install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, argv[0]) != 0) {
      allegro_message("Error initialising sound system\n%s\n", allegro_error);
      return 1;
   }

   sfxGetCard = load_sample( "getcard.wav" );
   sfxSwish = load_sample( "swish.wav" );
   sfxBleep = load_sample( "bleep.wav" );
   sfxSiren = load_sample( "airraid.wav" );

   sfxWinner = load_sample( "winner.wav" );
   sfxLoser = load_sample( "loser.wav" );

   music = load_midi("goofysong1.mid");
   titlemusic = load_midi("title.mid");

	// Turn on the screeny goodness
	if (set_gfx_mode(screenMode, 640,480, 0, 0) != 0) {
      set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
      allegro_message("Unable to set 640x480 graphic mode\n%s\n", allegro_error);
      return 1;
   }

	alfont_init();
	alfont_text_mode( -1 );

	// fonts
	titlefont = alfont_load_font("fatty.ttf");
	gamefont  = alfont_load_font("number44.ttf");
	

	// screen 
	backbuf = create_bitmap( 640, 480 );
	rectfill( backbuf, 0,0, 640, 480, 0x0 );	

	// load and separate tiles
	load_tileset( "floortiles.bmp", tileset );
	load_tileset( "walls.bmp", walltileset );	
	
	pukegfx = load_bitmap( "puke.bmp", NULL );
	discardBmp = load_bitmap( "discard.bmp", NULL );
	cashtiles = load_bitmap( "cashtiles.bmp", NULL );

	//titlebg = load_bitmap( "badfood-title.bmp", NULL );
	titlebg = load_jpeg( "badfood-title.jpg" );
	docs = load_jpeg( "instructions.jpg" );

	theGame = new Game();

	Card::initCardBmps();

	play_midi( titlemusic, TRUE );

	// main loop
	bool done = false;
	while (!done) {

		if (mode==MODE_TITLE) {
			//rectfill( backbuf, 0,0, 640, 480, 0x0 );	

			blit ( titlebg, backbuf, 0,0, 0,0, 640, 480 );

			int i;
			for (i = 0; i < 10; i++) {
				alfont_set_font_size(  titlefont, 72 );
				alfont_textout_centre( backbuf, titlefont, "Bad Food",  
								320, 100 - (int)(fabs(sin((float)(retrace_count+i)/20.0)) * 50), 
								makecol(107,50+(i*20),35) );
			}

			i=0;
			alfont_set_font_size( titlefont, 26 );
			while (menu[i]) {				
				draw_shadow_text_centre( backbuf, titlefont, 
					menu[i],  320, 200 + 30*i + (i>2?20:0), 
						menuNdx==i?makecol(255, 255,35):makecol(80, 110, 80) );
				i++;
			}


			alfont_set_font_size( gamefont, 25 );
			draw_shadow_text_centre( backbuf, gamefont, 
					"A game by Joel Davis",  320, 400, makecol(107,200,35) );
			draw_shadow_text_centre( backbuf, gamefont, 
					"Made for the fourth LudumDare 48 hour contest.",  320, 420, makecol(107,200,35) );

			alfont_set_font_size( gamefont, 15 );
			draw_shadow_text_centre( backbuf, gamefont, 
					"(But not finished within the 48 hours)",  320, 445, makecol(107,200,35) );

		} else if (mode==MODE_DOCS) {

			if (docText[docPage]) {				

				rectfill( backbuf, 0,0, 640, 480, 0x0 );	
				int yval = 80, yincr = 35;

				if (docPage==0) {
					alfont_set_font_size( gamefont, 30 );
				} else {
					alfont_set_font_size( gamefont, 20 );
					yval = 40;
					yincr = 35;
				}

				char *s;
				
				char txt[1024];
				strcpy( txt, docText[docPage] );
				s = strtok( txt, "\n" );
				while( s ) {
					alfont_textout( backbuf, gamefont, s, 100, yval, makecol(107,200,35) );
					yval += yincr;

					s = strtok( NULL, "\n" );
				}

				if (docPage==1) {
					// show example poker hands
					PokerHand ex;
					yval = 165;
					yincr = 35;
					for (int i=0; i < 9; i++) {
						ex.card.erase( ex.card.begin(), ex.card.end() );
						
						switch( i ) {
						case 0:
							// royal flush example
							ex.card.push_back( Card( 2, 10 ) );
							ex.card.push_back( Card( 2, 11 ) );
							ex.card.push_back( Card( 2, 12 ) );
							ex.card.push_back( Card( 2, 13 ) );
							ex.card.push_back( Card( 2, 14 ) );
							break;
						case 1:
							// stright flush example
							ex.card.push_back( Card( 3, 4 ) );
							ex.card.push_back( Card( 3, 5 ) );
							ex.card.push_back( Card( 3, 6 ) );
							ex.card.push_back( Card( 3, 7 ) );
							ex.card.push_back( Card( 3, 8 ) );
							break;
						case 2:
							// 4 of a kind example
							ex.card.push_back( Card( 2, 8 ) );
							ex.card.push_back( Card( 0, 12 ) );
							ex.card.push_back( Card( 1, 12 ) );
							ex.card.push_back( Card( 2, 12 ) );
							ex.card.push_back( Card( 3, 12 ) );							
							break;
						case 3:
							// full house example
							ex.card.push_back( Card( 3, 4 ) );
							ex.card.push_back( Card( 1, 4 ) );
							ex.card.push_back( Card( 0, 10 ) );
							ex.card.push_back( Card( 1, 10 ) );
							ex.card.push_back( Card( 2, 10 ) );							
							break;
						case 4:
							// flush
							ex.card.push_back( Card( 0, 2 ) );
							ex.card.push_back( Card( 0, 8 ) );
							ex.card.push_back( Card( 0, 9 ) );
							ex.card.push_back( Card( 0, 10 ) );
							ex.card.push_back( Card( 0, 14 ) );
							break;
						case 5:
							// straight
							ex.card.push_back( Card( 3, 7 ) );
							ex.card.push_back( Card( 2, 8 ) );
							ex.card.push_back( Card( 1, 9 ) );
							ex.card.push_back( Card( 0, 10 ) );
							ex.card.push_back( Card( 2, 11 ) );
							break;
						case 6:
							// three of a kind
							ex.card.push_back( Card( 2, 8 ) );
							ex.card.push_back( Card( 0, 8 ) );
							ex.card.push_back( Card( 1, 8 ) );
							ex.card.push_back( Card( 3, 11 ) );
							ex.card.push_back( Card( 2, 12 ) );
							break;
						case 7:
							// two pair
							ex.card.push_back( Card( 2, 4 ) );
							ex.card.push_back( Card( 0, 4 ) );
							ex.card.push_back( Card( 1, 6 ) );
							ex.card.push_back( Card( 3, 14 ) );
							ex.card.push_back( Card( 2, 14 ) );
							break;
						case 8:
							// pair
							ex.card.push_back( Card( 3, 2 ) );
							ex.card.push_back( Card( 2, 4 ) );
							ex.card.push_back( Card( 0, 9 ) );
							ex.card.push_back( Card( 1, 9 ) );
							ex.card.push_back( Card( 3, 11 ) );
							break;

						default:
							// royal flush example
							ex.card.push_back( Card( 0, 10 ) );
							ex.card.push_back( Card( 0, 11 ) );
							ex.card.push_back( Card( 0, 12 ) );
							ex.card.push_back( Card( 0, 13 ) );
							ex.card.push_back( Card( 0, 14 ) );
							break;
						}

						for (int j=0; j < 5; j++) {
							ex.card[j].drawSmall( backbuf, 320 + j*15, yval );
						}

						line( backbuf, 30, yval-1, 610, yval-1, makecol( 107,200,35) );

						yval += yincr;

						

					}
				}

			} else {
				// last page, show the diagram
				blit( docs, backbuf, 0,0, 0,0, 640, 480 );
			}

		} else if (mode==MODE_GAME) {
			//rectfill( backbuf, 0,0, 640, 480, 0x0 );				
			theGame->draw( backbuf );	
			
			// forget the alleg mouse driver, just draw the cursor manually
			if (editor) {
				if (!editCursor) {
					editCursor = load_bitmap( "cursor.bmp", NULL );
				}
				draw_sprite( backbuf, editCursor, mouse_x, mouse_y );
			}


			if (editor) {
				alfont_set_font_size( gamefont, 14 );
				alfont_textout( backbuf, gamefont, 
					"Edit Mode Active",  500, 10, makecol(50, 50, 255) );
			}

			if ((!editor) &&(retrace_count - last_retrace > theGame->updaterate) ) {
				

				if (theGame->phase == GAME_PLAY) {

					if (theGame->hand.size() <=3) {
						theGame->update();
					} else {
						theGame->discardTimeLeft--;	
						
						if (theGame->discardTimeLeft ==0 ) {
							// discard current discard
							theGame->hand[theGame->discardCursorPos] = theGame->hand.back();
							theGame->hand.pop_back();
							
							stop_sample(sfxSiren);
						}
					}
					last_retrace = retrace_count;
				}

				
				
			}
			
			
			if (theGame->phase == GAME_CALL) {
				theGame->callAnimX -= 50;
			}
		}

		// event handler
		while (keypressed()) {			
			int k = readkey();
			if (mode==MODE_TITLE) {
				switch(k>>8){
				case KEY_ENTER:
					if (menuNdx <3) {
						theGame->newGame();
						mode = MODE_GAME;
						
						play_midi( music, TRUE);

					} else if (menuNdx==3) {
						mode = MODE_DOCS;
						docPage = 0;
					} else  if (menuNdx==4) {
						// quit
						done = true;
					}
					break;
				case KEY_DOWN:
					menuNdx++;
					if (!menu[menuNdx]) menuNdx= 0;
					break;
				case KEY_UP:
					if (menuNdx > 0) {
						menuNdx--;
					} else {
						while(menu[menuNdx+1]) menuNdx++;
					}
				}
			} else if (mode==MODE_DOCS) {
				switch(k>>8){
				case KEY_ENTER:					
					if (!docText[docPage]) {
						mode = MODE_TITLE;
					} else {
						docPage++;
					}
					break;
				}
			} else if (mode==MODE_GAME) {

				switch(k>>8){
				case KEY_W:
				case KEY_UP:
					if (theGame->hand.size() <= 3 ) {					
						if (theGame->dir != DIR_DOWN) theGame->nextDir = DIR_UP;
					}
					break;
					
				case KEY_S:
				case KEY_DOWN:
					if (theGame->hand.size() <= 3 ) {					
						if (theGame->dir != DIR_UP) theGame->nextDir = DIR_DOWN;
					}
					break;

				case KEY_A:
				case KEY_LEFT:
					if (theGame->hand.size() >3 ) {
						theGame->discardCursorPos--;
						if (theGame->discardCursorPos == -1) {
							theGame->discardCursorPos = theGame->hand.size()-1;
						}
					} else {
						if (theGame->dir != DIR_RIGHT) theGame->nextDir = DIR_LEFT;
					}
					break;

				case KEY_D:
				case KEY_RIGHT:
					if (theGame->hand.size() >3 ) {
						theGame->discardCursorPos++;
						if (theGame->discardCursorPos == theGame->hand.size()) {
							theGame->discardCursorPos = 0;
						}

					} else {
						if (theGame->dir != DIR_LEFT) theGame->nextDir = DIR_RIGHT;
					}
					break;

				case KEY_ENTER:
				case KEY_SPACE:
					if (theGame->phase == GAME_PLAY) {
						if (theGame->hand.size() >3 ) {
							// discard current discard
							theGame->hand[theGame->discardCursorPos] = theGame->hand.back();
							theGame->hand.pop_back();
							last_retrace = retrace_count;
							stop_sample(sfxSiren);
						} else {
							if (theGame->hand.size() == 3) {
								theGame->Call();							
							}
						}
					} else if ((theGame->phase == GAME_FOLD ) ||
								(theGame->phase == GAME_CALL )) {
						
						if (theGame->cash > 0) {
							// next level
							theGame->initNextLevel();
							theGame->phase = GAME_PLAY;
						} else {
							mode = MODE_TITLE;
							//stop_midi();
							play_midi( titlemusic, TRUE );
						}
					}
					break;

				case KEY_F11:
					editor = !editor;					
					break;

// debugging codes
#if 0
				case KEY_1:
					// cheat.. deal a card
					theGame->hand.push_back( theGame->deck.front() );
					theGame->deck.pop_front();
					theGame->discardCursorPos = theGame->hand.size()-1;
					theGame->discardTimeLeft = DISCARD_TIME;
					break;

				case KEY_2:
					// cheat.. discard top card of deck					
					// for testing deck running out
					theGame->deck.pop_front();					
					break;

				case KEY_3:
					// cheat.. test message
					theGame->message( "Hello there." );
					break;

				case KEY_4:
					// cheat.. lose money
					theGame->cash -= 100;
					if (theGame->cash < 0) theGame->cash = 0;
					break;
#endif


				case KEY_PLUS_PAD:
				case KEY_MINUS_PAD:
					if (editor) {
						int x = mouse_x / 16;
						int y = mouse_y / 16;
						
						if (k>>8== KEY_PLUS_PAD) {
							theGame->map[x][y].tile ++;
							if (theGame->map[x][y].tile == tileset.size() ) {
								theGame->map[x][y].tile = 0;
							}
						} else {
							theGame->map[x][y].tile--;
							if (theGame->map[x][y].tile == -1 ) {
								theGame->map[x][y].tile = tileset.size()-1;
							}
						}						
						lasttile = theGame->map[x][y].tile;
					}
					break;
				case KEY_DEL_PAD:
					if (editor) {
						int x = mouse_x / 16;
						int y = mouse_y / 16;
						theGame->map[x][y].tile = lasttile;
					}
					break;

				case KEY_EQUALS:
				case KEY_MINUS:				
					if (editor) {
						int x = mouse_x / 16;
						int y = mouse_y / 16;
						
						if (k>>8== KEY_EQUALS) {
							theGame->map[x][y].wall++;
							if (theGame->map[x][y].wall > walltileset.size() ) {
								theGame->map[x][y].wall = 0;
							}
						} else {
							theGame->map[x][y].wall--;
							if (theGame->map[x][y].wall == -1 ) {
								theGame->map[x][y].wall = walltileset.size();
							}
						}					
						lastwall = theGame->map[x][y].wall;
					}
					break;
				
				case KEY_0:
					if (editor) {
						int x = mouse_x / 16;
						int y = mouse_y / 16;
						theGame->map[x][y].wall = lastwall;
					}
					break;

				case KEY_O:
					if (editor) {
						int x = mouse_x / 16;
						int y = mouse_y / 16;
						markX = x;
						markY = y;
					}
					break;

				case KEY_P: 
					if (editor) {
						int x = mouse_x / 16;
						int y = mouse_y / 16;
						for (int j=min(markY, y); j <= max(markY, y); j++) {
							for (int i=min(markX, x); i <= max(markX, x); i++) {
								theGame->map[i][j].tile = lasttile;
							}
						}
					}					
					break;

				case KEY_C:
					if (editor) {
						// clear walls
						for (int j=0; j < 30; j++) {
							for (int i=0; i < 40; i++) {
								theGame->map[i][j].wall = 0;
							}
						}
					}
					break;
					

				case KEY_ASTERISK:
					if (editor) {
						FILE *fp = fopen("level.txt", "wt" );
						int ndx = 0;
						fprintf( fp, "int leveldata = {\n" );
						for (int j=0; j < 30; j++) {
							for (int i=0; i < 40; i++) {
								if (ndx==20) {
									fprintf(fp, "\n" );
									ndx=0;
								}
								ndx++;
								fprintf( fp, "%d, ", 
									(theGame->map[i][j].wall << 8 ) +theGame->map[i][j].tile  );
							}
						}
						fclose(fp);
					}
					break;

				}
			}
		}
	
		if (key[KEY_ESC]) {
			//exit(0);
			break;
		}

		// Blit the screen		
		vsync();		
		acquire_screen();
		blit( backbuf, screen, 0, 0, 0, 0, 640, 480 );
		release_screen();
		
		yield_timeslice();
	}
	

	return 0;
}
END_OF_MAIN();
