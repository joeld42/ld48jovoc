#include <math.h>

#include "world.h"
#include "focus.h"

#include "gamefontgl.h"
//#include "background.h"

extern GLuint glFontTexId, fntFontId;

World::World() :
	initialized( false ),
		mp3file( NULL )
{
}

World::~World()
{
	if (mp3file)
	{
		FSOUND_Stream_Stop( mp3file );
		FSOUND_Stream_Close( mp3file );
	}
}

void World::makeGemTrack()
{
	gemtrack.clear();
	gemtrack.resize( 10000 ); // long enuf for Freebird...?

	std::fill( gemtrack.begin(), gemtrack.end(), GEM_REST );
	int ndx = 0; 
	while (ndx < gemtrack.size())
	{	
		// first 4 bars all rests for sync
		if (ndx < 16 * 4)
		{
			ndx++;
		}
		else
		{
			
			//if ((ndx % 2)==0)
			//{
				//gemtrack[ndx] = GEM_NOTE;
			//}
			//ndx++;
#if 1
			// choose a random pattern
			int pat = irand( 0, 4 );			

			switch (pat)
			{
			case 0:
				{
					// every nth beat
					int inc = irand( 0, 4 );
					int val=1;
					while( inc-- )
					{
						val = val << 1;
					}
					for (int i=0; i < 16; i++)
					{
						if (i % val == 0)
						{
							gemtrack[ndx] = GEM_NOTE;
						}
						ndx += 1;
					}
				}
				break;

			case 1:
				// totally random
				for (int i=0; i < 16; i++)
				{
					if (frand( 0.0f, 1.0f) > 0.6f)
					{
						gemtrack[ndx] = GEM_NOTE;
					}
					ndx ++;
				}
				break;
			case 2:				
			case 3:				
				// random bar, repeated 4x
				bool bar[4];
				for (int i=0; i < 4; i++)
				{
					if (frand( 0.0f, 1.0f) > 0.5f)					
						bar[i] = true;
					else bar[i] = false;										
				}
				for (int i=0; i < 16; i++)
				{
					if (bar[i%4]) gemtrack[ndx] = GEM_NOTE;
					ndx++;
				}
				break;			

			}
#endif
		}
	}	
}
void World::redraw( Beatronome &nome )
{
	// init here to make sure all
	// gfx resources are set up
	if (!initialized)
	{
		init();
		initialized = true;
	}

	//bg->redraw();		

	// Draw 2D stuff here
	// ---		

	// draw the title card
	glBindTexture( GL_TEXTURE_2D, texBackground );

	glEnable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );	

	glColor3f( 1.0, 1.0, 1.0 );
	glBegin( GL_QUADS );
	glTexCoord2d( 0.0, 0.0 ); glVertex3f( 0.0, 600.0, 0.0 );
	glTexCoord2d( 0.0, 1.0 ); glVertex3f( 0.0,  0.0, 0.0 );
	glTexCoord2d( 1.2, 1.0 ); glVertex3f(  800.0,  0.0, 0.0 );
	glTexCoord2d( 1.2, 0.0 ); glVertex3f(  800.0, 600.0, 0.0 );
	glEnd();

	gfEnableFont( fntFontId, 15 );	
	gfBeginText();
	glTranslated( 10, 10, 0 );
	int beat_ndx = (nome.beat_count * 4) + nome.beat_subcount;
	gfDrawStringFmt( "Beat: M %0.2f NDX %d:%d (%d) Note: %s", 
		nome.beat_measure,
		nome.beat_count, 
		nome.beat_subcount,
		beat_ndx,
		gemtrack[ beat_ndx ]==GEM_NOTE?"NOTE":"rest" );
	gfEndText();


	if (nome.beat_count < 20 )
	{
		gfEnableFont( fntFontId, 32 );	
		gfBeginText();
		glTranslated( 50, 400, 0 );	
		gfDrawString("Tap SHIFT to sync to the\n"
					 "beat of the song.\n\n"
					 "You can do this\n"
					 "anytime during the\n"
					 "song if the tempo\n"
					 "changes." );
		gfEndText();
	}

	//gfBeginText();
	//glTranslated( 600, 180, 0 );	
	//gfDrawString( gemtrack[ beat_ndx ]==GEM_NOTE?"#***#":"..." );
	//gfEndText();


	// draw the fretboard card
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();

	glLoadIdentity();
	gluPerspective( 45.0, 800.0/600.0, 1.0, 100.0 );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	glPushMatrix();
	glTranslated( 0.2, 0.3, -1.6 );
	glRotated( -35.0f, 1.0, 0.0, 0.0 );

	glBindTexture( GL_TEXTURE_2D, texFretboard );

	glEnable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );	

	glColor3f( 1.0, 1.0, 1.0 );	

	glBegin( GL_QUADS );
	glTexCoord2d( 0.0, 0.0 - nome.beat_measure ); glVertex3f( -0.15,  1.0, 0.0 );
	glTexCoord2d( 0.0, 1.0 - nome.beat_measure);  glVertex3f( -0.15, -1.0, 0.0 );
	glTexCoord2d( 1.0, 1.0 - nome.beat_measure);  glVertex3f(  0.15, -1.0, 0.0 );
	glTexCoord2d( 1.0, 0.0 - nome.beat_measure);  glVertex3f(  0.15,  1.0, 0.0 );
	glEnd();

	// draw target area
	glBindTexture( GL_TEXTURE_2D, texTarget );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );		

	glPushMatrix();	
	glTranslated( 0.0, -0.60, 0.0 );
	
	glBegin( GL_QUADS );
	glTexCoord2d( 0.0, 0.0 ); glVertex3f( -0.15,  0.04, 0.001 );
	glTexCoord2d( 0.0, 1.0 ); glVertex3f( -0.15, -0.04, 0.001 );
	glTexCoord2d( 1.0, 1.0 ); glVertex3f(  0.15, -0.04, 0.001 );
	glTexCoord2d( 1.0, 0.0 ); glVertex3f(  0.15,  0.04, 0.001 );
	glEnd();

	glPopMatrix();

	// draw gems
	glBindTexture( GL_TEXTURE_2D, texGem );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );		

	for (int i=0; i < 48; i++)
	{			

		// is there a gem here?
		size_t ndx = ((nome.beat_count/4) * 16) + i - 8;
		
		if ( (ndx >= gemtrack.size()) ||
			 (ndx < 0) ||
			 (gemtrack[ndx] == GEM_REST) ) continue;

		int gemType = gemtrack[ndx] -1;

		glPushMatrix();	
		glTranslated( 0.0, ((i-12)*0.125) - (nome.beat_measure * 2.0), 0.0 );
		glBegin( GL_QUADS );
		glTexCoord2d( 0.00 + gemType * 0.25, 0.0 ); glVertex3f( -0.04,  0.04, 0.001 );
		glTexCoord2d( 0.00 + gemType * 0.25, 1.0 ); glVertex3f( -0.04, -0.04, 0.001 );
		glTexCoord2d( 0.25 + gemType * 0.25, 1.0 ); glVertex3f(  0.04, -0.04, 0.001 );
		glTexCoord2d( 0.25 + gemType * 0.25, 0.0 ); glVertex3f(  0.04,  0.04, 0.001 );
		glEnd();
		glPopMatrix();
	}
	
	glPopMatrix();

	glDisable( GL_TEXTURE_2D );

	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();

}

void World::init()
{
	//bg = new BGMondrian();

	glDisable( GL_CULL_FACE );
//	glFillMode( GL_SOLID );
	
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );		
	
	// fretboard image
	ilGenImages( 1, &ilFretboard );
	if (!ilLoadImage( (ILstring)"gamedata/fretboard.png" ))
	{
		printf("Loading fretboard image failed.\n" );
	}	
	texFretboard = ilutGLBindTexImage();

	// gem image
	ilGenImages( 1, &ilGem );
	if (!ilLoadImage( (ILstring)"gamedata/gem.png" ))
	{
		printf("Loading gem image failed.\n" );
	}

	texGem = ilutGLBindTexImage();

	// target image
	ilGenImages( 1, &ilTarget );
	if (!ilLoadImage( (ILstring)"gamedata/target.png" ))
	{
		printf("Loading target image failed.\n" );
	}

	texTarget = ilutGLBindTexImage();

	// background image
	ilGenImages( 1, &ilBackground );
	if (!ilLoadImage( (ILstring)"gamedata/bg.png" ))
	{
		printf("Loading bg image failed.\n" );
	}

	texBackground = ilutGLBindTexImage();
}

void World::update( Beatronome &nome )
{
	float xtra = nome.beat_pulse * 4.0;
	xtra = xtra - floor(xtra);

	// look at the last note
	int beat_ndx = (nome.beat_count * 4) + nome.beat_subcount;	
	beat_ndx -= 1;

	if ((beat_ndx >=0) && (beat_ndx < gemtrack.size()) )
	{
		// missed the note
		if (gemtrack[beat_ndx] == GEM_NOTE)
		{			
			gemtrack[beat_ndx] = GEM_NOTE_MISSED;
		}
	}
	
}

void World::setMusic( const char *musicfile )
{
	FILE *fp = fopen( musicfile, "r" );
	if (!fp)
	{
		printf("File no exist: %s\n", musicfile );
		
	} 
	else fclose(fp);
	
	mp3file = FSOUND_Stream_Open( musicfile, 0,0, 0 );
}

void World::playMusic()
{
	if (!mp3file) return;

	makeGemTrack();

	FSOUND_Stream_PlayEx( FSOUND_FREE, mp3file, NULL, false );
}

void World::stopMusic()
{
	FSOUND_Stream_Stop( mp3file );
}