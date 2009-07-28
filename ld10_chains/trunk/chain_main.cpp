#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>

#include <deque>

#ifndef NDEBUG
#define ALLEGRO_USE_CONSOLE
#endif

#include <allegro.h>

#include <math.h>

unsigned char map[100][100];
volatile int ticks = 0;

// hertz
#define UPDATE_FREQ (30)

#define LINK_LEN (0.3)

// player state
enum 
{
	P_JUMP,
	P_WALK
};

enum
{
	FACE_LEFT,
	FACE_RIGHT
};

void ticker()
{
	ticks++;
}
END_OF_FUNCTION(ticker);

struct Link
{
	float x, y;
	float vx, vy;
	float fx, fy;
};

void apply_link_force( Link &A, Link &B, float dt )
{
	float cfx = B.x - A.x;
	float cfy = B.y - A.y;
	float d = sqrtf( cfx*cfx+cfy*cfy );
	cfx /= d; cfy /= d;	

	float cfLen = 40.0f;

	// compress link
	float fmag = (d - LINK_LEN);
	//fmag = pow( fmag, 2.0f) * cfLen;
	fmag *= cfLen;
	A.fx += cfx * fmag;
	A.fy += cfy * fmag;
	printf("fmag %f D %f\n", fmag, d );	
}


int main( int argc, char *argv[] )
{	
	BITMAP *backbuf;
	
	//=========================
	// load map
	unsigned char map2[100][100];
	FILE *fp = fopen( "gamedata\\testmap.dat", "rb" );
	if (!fp) 
	{
		allegro_message("Can't open gamedata\\testmap.dat!" );		
		exit(1);
	}
	size_t sz = fread( map2, 100*100, 1, fp );	

	fclose( fp );

	// doh.. flip map
	for (int j=0; j < 100; ++j)
	{
		for (int i=0; i < 100; ++i)
		{			
			map[i][j] = map2[j][i];
		}
	}


	//DBG: print map
#if 1
	for (int j=0; j < 20; ++j)
	{
		for (int i=0; i < 20; ++i)
		{
			printf("%02X ", map[i][j] );
		}
		printf("\n" );
	}
#endif
	
	//=========================
	allegro_init();

	install_keyboard();
	install_mouse();
	
	set_color_depth( 16 );
	if (set_gfx_mode(GFX_AUTODETECT_WINDOWED,640,400, 0, 0) != 0) {
	//if (set_gfx_mode(GFX_AUTODETECT_FULLSCREEN, 640,400, 0, 0) != 0) {
      set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
      allegro_message("Unable to set 320x200 graphic mode\n%s\n", allegro_error);
      return 1;
   }

	LOCK_VARIABLE( ticks );
	LOCK_FUNCTION( ticker );
	install_int_ex( ticker, BPS_TO_TIMER(UPDATE_FREQ) );

	//=========================
	// Load sprites
	BITMAP *tiles, *playerStrip, *player[10];

	int poffx= 18, poffy=32;
	float pxx, pyy;
	float vxx =0, vyy = 0; // velocity
	int px, py, pframe;
	int vx, vy; // view
	int fmx=0, fmy=0; // map floor
	int mmx=0, mmy=0; // map occupied square
	int pstate = P_JUMP;
	int pface = FACE_RIGHT;
	int justStartedJump = 0;
	std::deque<Link> chain;
	

	int animsubframe=0,animframe = 0; // 0..11

	pframe = 0;
	pxx = 3.5; pyy = 97.5; // start pos	
	
	tiles = load_bitmap( "gamedata\\tiles.bmp", NULL );
	playerStrip = load_bitmap( "gamedata\\hero.bmp", NULL );	
	if ((!tiles) || (!playerStrip))
	{
		allegro_message("Can't open graphics!" );		
		exit(1);
	}

	for (int i = 0; i < playerStrip->w / 32; i++)
	{
		player[i] = create_bitmap( 32, 32 );
		blit( playerStrip, player[i], i*32, 0, 0, 0, 32, 32 );
	}


	text_mode( -1 );	
	backbuf = create_bitmap( 320, 200 );
	while (1) {		
		// game draw
		//game_draw( backbuf );
		rectfill( backbuf, 0, 0, 320, 200, makecol(64,64,64) );

		// player pos
		px = (int)(pxx * 32);
		py = (int)(pyy * 32);		

		// center on player
		vx = px - 120;
		vy = py - 190;

		// draw tiles
		for (int j=0; j < 100; j++)
		{
			for (int i=0; i < 100; i++)
			{
				if (map[i][j] != 0xff)
				{
					masked_blit( tiles, backbuf, 
								 map[i][j]*32,0, 
								 (i*32)-vx, (j*32)-vy, 
								 32, 32 );
				}
#ifndef NDEBUG
				// active map square
				if ((i==fmx) && (j==fmy)) 
				{
					rect( backbuf, (i*32)-vx, (j*32)-vy, 
						((i*32)-vx)+32, ((j*32)-vy)+32, 
						makecol(255,0,0) );
				}				

				if ((i==mmx) && (j==mmy)) 
				{
					rect( backbuf, 
						((i*32)-vx)+1, ((j*32)-vy)+1, 
						((i*32)-vx)+30, ((j*32)-vy)+30, 
						makecol(0,255,0) );
				}				
#endif

			}
		}

		// draw player		
		if (pface == FACE_RIGHT)
		{
			draw_sprite( backbuf, player[pframe],
						 px - poffx - vx, py - poffy-vy );
		}
		else
		{
			draw_sprite_h_flip( backbuf, player[pframe],
						 px - poffx - vx, py - poffy-vy );			
		}

		// draw chain
		for (int i=1; i < chain.size(); i++ )
		{
			line( backbuf, 
				(int)(chain[i-1].x*32) - vx, (int)(chain[i-1].y*32) - vy,
				(int)(chain[i].x*32) - vx, (int)(chain[i].y*32) - vy,
				makecol( 255,180,20 ) );
#ifndef NDEBUG
			circle( backbuf, 
				(int)(chain[i].x*32) - vx, 
				(int)(chain[i].y*32) - vy, 
				2,
				makecol( 0, 255, 0 ) );
#endif
		}

		// draw chain goober at front of chain
		if (chain.size())
		{
			printf("chaingoob %d %d\n", 
				(int)(chain[0].x * 32) - vx, (int)(chain[0].y * 32) - vy );

			circlefill( backbuf, 
				(int)(chain[0].x * 32) - vx, (int)(chain[0].y * 32) - vy, 
				2 + (animframe % 3),
				makecol( 30, 200, 255 ) );
		}
								
		

		

// debug info
#ifndef NDEBUG
		putpixel( backbuf, px-vx, py-vy, makecol( 255,0,0 ) );

		char msg[1024];
		int mtile;
		if ( (fmx >=0 ) && (fmx < 100) && (fmy >=0 ) && (fmy < 100) )
		{
			mtile = map[fmx][fmy];
		}
		else
		{
			mtile = 0xee;
		}
		sprintf( msg, "PLR: %d %d map %d %d [%02X] offs %d %d", 
			px, py, fmx, fmy, mtile,
			poffx, poffy );
		textout( backbuf, font, msg, 8, 1, makecol( 255,255,255 ) );
		sprintf( msg, "P: %f %f v %f %f", pxx, pyy, vxx, vyy );
		textout( backbuf, font, msg, 8, 9, makecol( 255,255,255 ) );
		sprintf( msg, "A: %d %d", animframe, animsubframe );
		textout( backbuf, font, msg, 8, 20, makecol( 255,255,255 ) );
#endif
		textout_right( backbuf, font, "Z=Jump, X=Chain",
				 320,1, makecol( 255, 255, 255 ) );

		// swapbuf
		vsync();
		acquire_screen();
		stretch_blit( backbuf, screen, 0, 0, 320, 200, 0, 0, 640, 400 );
		release_screen();

		// update .. check for keys
		
		// one shot keys		
		if (keypressed()) {
			int k = readkey();
			float step = 1.0f / 32.0f;
			switch (k >> 8 )
			{
			case KEY_SPACE: 
			case KEY_Z: 
				if (pstate==P_WALK)
				{
					vyy = -6.0f;					
					fmy -= 1;
					pstate=P_JUMP;
					justStartedJump = 10;
					printf("jump JSJ %d\n", justStartedJump );
				}
				break;						
			}			
		}		

		// square we are stading on
		fmx = (px / 32);
		fmy = (py / 32);

		// map square we occupy
		mmx = (px / 32);
		mmy = (py-2) / 32;


		// continous keys
		if (key[KEY_LEFT] && !key[KEY_RIGHT])
		{
			pface = FACE_LEFT;			
			if (map[mmx-1][mmy] == 0xff) vxx = -5.2f;			
		}
		else if (key[KEY_RIGHT] && !key[KEY_LEFT])
		{
			pface = FACE_RIGHT;
			if (map[mmx+1][mmy] == 0xff) vxx = 5.2f;
		}
		else
		{
			vxx = 0.0f;
		}

		// Check if chain is chainifying
		static int chainRecharge = 0;
		static int chainStuck = 0;
		if (key[KEY_X])
		{
			if ((!chainRecharge) && (!chainStuck))
			{
				Link l;
				l.x = pxx;
				l.y = pyy - 0.5;
				l.vx = 10.0f * (pface == FACE_LEFT?-1.0:1.0);
				l.vy = -25.0f;
				l.fx = 0.0;
				l.fy = 0.0;
				chain.push_back( l );
				chainRecharge = 2;
				printf("chain %d\n", chain.size() );
			}
		}
		else
		{
			// clear chain if any
			if (chain.size())
			{
				chain.erase( chain.begin(), chain.end() );
			}
			chainStuck = 0;
		}

		// Update
		int ntick;		
		ntick = ticks;
		ticks = 0;
		while (ntick>0)
		{
			float dt = 1.0f / (float)(UPDATE_FREQ);			
			
			// square we are stading on
			fmx = (px / 32);
			fmy = (py / 32);

			// map square we occupy
			mmx = (px / 32);
			mmy = (py-2) / 32;
			
		
			if ((pstate==P_JUMP) && (!justStartedJump))
			{				
				if (map[fmx][fmy] != 0xff)
				{
					// solid ground
					pstate = P_WALK;
					printf("land jump JSJ %d\n", justStartedJump );
					vxx = vyy = 0;

					// cheat upwards
					int fmy2 = fmy;
					while (map[fmx][fmy2]!=0xff)
					{
						pyy -= (1.0f/32.0f);
						py = (int)(pyy * 32);
						fmy2 = (py/32);
					}
					pyy += (1.0f/32.0f);
				}
				else
				{
					printf("fall\n" );
					// gravity
					vyy += 15.8f * dt;

					// terminal velocity 
					if (vyy > 10.0f)
					{
						vyy = 10.0f;
					}
				}
			}
			else if (pstate==P_WALK)
			{
				// check if there is a tile beneath us..	
				if (map[fmx][fmy] == 0xff)
				{
					// fall..
					printf("fall off\n" );
					pstate = P_JUMP;
				}
			}

			// chain pulls on player
			if ((chainStuck) && (chain.size()))
			{
				vxx += chain.back().fx * dt;
				vyy += chain.back().fy * dt;
				
				if (map[fmx][fmy] != 0xff)
				{
					vyy += 15.8f * dt;				
				}
			}

			// any state.. check for horiz walls
			if ((vxx < 0.0f) && (map[mmx-1][mmy] != 0xff))
			{
				vxx = 0.0f;
			}
			if ((vxx > 0.0f) && (map[mmx+1][mmy] != 0xff))
			{
				vxx = 0.0f;
			}

			// check if we hit a roof
			if ((vyy < 0.0f) && (map[mmx][mmy-1] != 0xff))
			{
				vyy = 0.0f;
			}

			// integrate			
			pxx += vxx * dt;
			pyy += vyy * dt;			

			
			// update chain
			for (int i=0; i < chain.size(); i++)
			{
				// chain forces
				chain[i].vx += chain[i].fx * dt;
				chain[i].vy += chain[i].fy * dt;

				// chain drag
				chain[i].vx *= 0.85;
				chain[i].vy *= 0.85;

				// chain update
				chain[i].x += chain[i].vx * dt;
				chain[i].y += chain[i].vy * dt;				

				// reset chain (spring) forces...
				chain[i].fx = 0.0f;
				chain[i].fy = 0.0f;

				// next link
				if (i < chain.size()-1)
				{
					apply_link_force( chain[i], chain[i+1], dt );					
				}

				// prev link
				if (i > 0)
				{
					apply_link_force( chain[i], chain[i-1], dt );					
				}

				// gravity on chain
				if ((!chainStuck) || (i>0))
				{
					chain[i].fy += 15.8f;
				}
			}

			// check for sticky
			if (chain.size())
			{
				int cx = (int)chain[0].x;
				int cy = (int)chain[0].y;
				if (map[cx][cy] != 0xFF)
				{
					chain[0].vx = chain[0].vy = 0.0;
					chain[0].fx = 0.0;
					chain[0].fy = 0.0;
					chain[0].vx = 0.0;
					chain[0].vy = 0.0;
					chainStuck = 1;
				}

				// if the chain is stuck, the last link gets the player
				if (chainStuck)
				{					
					chain.back().x = pxx;
					chain.back().y = pyy - 0.5;
					//chain.back().vx = vxx;
					//chain.back().vy = vyy;
					//chain.back().fx = 0.0f;
					//chain.back().fy = 0.0f;

				}
			}

			animsubframe += 1;
			if (animsubframe > 4)
			{
				animsubframe = 0;
				animframe += 1;
				if (animframe > 11)
				{
					animframe = 0;
				}
			}

			// jumps get a few tick before 
			// they can land
			if (justStartedJump) justStartedJump--;
			if (chainRecharge) chainRecharge--;

			// tick
			ntick--;
		}		

		// update sprite based on activity
		if (pstate == P_JUMP)
		{
			pframe = 5;
		}
		else
		{
			if (vxx > 0.05)
			{
				pframe = 2 + (animframe % 3);
			}
			else if (vxx < -0.05)
			{
				pframe = 2 + (animframe % 3);
			}
			else
			{
				pframe = 0;
				if (animframe==10) pframe = 1;
			}			
		}

		// DBG 
#if 0
		pxx = (mouse_x /320.0f) * 100.0f;
		pyy = (mouse_y /200.0f) * 100.0f;
		if (pxx < 0.0f) pxx = 0.0f;
		if (pxx > 3200.0f) pxx = 100.0f;
		if (pyy < 0.0f) pyy = 0.0f;
		if (pyy > 3200.0f) pyy = 100.0f;
#endif


		// check for quit
		if (key[KEY_ESC]) {
			exit(0);
			break;
		}

		yield_timeslice();
	}
		
}
END_OF_MAIN();