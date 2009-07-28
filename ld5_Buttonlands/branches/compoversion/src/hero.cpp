#include <allegro.h>

#include <tinyxml.h>

#include <vector>
#include <assert.h>

#include "buttonlands.h"
#include "hero.h"

BITMAP *fancy_portrait( BITMAP *orig, int topcol, int botcol )
{
	BITMAP *newpic = create_bitmap( 200, 200 );
	static BITMAP *ring = NULL;
	static BITMAP *ringmask = NULL;

	if (!ring) {
		ring = load_jpeg( "gamedata/ring.jpg", 60);
	}

	if (!ringmask) {
		ringmask = create_bitmap( 200, 200 );
		rectfill( ringmask, 0, 0, 200, 200, makecol( 255,0,255) );
		circlefill( ringmask, 100, 100, 95, makecol( 0, 0, 255 ) );
	}

	for (int j=0; j < 200; j++) {
		float t = (float)j / (float)200, t2;
		t2 = 1.0- t;
		int r1,g1,b1,r2,g2,b2;
		r1 = getr( topcol ); g1 = getg( topcol ); b1 = getb( topcol );
		r2 = getr( botcol ); g2 = getg( botcol ); b2 = getb( botcol );
		hline( newpic, 0, j, 200, makecol( r2*t + r1*t2, g2*t+g1*t2, b2*t+b1*t2 ) );		
	}

	masked_blit( orig, newpic, 0, 0, 0,0, 200, 200 );
	
	for ( j=0; j < 200; j++) {
		for (int i=0; i < 200; i++) {
			if (getpixel( ringmask, i,j ) == makecol(255,0,255) ) {
				putpixel( newpic, i,j, makecol( 255,0,255 ) );
			}
		}
	}

	masked_blit( ring, newpic, 0, 0, 0,0, 200, 200 );

	destroy_bitmap( orig );

	return newpic;
}

void load_buttons( const char *filename, std::vector<BMHero*> &heros ) 
{
	BMHero *bm;


	TiXmlDocument *xmlDoc = new TiXmlDocument( filename );

	if (!xmlDoc->LoadFile() ) {
		printf("ERR! Can't load %s\n", filename );
	}


	TiXmlElement *xButtonSet, *xButtonMan;
	TiXmlNode *xText;	


	xButtonSet = xmlDoc->FirstChildElement( "buttonset" );
	assert( xButtonSet );

	xButtonMan = xButtonSet->FirstChildElement( "buttonman" );
	while (xButtonMan) {		
		bm = new BMHero();
		bm->name = xButtonMan->Attribute( "name" );
		xText = xButtonMan->FirstChild("bio" );		

		if (xText) {
			bm->bio = xText->FirstChild()->Value();
		} else {
			bm->bio = std::string("Little is known about him, other than he likes to beat people up.\n" );
		}

		if (xButtonMan->Attribute("pic")) {
				const char *picfn = xButtonMan->Attribute("pic");
				int ctop,cbot, r,g,b;

				if (xButtonMan->Attribute("ctop"))  {
					sscanf( xButtonMan->Attribute("ctop"), "%d,%d,%d", &r, &g, &b );
					ctop = makecol( r,g,b );
				} else {
					ctop = makecol( 0,0,0 );
				}

				if (xButtonMan->Attribute("cbot"))  {
					sscanf( xButtonMan->Attribute("cbot"), "%d,%d,%d", &r, &g, &b );
					cbot = makecol( r,g,b );
				} else {
					cbot = makecol( 255,255,255);
				}

				char fn[1024];
				sprintf( fn, "gamedata/%s", picfn );
				bm->photo = fancy_portrait( load_jpeg( fn, 70 ), ctop, cbot );
				

			}

		// read dice
		int ndx = 0;
		TiXmlElement *xDie;
		xDie = xButtonMan->FirstChildElement("die");
		while (xDie) {

			assert( ndx < 5 ); // max 5 dice per bm
			
			const char *sides = xDie->Attribute( "sides" );
			if (!sides) {
				printf( "Warning: %s, die %d has no sides attr\n", bm->name.c_str(), ndx );
				bm->die[ndx].nrmsides = 20;
			} else {
				if (isdigit(sides[0])) {
					bm->die[ndx].nrmsides = atoi( sides );
					bm->die[ndx].sides = atoi( sides );
				} else if (sides[0]=='X') {
					bm->die[ndx].nrmsides = SIDES_X;
					bm->die[ndx].sides = 20;
				}
			}

			// altsides can't have an X
			sides = xDie->Attribute( "altsides" );
			if (sides)  {		
				if (isdigit(sides[0])) {
					bm->die[ndx].altsides = atoi( sides );			
				}
			} else {
				bm->die[ndx].altsides = SIDES_NONE;
			}

			// special type
			bm->die[ndx].shadow = false;
			const char *type = xDie->Attribute("type");
			if (type) {
				if (!strcmp( type, "shadow" )) {
					bm->die[ndx].shadow = true;
				}
			}

			

			ndx++;			
			

			xDie = xDie->NextSiblingElement( "die" );
		}
		heros.push_back( bm );

		printf( "Name %s\nBio: %s\n", bm->name.c_str(), bm->bio.c_str() );

		xButtonMan = xButtonMan->NextSiblingElement( "buttonman" );
	}

	xmlDoc->Clear();
	delete xmlDoc;
}


//======================================================================================

void BMDie::roll()
{	
	
	val = ((float)rand() / (float)RAND_MAX) * sides;	
	val += 1;	
}

//======================================================================================

void BMHero::rollAll() 
{
	for (int i=0; i < 5; i++) {
		die[i].roll();
	}
}