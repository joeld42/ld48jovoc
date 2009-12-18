//==================================================================
// Copyright (c) 2009 Joel Davis. 
//==================================================================
#ifdef WIN32

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <crtdbg.h>
#endif

#include <time.h>
#include <math.h>
#include <assert.h>

#include <algorithm>
#include <iostream>
#include <vector>

#include <SDL.h>
#include <SDL_endian.h>
#include <SDL_draw.h>
#include <SDL_ttf.h>

#include <GLee.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <fmod.hpp>

using namespace std;

// 30 ticks per sim frame
#define STEPTIME (33)

#define BAR_TEX_W (2048)
#define BAR_TEX_H (64)
#define BAR_TEX_H_2 ((BAR_TEX_H-1) / 2)

#define WAVE_SIZE (256)

#define SAMPLE_THRESHOLD (1200)

#ifndef M_PI
const double M_PI = 3.14159265358979323846;
#endif

enum 
{	
	TRACK_DRUMS,
	TRACK_BASSLINE,	
	TRACK_LEAD,
	TRACK_RHYTHM
};

//===================================================================
// Globals 
FMOD::Sound *g_recSound = NULL;
FMOD::System *g_fmod = NULL;

TTF_Font *g_font = NULL;

//===================================================================
class Bar
{
public:
	Bar() : 
	  m_willRecFlag( false ),
	  m_sample (NULL),
	  m_wave(NULL)
	{
	}

	 ~Bar()
	 {
		 delete [] m_wave;
	 }

	 // returns false if nothing recorded
	 bool normalizeAndCreateWaveform()
	 {
		 if (!m_sample) return false;

		 if (m_wave)
		 {
			 delete [] m_wave;
		 }

		 m_wave = new int[WAVE_SIZE];			
		 memset( m_wave, 0, sizeof(int)* WAVE_SIZE );

		 unsigned int len;
		 m_sample->getLength( &len, FMOD_TIMEUNIT_PCMBYTES );
		 void *data1, *data2;
		 unsigned int len1, len2;
		 m_sample->lock( 0, len, &data1, &data2, &len1, &len2 );

		 printf("LOCK len %u data1 %p len %u data2 %p len %u\n", 
					len, data1, len1, data2, len2 );
		 
		 // assume len2, data2 is NULL ...

		 // Normalize the sample
		 Sint16 maxVal = 0;
		 Sint16 *sample = (Sint16*)data1;
		 for (size_t i=0; i < (len1>>1); ++i )
		 {
			 // left, right interleaved
			Sint16 s = abs(sample[i]);
			if (s > maxVal) maxVal = s;
		 }
		
		 bool doNormalize = (maxVal > SAMPLE_THRESHOLD);

		for (size_t i=0; i < (len1>>1); ++i )
		{
			// left, right interleaved .. just glom together
			Sint16 s = sample[i];
			if (doNormalize)
			{
				sample[i] = (Sint16)( ((float)s / (float)maxVal) * 32768.0f);
			}
			
			// update waveform
			size_t waveNdx = (size_t)( ((float)i / (float)(len1>>1)) * WAVE_SIZE );
			if (abs(sample[i]) > m_wave[waveNdx])
			{
				m_wave[waveNdx]= abs(sample[i]);							
			}
		}
		

		 printf("================================\n");
		 printf("Max Sample Val %d\n", maxVal );
		 printf("================================\n");

		 m_sample->unlock( data1, data2, len1, len2 );
		 return true;
	 }

	bool m_willRecFlag;
	FMOD::Sound *m_sample;
	int *m_wave;
};

//============================================================================
class Track
{
public:
	Track( int ring ) :
		m_ring( ring+1),
		m_displayRadius( 0.0 ),
		m_surf(NULL),		
		m_chan( NULL )
	{
		m_bar.resize( 8 );
	}

	void adjustDisplayRadius( float dt )
	{
		float targ = ((m_ring+1) * 15.0f);

		m_displayRadius = (targ*dt) + (m_displayRadius*(1.0f-dt));
	}

	void setBarWillRecord( int bar )
	{
		m_bar[bar].m_willRecFlag = true;
		m_barsChanged = true;
	}

	void assignSound( int bar, FMOD::Sound *sound )
	{
		// Free existing sample
		if (m_bar[bar].m_sample)
		{
			m_bar[bar].m_sample->release();
			m_bar[bar].m_sample = NULL;
		}
		
		Bar newBar;
		newBar.m_sample = sound;

		m_bar[bar] = newBar;
		m_barsChanged = true;
	}

	void newBar( int bar )
	{
		if (m_bar[bar].m_sample)
		{
			printf("Playing sample!\n" );

			if (m_chan) m_chan->stop();
			g_fmod->playSound( FMOD_CHANNEL_FREE, m_bar[bar].m_sample, false, &m_chan );
		}
		else
		{
			// silence this bar
			if (m_chan)
			{
				m_chan->stop();
			}
		}

		// just be conservative and redraw just in case
		m_barsChanged = true;
	}

	void draw()
	{
		// Allocate surface if not exist
		if (!m_surf)
		{
			Uint32 rmask, gmask, bmask, amask;

    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
       on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

			m_surf = SDL_CreateRGBSurface( SDL_SWSURFACE, BAR_TEX_W, BAR_TEX_H, 32,
											rmask, gmask, bmask, amask );

			assert(m_surf);

			// Also allocate a gl texture for this
			glEnable( GL_TEXTURE_2D );
			glEnable( GL_ALPHA_TEST );
			glAlphaFunc( GL_GREATER, 0.5 );

			glGenTextures( 1, &m_glTex );
			glBindTexture( GL_TEXTURE_2D, m_glTex );

			// Set the texture's stretching properties
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );

			// and make a note to redraw
			m_barsChanged = true;
		}

		// Redraw surface if bars changed
		glBindTexture( GL_TEXTURE_2D, m_glTex );
		if (m_barsChanged)
		{
			Uint32 c_red = SDL_MapRGB(m_surf->format,255,0,0);
			
			// Draw bars
			Sint16 barWidth = BAR_TEX_W / (Sint16)m_bar.size();
			Uint32 c_barNormal = SDL_MapRGB(m_surf->format,180,222,193 );
			Uint32 c_barNormalGrey = SDL_MapRGB(m_surf->format,189,197,185 );
			Uint32 c_barDark = SDL_MapRGB(m_surf->format,87, 108, 94 );
			Uint32 c_sampleCol = SDL_MapRGB(m_surf->format,87, 108, 94 );
			Uint32 c_barOutlineNormal = SDL_MapRGB(m_surf->format,207,255,221 );
			Uint32 c_barOutlineGrey = SDL_MapRGB(m_surf->format,208,220,211 );
			Uint32 c_barOutlineWillRecord = SDL_MapRGB(m_surf->format,168, 81, 99 );
			Uint32 c_barOutlineRecording = SDL_MapRGB(m_surf->format,255, 31, 76 );
			
			//Uint32 c_barNormal = SDL_MapRGB(m_surf->format, 255, 255, 255, 255 );
			for (size_t i=0; i < m_bar.size(); ++i)
			{				
				Uint32 barOutlineCol = c_barOutlineNormal;
				Uint32 barFillCol = c_barNormal;

				if (m_bar[i].m_sample == NULL )
				{
					barOutlineCol = c_barOutlineGrey;
					barFillCol = c_barNormalGrey;
				}

				if ( (m_bar[i].m_sample) && (m_bar[i].m_sample == g_recSound))
				{
					barOutlineCol = c_barOutlineRecording;
					barFillCol = c_barOutlineWillRecord;
				}
				else if (m_bar[i].m_willRecFlag)
				{
					barOutlineCol = c_barOutlineWillRecord;
				}
				
				// Outline
				Sint16 bx = (Sint16)(i*barWidth);
				Draw_FillRound( m_surf, bx, 0, barWidth - 5, BAR_TEX_H-1, 10, barOutlineCol );				

				// Fill
				Draw_FillRound( m_surf, bx+5, 5, barWidth - 15, BAR_TEX_H-11, 10, barFillCol );

				// waveform
				if (m_bar[i].m_wave)
				{
					for (int x=0; x <= barWidth -10; ++x)
					{
						int waveNdx = (int)( ((float)x / (float)(barWidth-10)) * (WAVE_SIZE-1) );		
						float fhite = m_bar[i].m_wave[waveNdx] / 32768.0f;						

						int hite = (int)(fhite * BAR_TEX_H_2);
						Draw_Line( m_surf, bx+x, BAR_TEX_H_2 - hite, bx+x, BAR_TEX_H_2 + hite, c_sampleCol );
					}
				}

				// Measure bars
				Draw_Line( m_surf, bx + (Sint16)(barWidth*0.25), 0, bx + (Sint16)(barWidth*0.25), BAR_TEX_H-1, c_barDark );
				Draw_Line( m_surf, bx + (Sint16)(barWidth*0.50), 0, bx + (Sint16)(barWidth*0.50), BAR_TEX_H-1, c_barDark );
				Draw_Line( m_surf, bx + (Sint16)(barWidth*0.75), 0, bx + (Sint16)(barWidth*0.75), BAR_TEX_H-1, c_barDark );				
			}			

			//Draw_Line( m_surf, 0,0, BAR_TEX_W-1, BAR_TEX_H-1, c_red );
			//Draw_Line( m_surf, 0,BAR_TEX_H-1, BAR_TEX_W-1, 0, c_red );
		
			// update the opengl texture
			glTexImage2D( GL_TEXTURE_2D, 0, m_surf->format->BytesPerPixel,
							m_surf->w, m_surf->h, 0,
							GL_RGBA, GL_UNSIGNED_BYTE, m_surf->pixels );
					
			m_barsChanged = false;
		}		
		

		
		glColor3f( 1.0, 1.0, 1.0 );

		const float STEPS = 100;
		glBegin( GL_QUAD_STRIP );
		for (float t=0.0; t <= 1.0; t += (1.0f/STEPS) )
		{
			float a = t * 2.0f * (float)M_PI;
			float c = cos( a ), s = sin( a );
			glTexCoord2d( t, 1.0 );
			glVertex3f( s * m_displayRadius, c * m_displayRadius, 0.0f );

			glTexCoord2d( t, 0.0 );
			glVertex3f( s * (m_displayRadius+10.0f), c * (m_displayRadius+10.0f), 0.0f );
		}
		glEnd();

		
	}

//protected:
	int m_ring;	
	float m_displayRadius;

	SDL_Surface *m_surf;	
	GLuint m_glTex;
	bool m_barsChanged;

	// this track's channel
	FMOD::Channel *m_chan;
	std::vector<Bar> m_bar;
};

void chooseNextPart( std::vector<Track*> &tracks, int bar, int partLength, int & barsLeft )
{
	static int currPart = TRACK_RHYTHM;

	// If we're out of bars for this part, pick a new one
	if (!barsLeft)
	{
		// Pick a channel to record on the OLD barnum
		int ndx = 1 + (int)(((float)rand() / (float)RAND_MAX) * 3.0);
		currPart = ndx;
		barsLeft = partLength;
	}
						
	printf("track %d bar %d will record\n", currPart, bar);
	tracks[currPart]->setBarWillRecord( bar );
	barsLeft--;
}


//============================================================================
int main( int argc, char *argv[] )
{	

	#ifndef NDEBUG
	AllocConsole();
	SetConsoleTitle( L"miniLD9 CONSOLE" );
	freopen("CONOUT$", "w", stdout );
	#endif

	// Initialize SDL
	if (SDL_Init( SDL_INIT_NOPARACHUTE | SDL_INIT_VIDEO ) < 0 ) 
	{
		fprintf( stderr, "Unable to init SDL: %s\n", SDL_GetError() );
	}

	if (SDL_SetVideoMode( 800, 800, 32, SDL_OPENGL ) == 0 ) 
	{
		cout << "Unable to set video mode: " << SDL_GetError() << endl;
		exit(1);
	}
		

	SDL_WM_SetCaption( "MiniLD9 -- JamClock", NULL );

	// seed rand
	srand( (unsigned int)time(0) );

	// Init Fmod
	FMOD_RESULT res;	
	res = FMOD::System_Create( &g_fmod );
	assert( res == FMOD_OK );

	// init fmod
	res = g_fmod->init( 20, FMOD_INIT_NORMAL, 0 );

	if (!TTF_Init() != -1 )
	{
		printf("TTF_Init error: %s\n", TTF_GetError() );		
	}
	SDL_version compile_version;
	const SDL_version *link_version = TTF_Linked_Version();
	SDL_TTF_VERSION( &compile_version );
	printf("SDL_ttf -- link version %d.%d.%d compile version %d.%d.%d\n",
			link_version->major, link_version->minor, link_version->patch,
			compile_version.major, compile_version.minor, compile_version.patch );


	//=============================
	// Menu screen
	//=============================
	
	// TODO: menu screen


	// kick off the drumloop
	FMOD::Sound *drumloop;
	g_fmod->createSound( "drums_ad3_007_120bpm.wav", FMOD_SOFTWARE, 0, &drumloop );	
	//g_fmod->createSound( "drumloop5.wav", FMOD_SOFTWARE, 0, &drumloop );	
	//drumloop->setLoopCount( -1 );
	
	//g_fmod->playSound( FMOD_CHANNEL_FREE, drumloop, false, &chan );

	// Main stuff
	float bpm = 120;
	float trackRot = 0.0;
	int partLength = 8; // # of bars before switching parts
	int barsLeft = partLength;

	std::vector<Track*> tracks;
	Track drumTrack( TRACK_DRUMS );
	Track bassTrack( TRACK_BASSLINE );
	Track rhythmTrack( TRACK_RHYTHM );
	Track leadTrack( TRACK_LEAD );

	tracks.push_back( &drumTrack );
	tracks.push_back( &bassTrack );	
	tracks.push_back( &leadTrack );
	tracks.push_back( &rhythmTrack );

	// Put a dsp on the drum track to shift down
	FMOD::DSP *dsp;
	g_fmod->createDSPByType( FMOD_DSP_TYPE_PITCHSHIFT, &dsp );	
	dsp->setParameter( 0, 0.5 );

	// set up recording	
	int numRecDrivers;
	g_fmod->getRecordNumDrivers( &numRecDrivers );
	printf("Num Record drivers: %d\n", numRecDrivers );

	FMOD_CREATESOUNDEXINFO exinfo;
	memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    exinfo.cbsize           = sizeof(FMOD_CREATESOUNDEXINFO);
    exinfo.numchannels      = 2;
    exinfo.format           = FMOD_SOUND_FORMAT_PCM16;
    exinfo.defaultfrequency = 44100;
    exinfo.length           = exinfo.defaultfrequency * sizeof(short) * exinfo.numchannels * 5;

	// Chop drumloop up into samples
	unsigned int chopOffs = 0;
	unsigned int drumLength;
	drumloop->getLength( &drumLength, FMOD_TIMEUNIT_PCMBYTES );

	for (size_t i=0; i < drumTrack.m_bar.size(); ++i)
	{
		float sampleLenSecs = 4.0f / (bpm / 60.0f);
		printf("Sample Len: %f s\n", sampleLenSecs );
		exinfo.length = (unsigned int)(exinfo.defaultfrequency * sizeof(short) * exinfo.numchannels * sampleLenSecs);
		res = g_fmod->createSound(0, FMOD_2D | FMOD_SOFTWARE | FMOD_OPENUSER, 
												&exinfo, &(drumTrack.m_bar[i].m_sample) );

		// Copy data in from drum loop
		if (chopOffs < drumLength)
		{
			FMOD::Sound *sample = drumTrack.m_bar[i].m_sample;

			unsigned int len;
			sample->getLength( &len, FMOD_TIMEUNIT_PCMBYTES );
			void *data1, *data2;
			void *drumData1, *drumData2;
			unsigned int len1, len2, drumLen1, drumLen2;
			unsigned int chopLen = min( len, (drumLength - chopOffs) );
			sample->lock( 0, len, &data1, &data2, &len1, &len2 );
			drumloop->lock( chopOffs, chopLen, &drumData1, &drumData2, &drumLen1, &drumLen2 );

			memcpy( data1, drumData1, drumLen1 );

			drumloop->unlock( drumData1, drumData2, drumLen1, drumLen2 );
			sample->unlock( data1, data2, len1, len2 );			

			chopOffs += chopLen;
		}
	}

	// initialize willRecord flags... 
	for (size_t i=0; i < drumTrack.m_bar.size(); ++i)
	{							
			chooseNextPart( tracks, (int)i, partLength, barsLeft );		

#if 0
			// for debugging -- put some samples in
			FMOD::Sound *sndBass, *sndRhythm, *sndLead;
			g_fmod->createSound( "twinklysynth.wav", FMOD_SOFTWARE, 0, &sndBass );	
			g_fmod->createSound( "synthguitar.wav", FMOD_SOFTWARE, 0, &sndRhythm );	
			g_fmod->createSound( "determined303synth.wav", FMOD_SOFTWARE, 0, &sndLead );	
			tracks[1]->m_bar[i].m_sample = sndBass;
			tracks[2]->m_bar[i].m_sample = sndRhythm;
			tracks[3]->m_bar[i].m_sample = sndLead;
#endif

	}


	//=====[ Main loop ]======
	Uint32 ticks = SDL_GetTicks(), ticks_elapsed, sim_ticks = 0;
	bool done = false;
	float currBar = 0.0;
	int currBarNum = -1;

	bool paused = true;

	
	while(!done)
	{
		int muteRequest = -1;

		SDL_Event event;

		while (SDL_PollEvent( &event ) ) 
		{
			switch (event.type )
			{
				case SDL_KEYDOWN:

					switch( event.key.keysym.sym ) 
					{						
						case SDLK_ESCAPE:
							done = true;
							break;

						case SDLK_SPACE:
							trackRot = 0;
							paused = !paused;
#if 0
							g_fmod->playSound( FMOD_CHANNEL_FREE, drumloop, false, &(drumTrack.m_chan) );
							drumTrack.m_chan->setVolume( 0.5 );
							//drumTrack.m_chan->addDSP( dsp, NULL );							
							break;

						case SDLK_PERIOD:
							bpm += 1;
							printf("BPM: %f\n", bpm );
							g_fmod->playSound( FMOD_CHANNEL_REUSE, drumloop, false, &(drumTrack.m_chan) );
							trackRot = 0;
							break;

						case SDLK_COMMA:
							bpm -= 1;
							printf("BPM: %f\n", bpm );
							g_fmod->playSound( FMOD_CHANNEL_REUSE, drumloop, false, &(drumTrack.m_chan) );
							trackRot = 0;
#endif
							break;

						case SDLK_1:
							muteRequest = 0;
							break;

						case SDLK_2:
							muteRequest = 1;
							break;

						case SDLK_3:
							muteRequest = 2;
							break;

						case SDLK_4:
							muteRequest = 3;
							break;

						// TESTING
						case SDLK_d:
							{																
								//int numParams;
								//dsp->getNumParameters( &numParams );
								//for (int i=0; i < numParams; ++i)
								//{
								//	char name[20], label[20], desc[1000];
								//	float pmin, pmax;
								//	dsp->getParameterInfo( i, name, label, desc, 1000, &pmin, &pmax );
								//	printf("%d: %s %s - %s [%f, %f]\n\n", 
								//		i, name, label, desc, pmin, pmax );
								//}

								// Toggle pitch shift
								float pitchShift;
								dsp->getParameter( 0, &pitchShift, NULL, 0 );
								if (pitchShift > 0.75 )
								{
									// Octave drop
									dsp->setParameter( 0, 0.5 );
								}
								else
								{
									// No pitch change
									dsp->setParameter( 0, 1.0 );
								}
							}
							break;
					}
					break;
				case SDL_MOUSEMOTION:					
					break;

				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == 1)
					{					
						//
					}
					break;

				case SDL_MOUSEBUTTONUP:
					break;				

				case SDL_QUIT:
					done = true;
					break;
			}
		}

		// Did the user toggle a channel muted?
		if (muteRequest >= 0)
		{
			if (tracks[muteRequest]->m_chan)
			{
				bool isMute;
				tracks[muteRequest]->m_chan->getMute( &isMute );
				tracks[muteRequest]->m_chan->setMute( !isMute );

				// TODO: display greyed out or something
			}
		}
		
		// Continuous (key state) keys
		Uint8 *keyState = SDL_GetKeyState( NULL );
		//if (keyState[ SDLK_LEFT]) {}		

		// Timing
		ticks_elapsed = SDL_GetTicks() - ticks;
		ticks += ticks_elapsed;

		// Bars per second (rotation speed)
		//float barsPerSec = (bpm/60.0) / (drumTrack.m_bar.size());
		float barsPerSec = bpm/(4 * 60.0f);

		// fixed sim update
		sim_ticks += ticks_elapsed;
		while (sim_ticks > STEPTIME) 
		{
			sim_ticks -= STEPTIME;						

//			printf("update sim_ticks %d ticks_elapsed %d\n", sim_ticks, ticks_elapsed );			
			float dtStep = (float)STEPTIME / 1000.0f;
			
			// update dtStep...
			//float rotPerSec = (bpm / 60.0) // bps...
			//	              * (360 / (drumTrack.m_bar.size() * 4));			
			if (!paused)
			{
				// Update current bar
				currBar += barsPerSec * dtStep;
				if (currBar > drumTrack.m_bar.size())
				{
					currBar -= drumTrack.m_bar.size();
				}
				
				//printf( "currBar %f\n", currBar );
				//==========================================================
				// New Bar!
				//==========================================================
				int barNum = (int)floor( currBar );
				if (barNum != currBarNum)
				{
					if (currBarNum >=0)
					{
						chooseNextPart( tracks, currBarNum, partLength, barsLeft );
					}

					// Update the bar num
					currBarNum = barNum;
					printf("-- Bar %d --\n", currBarNum );

					// Need to record this time?
					bool needToRecord = false;
					for (size_t i=0; i < tracks.size(); ++i)
					{
						if (tracks[i]->m_bar[currBarNum].m_willRecFlag)
							needToRecord = true;
					}

					if (needToRecord)
					{
						// Stop the current recording just in case
						res = g_fmod->recordStop( 0 );
						if (res != FMOD_OK)
						{
							printf("recordStop failed!\n" );
						}						

						g_recSound = NULL;

						// Start recording the current bar						
						float sampleLenSecs = 4.0f / (bpm / 60.0f);
						printf("Sample Len: %f s\n", sampleLenSecs );
						exinfo.length = (unsigned int)(exinfo.defaultfrequency * sizeof(short) * exinfo.numchannels * sampleLenSecs);
						res = g_fmod->createSound(0, FMOD_2D | FMOD_SOFTWARE | FMOD_OPENUSER, 
												&exinfo, &g_recSound);

						//res = g_fmod->createSound(0, FMOD_2D | FMOD_SOFTWARE | FMOD_OPENUSER, 
						//						NULL, &g_recSound);

						if (res != FMOD_OK)
						{
							printf("Could not create sound!\n" );
						}												


						// start recording the new sample
						//g_fmod->recordStart( 0, g_recSound, false );
						res = g_fmod->recordStart( 0, g_recSound, false );
						if (res != FMOD_OK)
						{
							printf("recordStart failed!\n" );
						}

						// Give this to the "willRecord" bar
						// (skip drums)
						for (size_t i=0; i < tracks.size(); ++i)
						{
							if (tracks[i]->m_bar[currBarNum].m_willRecFlag)
							{
								tracks[i]->assignSound( currBarNum, g_recSound );
								tracks[i]->m_bar[currBarNum].m_willRecFlag = false;
							}							

							// Tell the track that it's a new bar
							tracks[i]->newBar( currBarNum );

							// Make sure the bass has the dsp
							if (i==TRACK_BASSLINE)
							{
								tracks[i]->m_chan->addDSP( dsp, NULL );							
							}
						}
					}

				}


				// get current rot from bars
				trackRot = (currBar / drumTrack.m_bar.size()) * 360.0f;
			}
		}	

		// update the waveform if needed		
		for (size_t i=0; i < tracks.size(); ++i)
		{
			for (size_t bar=0; bar < drumTrack.m_bar.size(); ++bar )
			{
				Track *trk = tracks[i];
				if ( (trk->m_bar[bar].m_sample) &&
					 (trk->m_bar[bar].m_sample != g_recSound) &&
					 (trk->m_bar[bar].m_wave==NULL) )
				{
					// Normalize and create waveform for drawing
					if (!trk->m_bar[bar].normalizeAndCreateWaveform())
					{
						// it's empty.. do something here.
						// TODO: remove the sample
					}
				}
			}
		}

		// redraw as fast as possible		
		float dtRaw = (float)(ticks_elapsed) / 1000.0f;
			
		glClearColor( 92 / 256.0, 88.0  / 256.0, 99.0 / 256.0, 1.0 );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		glOrtho( -100, 100, -100, 100, -1.0, 1.0 );

		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();

		glPushMatrix();

		glRotated( trackRot, 0.0, 0.0, 1.0 );

		// draw tracks
		for( size_t i=0; i < tracks.size(); ++i)
		{
			tracks[i]->adjustDisplayRadius( dtRaw );
			tracks[i]->draw();
		}

		glPopMatrix();

		// Draw the cursor bar		
		glDisable( GL_TEXTURE_2D );
		glLineWidth( 3.0 );
		
		glColor3f( 1.0f, 31 / 255.0f, 76 / 255.0f );
		glBegin( GL_LINES );
		glVertex3f( 0.0, 25.0, 0.0 );
		glVertex3f( 0.0, 90.0, 0.0 );
		glEnd();

		// Restore state
		glEnable( GL_TEXTURE_2D );
		glColor3f( 1.0, 1.0, 1.0 );

		// swap gl
		SDL_GL_SwapBuffers();

		// nudge fmod
		g_fmod->update();
	}

	SDL_Quit();	

	return 0;
}
