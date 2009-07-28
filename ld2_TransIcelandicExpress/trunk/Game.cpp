#include "Game.h"

float TransIcelandicExpress::c_RoadLevel = 0.3f;
float TransIcelandicExpress::c_PlayerHeight = 0.03f;

const char *victoryStr[] = { "Roads", "Crates", "Sheep" };

TransIcelandicExpress::TransIcelandicExpress() {
	// constructor
	music = NULL;
	angle = 0.0;
	dot_init = false;

	levelListNum = 0;

	num_crates = 0;

	sgSetVec3( cameraPos, 0.0f, 0.013f, 0.2f );
	sgSetVec3( playerStartPos, 0.0f, 1.0f, 0.0f );

	sgSetVec4( light_pos, -1.0f, -1.0f, -1.0f, 0.0f );

	sgSetVec4( light_amb,  0.2f, 0.2f, 0.3f, 1.0f );
	sgSetVec4( light_diff, 1.0f, 1.0f, 0.8f, 1.0f );
	sgSetVec4( light_spec, 1.0f, 1.0f, 1.0f, 1.0f );

	sgSetVec4( ocean_diff, 0.4f, 0.5f, 0.7f, 1.0f );

	victory = CONNECT_ALL_ROADS;

	player = new Player();
	blueprint = new Road();

	showHelp = true;
	dbgPhysics = false;
	deathSnd = false;
	showLevels = false;

	roadX = 0;
	roadY = 0;

	cf_jump = 0.0;
	falling = false;
	jumping = false;
	onroad = false;
	elevating = false;

	checkStable = false;

	
	loadLevel( "Default.txt" );

#ifdef WIN32 
	LevelInfo info;
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile("./gamedata/levels/*.txt",&wfd);
	if (hFind != INVALID_HANDLE_VALUE) {	
	do {
		if (strcmp( "levels.txt", wfd.cFileName )) {
			char *fn = (char*)malloc( strlen( wfd.cFileName)+1);
			strcpy(fn , wfd.cFileName);
			info = preloadLevel (wfd.cFileName);
			levelInfo.push_back( info );
		}
	} while (FindNextFile(hFind, &wfd));
	FindClose(hFind);
	}
#endif
#ifdef LINUX
	 assert(0);
	//opendir... blah blah...
#endif

	//loadLevel( "./gamedata/levels/SimpleRoad.txt" );
	//loadLevel( "./gamedata/levels/Steps3a.txt" );
	//loadLevel( "./gamedata/levels/cw.txt" );
	//loadLevel( "./gamedata/levels/Challenge2.txt" );
}

void TransIcelandicExpress::do_quit()
{
	Mix_CloseAudio();
	SDL_WM_GrabInput( SDL_GRAB_OFF );
	SDL_ShowCursor( 1 );
	SDL_Quit();
	exit(0);
}

void TransIcelandicExpress::loadLevel( const char *lvlname ) {
	FILE *fp;
	char line[1000], cmd[100], type[100], filename[1000];
	IceFloe *floe;
	sgVec2 *pnt;
	float x, y;
	int num, npnt, i, j;

	sprintf( filename, "./gamedata/levels/%s", lvlname );
	fp = fopen( filename, "r" );
	if (!fp) {
		printf ("Cannot load level %s\n", filename );
		do_quit();
	}

	victory = CONNECT_ALL_ROADS;
	num_crates = 0;

	// clear any floes
	for (std::vector<IceFloe*>::iterator floei=floes.begin();
		floei != floes.end(); floei++ ) {

		delete *floei;
	}
	floes.erase( floes.begin(), floes.end() );

	sheep.erase( sheep.begin(), sheep.end() );
	crates.erase( crates.begin(), crates.end() );


	while (fgets( line, 1000, fp )) {
		if (line[0]=='#') continue;

		sscanf( line, "%s", cmd );
		if (!strcmp(cmd, "location")) {
			sscanf( line, "%*s %f %f %s\n", &x, &y, type );
			if (!strcmp( type, "StartPos" )) {
				sgSetVec3( player->pos, x, 0.05f, y );
				player->pos[1] = getHeight( player->pos );
				sgCopyVec3( playerStartPos, player->pos );
			} else if (!strcmp( type, "Sheep" )) {
				Sheep *shp;
				shp = new Sheep();
				sgSetVec3( shp->pos, x, 0.05f, y );
				shp->pos[1] = getHeight( shp->pos );
				sgSetVec4( shp->dbgDiffColor, 0.8f, 0.8f, 0.8f, 1.0f );
				sheep.push_back( shp );
			} else if (!strcmp( type, "Crate" )) {
				Crate *crt;
				crt = new Crate();
				sgSetVec3( crt->pos, x, 0.05f, y );
				crt->pos[1] = getHeight( crt->pos );
				if (getFloe( crt->pos )) {
					crt->onLand = 1;
				} else {
					crt->onLand = 0;
				}

				sgSetVec4( crt->dbgDiffColor, 1.0f, 0.9f, 0.4f, 1.0f );
				crates.push_back( crt );
			}

		} else if (!strcmp(cmd, "victory")) {
			sscanf( line, "%*s %s", cmd );
			printf("Victory condition is %s\n", cmd );
			if (!strcmp(cmd, "Connect")) {
				victory = CONNECT_ALL_ROADS;
			} else if (!strcmp(cmd, "KillSheep")) {
				victory = KILL_ALL_SHEEP;
			} else if (!strcmp(cmd, "GetCrates")) {
				victory = GET_ALL_CRATES;
			}
		} else if (!strcmp(cmd, "mapsize")) {

			// delete the old map if present
			for (j = 0; j < roadY; j++) {
				for (i = 0; i < roadX; i++) {
					delete road[i][j];
					road[i][j] = NULL;
				}
			}

			sscanf( line, "%*s %d %d", &roadX, &roadY );

			// make a new map
			for (j = 0; j < roadY; j++) {
				for (i = 0; i < roadX; i++) {				
					road[i][j] = new Road();
				}
			}
		} else if (!strcmp(cmd, "road_tiles")) {
			int pylon, roadway;

			sscanf( line, "%*s %d", &num );

			for (int n = 0; n < num; n++ ) {
				fgets( line, 1000, fp );
				sscanf( line, "%s %d %d %d %d", cmd, &i, &j, &roadway, &pylon ); 
				if (strcmp(cmd, "road")) {
					printf( "Error: Expecting 'road' got %s\n", line );
				} else {
					road[i][j]->pylon = pylon;
					road[i][j]->road = roadway;
					road[i][j]->base_height = c_RoadLevel * 0.5;

					sgSetVec3( road[i][j]->pos, float(i) + 0.5f, 0.0f, float(j) + 0.5f);
				}
			}

		} else if (!strcmp(cmd, "num_floes")) {
			sscanf( line, "%*s %d", &num );
			for (i = 0; i < num; i++) {
				fgets( line, 1000, fp );
				floe = new IceFloe();
				sscanf( line, "%s %f %f %d %d %s %d",
					cmd, &(floe->health), &(floe->height),
					&(floe->breakable), &(floe->anchored),
					type, &npnt );

				if (!strcmp(type, "Ice")) {
					floe->type = FLOE_ICE;
				} else if (!strcmp(type, "Land")) {
					floe->type = FLOE_LAND;
				} else if (!strcmp(type, "Blocker")) {
					floe->type = FLOE_BLOCKER;
				} else  {
					floe->type = FLOE_CUSTOM;
					// floe->custom model name = type
				}

				pnt = new sgVec2[ npnt ];
				for ( j = 0; j < npnt; j++ ) {
					fgets( line, 1000, fp );
					sscanf( line, "%f %f", &x, &y );
					pnt[j][0] = x;
					pnt[j][1] = y;
				}

				floe->build( npnt, pnt );
				delete [] pnt;

				floes.push_back( floe );
			}
		} else {
			printf ("Skipping command %s \n", cmd );
		}
	}

	// check initial stability
	checkStable = true;
	
}

void TransIcelandicExpress::computeTimeDelta( void ) {	
	static Uint32 last_tick=0;	

	Uint32 tick;
	tick = SDL_GetTicks();	

	if (last_tick==0) {		
		deltaT = 0.001f;
	} else {
		deltaT = (float)(tick - last_tick) / 1000.0f;
	}
	last_tick = tick;

}

void TransIcelandicExpress::simulate() {
	IceFloe *floe;
	sgVec3 fwd, right, up, frict, v, g;
	sgVec3 playerForce;
	float cf_accel = 1.0,  // m/s^2 
		  maxVel = 0.5,    // m/2
		  cf_gravity = 1.0,
		  cf_jumpDecay = 1.0,
		  cf_friction_land = 8.0,
		  cf_friction_ice = 5.0;
	float pv, 
		  ff; // friction fudge... more friction for slow objects

	float timestep = 0.01f;
	static float timeleft = 0.0f;	
	timeleft += deltaT;

	sgSetVec3( up, 0.0, 1.0, 0.0 );
	sgSetVec3( playerForce, 0.0, 0.0, 0.0 );	

	while (timeleft > timestep) {

		floe = getFloe( player->pos );
		
		sgCopyVec3( fwd, cameraPos );
		sgNegateVec3( fwd );
		fwd[1] = 0.0;
		sgNormalizeVec3( fwd );
		sgVectorProductVec3( right, fwd, up );

		// check for ocean death
		if ((!deathSnd)&&(player->pos[1] < -0.05 )) {
			deathSnd = true;
			play_sound( SFX_DEATH );
		}

		if (player->pos[1] < -3.0 * c_RoadLevel) {
			sgCopyVec3( player->pos, playerStartPos) ;
			sgSetVec3( player->vel, 0.0, 0.0, 0.0 );
			falling = false;
			jumping = false;
			deathSnd = false;
			cf_jump = 0.0;
		}
		
		if (elevating) {
			player->pos[1] += c_RoadLevel * timestep;
			if (player->pos[1] > c_RoadLevel) elevating = false;
		}
		
		// could be standing on a road 
		if  ((player->pos[1] > (c_RoadLevel - 0.03 )) &&
			(player->pos[1] <= (c_RoadLevel )) ) {
			
			// could land on a road.
			int i,j;
			i = int( floor(player->pos[0]));
			j = int( floor(player->pos[2]));
			if (road[i][j]->road) {
				onroad = true;
				falling = false;
				elevating = false;
				player->pos[1] = c_RoadLevel;
				cf_jump = 0.0; // no bouncing		
			} else {
				onroad = false;
			}
		} else {
			onroad = false;
		}

		if (!onroad) {
			if (!floe) {
				falling = true;
			} else {	
				// contact with flow
				float h = floe->height * c_RoadLevel;
				if (player->pos[1] < h) {

					// close to the edge?
					if (h - player->pos[1] < 0.05) {
						// catch the edge of the floe
						falling = false;
						player->pos[1] = floe->height * c_RoadLevel;
						cf_jump = 0.0; // no bouncing		
					} else {
						// bounce off
						player->vel[0] = -player->vel[0];
						player->vel[2] = -player->vel[2];
					}
				} else {
					falling = true;
				}
			}
		}

		pv = sgLengthVec3( player->vel ) ;

		if (!falling) {
			sgScaleVec3( fwd, cf_moveForward );
			sgAddVec3( playerForce, fwd );

			sgScaleVec3( right, cf_moveSideways );
			sgAddVec3( playerForce, right );

			if (jumping) {
				cf_jump = 20.0 + (40.0 * pv);
				jumping = false;
			}

			if (cf_jump > 0.0) {								
				sgSetVec3( g, 0.0, cf_jump, 0.0 );
				sgAddVec3( playerForce, g );
				cf_jump -= cf_jumpDecay * timestep;

				if (cf_jump <= 0.0) {
					jumping = false;
					cf_jump = 0.0;
				}
			}

			sgScaleVec3( playerForce, cf_accel * timestep );
			sgAddVec3( player->vel, playerForce );
			
			ff = (1.0 - ((pv / maxVel)* 0.8));
			ff = ff*ff;

			sgCopyVec3( frict, player->vel );
			sgNegateVec3( frict );
				
			if ((!onroad)&&(floe)&&(floe->type==FLOE_ICE)) {
				sgScaleVec3( frict, ff * cf_friction_ice * timestep );	
			} else {
				sgScaleVec3( frict, ff * cf_friction_land * timestep );	
			}

			sgAddVec3( player->vel, frict );		
		} else if (!elevating) {
			// falling
			sgSetVec3( g, 0.0, -1.0, 0.0 );
			sgScaleVec3( g, cf_gravity * timestep );

			sgAddVec3( player->vel, g );			
		}


		if (dbgPhysics) {
		dbgVel.push_back( pv );
			if (dbgVel.size() > 100 ) {
				dbgVel.pop_front();
			}
		}
		
		if ((!falling) && ( pv > maxVel )) {
			//printf("maxvel!\n" );
			sgNormalizeVec3( player->vel );
			sgScaleVec3( player->vel, maxVel );
		}

		sgCopyVec3( v, player->vel );
		sgScaleVec3( v, timestep );
		sgAddVec3( player->pos, v );
	
		// advance
		timeleft -= timestep;		
	}	


	//**** gameplay stuff, no physics here
	if (checkStable) {
		checkStable = false;
		checkStability();		
	}

	int i;
	SceneObject *thing;	
	for (i=0; i < brokenThings.size(); i++) {
		thing = brokenThings[i];

		//printf( " brokenThing %d y %f\n", i, brokenThings[i]->pos[1] );

		brokenThings[i]->pos[1] -= deltaT * brokenThings[i]->fallRate;
		if (brokenThings[i]->pos[1] < -(2.0f*c_RoadLevel) ) {
			//printf("Removing thing %d\n", i );

			brokenThings[i] = brokenThings[brokenThings.size()-1];
			brokenThings.pop_back();
			i--; // don't skip
		}
	}
	
	// pick up crates
	sgVec3 cd;	
	for (i = 0; i < crates.size(); i++) {
		float r;
		
		if (crates[i]->onLand) {
			r = SQR(0.05f);
		} else {
			r = SQR(0.3f);
		}
	
		sgSubVec3( cd, crates[i]->pos, player->pos );
		bool gotCrate = false;
		if ( sgLengthSquaredVec3( cd ) < r ) {
			num_crates ++;
			gotCrate = true;
			crates[i] = crates[crates.size()-1];
			crates.pop_back();
			i--; // don't skip
		}

		if (gotCrate) play_sound( SFX_GOT_CRATE );
	}

	// piss off sheep	
	for (i = 0; i < sheep.size(); i++) {
		float r;

		if (sheep[i]->annoyed) {
			r = SQR(0.15f);
		} else {
			r = SQR(0.3f);
		}
	
		sgSubVec3( cd, sheep[i]->pos, player->pos );		
		if ( sgLengthSquaredVec3( cd ) < r ) {
			if (!sheep[i]->annoyed) {
				sheep[i]->annoyed = 1;
				play_sound( SFX_BAA );
			} else {
				play_sound( SFX_BAA_SPLAT );

				// make an explosion
				SceneObject *bang = new SceneObject();
				sgCopyVec3( bang->pos, sheep[i]->pos );
				sgSetVec4( bang->dbgDiffColor, 1.0f, 0.9f, 0.0f, 1.0f );
				bang->fallRate = 0.1f;

				// damge the ice
				IceFloe *floe = getFloe( sheep[i]->pos );
				if (!floe) {
					printf( "Weird, couldn't find floe..\n");
				}
				if ( (floe) && (floe->breakable)) {
					floe->health -= 0.34f;
					if (floe->health <= 0.0) {
						floe->sinkDelay = 2.0;						
						floe->sinking = true;						
					}
				}
				
				brokenThings.push_back( bang );

				// get rid of the sheep
				sheep[i] = sheep[ sheep.size()-1];
				sheep.pop_back();
				i--; // don't skip				
			}
		}		
	}

	for (i = 0; i < floes.size(); i++) {

		if (floes[i]->sinking) {
			floes[i]->sinkDelay -= deltaT;

			if (floes[i]->sinkDelay < 0.0) {
				floes[i]->fallRate = 0.1f;
				brokenThings.push_back( floes[i] );

				// was it supporting a road?
				sgVec3 roadPos;
				int ii,jj;

				for (ii=0; ii < roadX; ii++) {
					for (jj=0; jj < roadY; jj++) {
						sgSetVec3( roadPos, float(ii)+0.05f, 0.0f, float(jj)+0.05f );
						if (floes[i]->contains( roadPos ) ) {
							fallRoad( ii, jj );
							checkStable = true;
						}
					}
				}

				// it's gone
				floes[i] = floes[floes.size()-1 ];
				floes.pop_back();						
				
			}
		}		
	}





	if (blueprint_on) {
		updateBlueprint();
	}
}



// kind of brute force, could be sped up
IceFloe *TransIcelandicExpress::getFloe( sgVec3 &pos ) {
	IceFloe *floe = NULL;

	for (std::vector<IceFloe*>::iterator floei=floes.begin();
		floei != floes.end(); floei++ ) {

		if ( (*floei)->contains( pos )) {
			floe = (*floei);
			break;
		}
	}

	return floe;	
}

float TransIcelandicExpress::getHeight( sgVec3 &pos ) {

	IceFloe *floe = getFloe( pos );
	if ((floe) && (floe->type != FLOE_BLOCKER)) { 		
		return floe->height * c_RoadLevel;
	} else {
		return 0.0;
	}	
}

void TransIcelandicExpress::play_sound( int sfxnum ) {
	sfx_chan[sfxnum] = Mix_PlayChannel(-1, sfx[ sfxnum], 0);
}

void TransIcelandicExpress::buildSomething() {
	int i,j;

	i = int( floor(blueprint->pos[0]));
	j = int( floor(blueprint->pos[2]));

	 

	if ((num_crates)&&(blueprint->building_pylon)) {
		num_crates--;		
		road[i][j]->pylon = 1;
	}

	if ((num_crates)&&(blueprint->building_road)) {
		num_crates--;		
		road[i][j]->road = 1;
	}

	checkStable = true;

	// dbg
	/*
	if ((road[i][j]->road) && (!road[i][j]->pylon)) {
		printf ("DBG Killing road\n");
		Road *newRoad;
		brokenThings.push_back( road[i][j] );
		newRoad = new Road();		
		sgCopyVec3( newRoad->pos, road[i][j]->pos );
		road[i][j] = newRoad;
	}
	*/
	
	updateBlueprint();
}

// this is a mess
void TransIcelandicExpress::checkStability() {
	int dist[MAX_ROAD][MAX_ROAD];
	int conn[MAX_ROAD][MAX_ROAD];
	int i,j, n,s,e,w, best;
	bool change, con;

	con = false;
	for (i=0; i < roadX; i++) {
		for (j=0; j < roadY; j++) {
			conn[i][j] = 0;
			if (road[i][j]->pylon) {
				dist[i][j] = 0;
				if (!con) {
					conn[i][j] = 1;
					con = true;
				}
			} else {
				dist[i][j] = -1;
			}
		}
	}

	change = true;
	while (change) {
		change = false;

#if 0
	for (i=0; i < roadX; i++) {
		for (j=0; j < roadY; j++) {
			if (!road[i][j]->road) {
				printf(".. " );
			} else {
				printf("%2d ", conn[i][j] );
			}
		}
		printf("\n");
	}
#endif

		for (i=0; i < roadX; i++) {
			for (j=0; j < roadY; j++) {

				if ((i>0) && (road[i-1][j]->road)) {						
						if (conn[i-1][j]) {
							if (!conn[i][j]){
								change = true;
								conn[i][j] = 1;
							}
						}
				}
						
				if ((i<roadX-1) && (road[i+1][j]->road)) {				
					if (conn[i+1][j]) {
						if (!conn[i][j]){
							change = true;
							conn[i][j] = 1;
						}
					}
				}
					

				if ((j<roadY-1) && (road[i][j+1]->road)) {				
					if (conn[i][j+1]) {
						if (!conn[i][j]){
							change = true;
							conn[i][j] = 1;
						}
					}
				}
				
				if ((j>0) && (road[i][j-1]->road)) {				
						if (conn[i][j-1]) {
							if (!conn[i][j]){
								change = true;
								conn[i][j] = 1;
							}
						}
				}

				if (dist[i][j] == -1) {
					
					if ((i>0) && (road[i-1][j]->road)) {
						w = dist[i-1][j];						
					} else {
						w = -1;					
					}

					if ((i<roadX-1) && (road[i+1][j]->road)) {
						e = dist[i+1][j];						
					} else {
						e = -1;					
					}

					if ((j<roadY-1) && (road[i][j+1]->road)) {
						s = dist[i][j+1];						
					} else {
						s = -1;					
					}

					if ((j>0) && (road[i][j-1]->road)) {
						n = dist[i][j-1];						
					} else {
						n = -1;	
					}

					best = dist[i][j];
					if (best < 0) best = 100;

					if (n>=0) { best = min( best, n ); }
					if (s>=0) { best = min( best, s ); }
					if (e>=0) { best = min( best, e ); }
					if (w>=0) { best = min( best, w ); }

					//printf(" nsew %d %d %d %d best %d\n", n, s, e, w, best ); 

					// closest distance 
					if ((best != 100) && (best!=-1)) {
						change = true;
						dist[i][j] = best+1;
					}

				}
			}
		}
	}
	
	
#if 0
	printf("------------------\n" );
	for (i=0; i < roadX; i++) {
		for (j=0; j < roadY; j++) {
			if (!road[i][j]->road) {
				printf(".. " );
			} else {
				printf("%2d ", dist[i][j] );
			}
		}
		printf("\n");
	}
#endif

	// make unstable roads fall
	allconnected = true;

	for (i=0; i < roadX; i++) {
		for (j=0; j < roadY; j++) {
			// check connectivity
			if ((road[i][j]->pylon) && (!conn[i][j])) {
				allconnected = false;
			}

			if ((road[i][j]->road) && 
				((dist[i][j] >= 2)||(dist[i][j]<0)) ) {

				// road must have opposide neighbors
				bool hasNbr = false;
				if ( (i>0) && (road[i-1][j]->road) &&
					 (i<roadX-1) && (road[i+1][j]->road) ) {
					hasNbr = true;
				}

				if ( (j>0) && (road[i][j-1]->road) &&
					 (j<roadY-1) && (road[i][j+1]->road) ) {
					hasNbr = true;
				}

				// unstable? then fall like leaves.. in fall..				
				if ( !hasNbr ) {
					//printf( "road %d %d is unsupported\n", i,j );

					fallRoad( i, j );					
				}
			}
		}
	}
}


void TransIcelandicExpress::fallRoad( int i, int j ) {
	Road *newRoad;
	brokenThings.push_back( road[i][j] );
	road[i][j]->fallRate = 0.1 +  ( (float)rand() / (float)RAND_MAX ) * 0.1;

	newRoad = new Road();		
	sgCopyVec3( newRoad->pos, road[i][j]->pos );
	road[i][j] = newRoad;

	// make sure to check again, this could cause future instablity
	checkStable = true;
}

void TransIcelandicExpress::updateBlueprint() {
	sgVec3 buildP, build_dir;
	int i, j;

	if ((!num_crates)||(falling)) {
		blueprint->road = 0;
		blueprint->pylon = 0;
		blueprint->building_road = 0;
		blueprint->building_pylon = 0;
		return;
	}

	sgSetVec3( build_dir, -cameraPos[0], 0.0, -cameraPos[2] );
			   
	sgNormalizeVec3( build_dir );
	sgScaleVec3( build_dir, 0.25 );

	sgAddVec3( buildP, player->pos, build_dir );
	i = int( floor(buildP[0]));
	j = int( floor(buildP[2]));

	blueprint->base_height = c_RoadLevel * 0.5;
	sgSetVec3( blueprint->pos, float(i) + 0.5f, 0.0f, float(j) + 0.5f);

	// build a pylon if there isn't one	
	if ((!onroad)&&(!road[i][j]->pylon) && (getFloe( blueprint->pos )) ) {
		blueprint->pylon = 1;
		blueprint->building_pylon = 1;	

	} else {
		blueprint->pylon = 0;
	}

	// build a road surface if there isn't one
	if ( (!road[i][j]->road) &&
		 ( (onroad) || (road[i][j]->pylon) ||
			((num_crates>=2) && (blueprint->building_pylon) )) ){		
		blueprint->road = 1;
		blueprint->building_road = 1;
	} else {
		blueprint->road = 0;
	}
}

ILuint ImageName;
unsigned int gl_tex_id;
void TransIcelandicExpress::redraw( void ) {	

	static tex_init = 0;	

	if (!tex_init) {		

		ilutRenderer( ILUT_OPENGL );

		ilGenImages( 1, &ImageName );

		ilBindImage( ImageName );
		glEnable( GL_TEXTURE_2D );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
		if (!ilLoadImage( "earth.png" )) {
			printf("Loading image failed\n");
		}

		
		gl_tex_id = ilutGLBindTexImage();
		ilutGLTexImage( 0 );

		tex_init = 1;
	}		

	glBindTexture (GL_TEXTURE_2D, gl_tex_id );

	glClearColor( 0.3f, 0.4f, 0.6f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	//glEnable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );

	// 3d part
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 60.0, 800.0/600.0, 0.05, 100.0 );



	gluLookAt(	player->pos[0]+cameraPos[0], 
				player->pos[1]+cameraPos[1]+c_PlayerHeight, 
				player->pos[2]+cameraPos[2],

				player->pos[0], 
				player->pos[1]+ c_PlayerHeight,
				player->pos[2],

				0.0, 1.0, 0.0 );
				

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	// apply some of the camera xform to the light just to make
	// it more shiny
	glPushMatrix();
	float viewHead = atan2( cameraPos[2], cameraPos[0] );	
	glRotated( viewHead * SG_RADIANS_TO_DEGREES, 0.0f, 1.0f, 0.0f );
	setupLights();
	glPopMatrix();

	draw3d();

	// 2d part
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();	
	gluOrtho2D( 0, 800, 0, 600 ) ;

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glDisable( GL_LIGHTING );
	draw2d();

	SDL_GL_SwapBuffers();

}

void TransIcelandicExpress::drawOcean( void ) {
	float sz = 1000.0;
	glPushMatrix();	
	glColor3f( 0.2f, 0.35f, 0.4f );
	//glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ocean_diff );

	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_2D );
	glDepthMask( 0 );

	glBegin( GL_POLYGON );
	glVertex3f(  sz, 0.0, -sz );
	glVertex3f( -sz, 0.0, -sz );
	glVertex3f( -sz, 0.0,  sz );
	glVertex3f(  sz, 0.0,  sz );
	glEnd();
	glPopMatrix();

	glDepthMask( 1 );
}

void TransIcelandicExpress::setupLights( void ) {	
	// lighting
	glLightfv( GL_LIGHT0, GL_POSITION, light_pos );
	glLightfv( GL_LIGHT0, GL_AMBIENT, light_amb );
	glLightfv( GL_LIGHT0, GL_DIFFUSE, light_diff );
	glLightfv( GL_LIGHT0, GL_SPECULAR, light_spec );

	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
}

void TransIcelandicExpress::draw3d( void ) {	

	
	glPushMatrix();		

	// The ocean
	drawOcean();

	glEnable( GL_LIGHTING );

	// draw the ice floes
	for (std::vector<IceFloe*>::iterator floei=floes.begin();
		floei != floes.end(); floei++ ) {

		(*floei)->draw();
	}

	// draw the sheep
	for (std::vector<Sheep*>::iterator shpi=sheep.begin();
		shpi != sheep.end(); shpi++ ) {

		(*shpi)->draw();
	}

	// draw the crates
	for (std::vector<Crate*>::iterator crti=crates.begin();
		crti != crates.end(); crti++ ) {

		(*crti)->draw();
	}

	// draw the roadways
	for (int j = 0; j < roadY; j++) {
		for (int i=0; i < roadX; i++) {
			road[i][j]->draw();
		}
	}

	// draw broken things
	for (std::vector<SceneObject*>::iterator bi=brokenThings.begin();
		bi != brokenThings.end(); bi++ ) {

		(*bi)->draw();
	}

	if (blueprint_on) {
		blueprint->draw();
	}

	// draw the player();
	player->draw();


	glPopMatrix();

#if 0
	// Texture test (spinning logo)
	glEnable( GL_TEXTURE_2D );
	glPushMatrix();

	glTranslated( 0.0, 0.0, -3.0 );
	glRotated( angle, 0.0, 1.0, 0.0 );
	
	angle = angle + deltaT * (360.0f / 4.0f);

	glColor3f( 1.0, 1.0, 1.0 );
	glBegin( GL_QUADS );

	glTexCoord2d( 0.0, 1.0 );
	glVertex3f( -1.0, -1.0, 0.0 );

	glTexCoord2d( 0.0, 0.0 );
	glVertex3f( -1.0,  1.0, 0.0 );

	glTexCoord2d( 1.0, 0.0 );
	glVertex3f(  1.0,  1.0, 0.0 );

	glTexCoord2d( 1.0, 1.0 );
	glVertex3f(  1.0, -1.0, 0.0 );
	glEnd();

	glPopMatrix();
#endif

}	

LevelInfo TransIcelandicExpress::preloadLevel( char *name ) {
	LevelInfo info;
	FILE *fp;
	char line[1000], fn[500], cmd[100];
	int lines_left = 10;

	info.name = (char*)malloc( strlen(name)+1);
	strcpy( info.name, name );

	info.desc = NULL;
	info.final = 0;
	info.victory = CONNECT_ALL_ROADS;

	sprintf( fn, "./gamedata/levels/%s", name );
	fp = fopen( fn, "r" );
	while ((lines_left)&&(fgets( line, 1000, fp ))) {
		sscanf( line, "%s", cmd );
		if (!strcmp( cmd, "level_desc")) {
				info.desc = (char*)malloc( strlen(line));
				strcpy( info.desc, line+11 );
		} 
		if (!strcmp( cmd, "level_final")) {
			sscanf( line, "%*s, %d", &(info.final) );
		} else if (!strcmp(cmd, "victory")) {
			sscanf( line, "%*s %s", cmd );			
			if (!strcmp(cmd, "Connect")) {
				info.victory = CONNECT_ALL_ROADS;
			} else if (!strcmp(cmd, "KillSheep")) {
				info.victory = KILL_ALL_SHEEP;
			} else if (!strcmp(cmd, "GetCrates")) {
				info.victory = GET_ALL_CRATES;
			}
		}

		lines_left--;
	}
	fclose( fp );

	return info;
}


void TransIcelandicExpress::draw2d( void ) {	

	char buf[1024];

	glEnable( GL_TEXTURE_2D );
	glDisable( GL_CULL_FACE );
	glDisable ( GL_DEPTH_TEST );


	fnt->begin();
	
	
	glColor3f ( 1.0f, 0.85f, 0.4f ) ;
	fnt->setPointSize ( 20 ) ;
	fnt->start2f ( 650, 550 ) ;		
	sprintf( buf, "Crates: %d\n", num_crates );
	fnt->puts( buf );		

	
	glColor3f ( 0.3f, 1.0f, 1.0f ) ;
	fnt->setPointSize ( 20 ) ;
	fnt->start2f ( 650, 520 ) ;			

	sprintf( buf, "Blueprint:%s",blueprint_on?"ON":"OFF" );
	fnt->puts( buf);	
	
	fnt->end();

	// check for victory condition
	char *victoryCond = NULL;
	bool winner = false;
	if (victory==CONNECT_ALL_ROADS) {
		victoryCond = "Connect all roads to win this level.";
		if (allconnected) winner = true;
	} else if (victory==KILL_ALL_SHEEP) {
		victoryCond = "Kill all sheep to win this level.";
		if (sheep.size()==0) winner = true;
	} else if (victory==GET_ALL_CRATES) {
		victoryCond = "Get all the crates to win this level.";
		if (crates.size()==0) winner = true;
	}

	if (winner) {
		glColor3f ( 1.0f, 0.0, 1.0f ) ;
		fnt->setPointSize ( 100 ) ;
		fnt->start2f ( 300, 300 ) ;				
		fnt->puts( "YOU\nWIN" );	
		fnt->end();
	}


	if (dbgPhysics) {
		glColor3f ( 0.6f, 1.0f, 1.0f ) ;
	
		if (falling) {				
			fnt->start2f ( 20, 50 ) ;	
			fnt->puts( "Falling" );
		}		

		if (elevating) {				
			fnt->start2f ( 20, 100 ) ;	
			fnt->puts( "Elevating" );
		}

		if (allconnected) {				
			fnt->start2f ( 20, 150 ) ;	
			fnt->puts( "Connected" );
		}

		fnt->end();
	}

	if (showLevels) {
		fnt->begin();
	
		glColor3f ( 0.6f, 1.0f, 1.0f ) ;
		fnt->setPointSize ( 40 ) ;
		fnt->start2f ( 20, 550 ) ;	
		fnt->puts( "Trans-Icelandic Expressway" );

		glColor3f ( 1, 1, 1 ) ;
		fnt->setPointSize ( 20 ) ;
		fnt->start2f ( 20, 530 ) ;	
		fnt->puts(	"Ludum Dare 48 hour game competition\n");					

		for (int i = levelListNum; i < min(levelInfo.size(), levelListNum+9); i++) {
			sprintf( buf, "%-2d) %s", i-levelListNum, levelInfo[i].name );
			fnt->start2f ( 40, 450 - (i-levelListNum)*20) ;	
			fnt->puts( buf );

			fnt->start2f ( 250, 450 - (i-levelListNum)*20) ;
			sprintf( buf, "%s", victoryStr[ levelInfo[i].victory ] );
			fnt->puts( buf );


			if (levelInfo[i].desc) {
				fnt->start2f ( 330, 450 - (i-levelListNum)*20) ;
				sprintf( buf, "%s", levelInfo[i].desc );
				fnt->puts( buf );
			}				
		}
		fnt->start2f ( 40, 450 - 180) ;	
		fnt->puts( "9) More..." );

		fnt->end();
	}

	if ((!showLevels)&&(showHelp)) {
		fnt->begin();
	
		glColor3f ( 0.6f, 1.0f, 1.0f ) ;
		fnt->setPointSize ( 40 ) ;
		fnt->start2f ( 20, 550 ) ;	
		fnt->puts( "Trans-Icelandic Expressway" );

		glColor3f ( 1, 1, 1 ) ;
		fnt->setPointSize ( 20 ) ;
		fnt->start2f ( 20, 530 ) ;	
		fnt->puts(	"Ludum Dare 48 hour game competition\n"
					"Joel Davis (joeld42@yahoo.com)\n"
					"Controls: w,a,s,d - move around\n"
					"          e, RETURN - build blueprint\n"
					"          SPACE - Toggle blueprint (need crates)\n"
					"          click - Jump (elevator near pylon)\n" 
					"          home - save screenshot\n"
					"          l - choose another level\n"
#ifdef MUSIC
					"          m - toggle music\n"					
#endif
					"          h - toggle help\n" 
					"          p - physics dbg\n" 
					"* Sheep are timid and explode when frightened.\n"
					"* Explosions can damage ice floes, be careful.\n"
					"* Roads must be adjacent to a pylon or have a road on two opposite\n"
					"  sides or it will fall.\n"
					) ;
		

		glColor3f ( 0.8f, 1.0f, 0.8f ) ;
		fnt->setPointSize ( 20 ) ;
		fnt->start2f ( 200, 50 ) ;	
		fnt->puts( victoryCond );
		fnt->end();	
	}

	if (dbgPhysics) {
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_DEPTH_TEST );
		glBegin( GL_LINE_STRIP );
		float v;
		glColor3f( 0.0, 1.0, 0.0 );
		for (int i = 0; i < dbgVel.size(); i++) {
			v = dbgVel[i];
			glVertex3f( i * 8.0f, v*600.0, 0.0 );
		}
		glEnd();
	}

	glEnable ( GL_DEPTH_TEST );
}

void TransIcelandicExpress::initialize() {
	musicOn = false;

	sfx[SFX_DEATH]     = Mix_LoadWAV("./gamedata/sounds/drown.wav");
	sfx[SFX_NO_CRATES] = Mix_LoadWAV("./gamedata/sounds/no_crates.wav");	
	sfx[SFX_GOT_CRATE] = Mix_LoadWAV("./gamedata/sounds/got_crate.wav");
	sfx[SFX_BAA]       = Mix_LoadWAV("./gamedata/sounds/baa.wav");	
	sfx[SFX_BAA_SPLAT] = Mix_LoadWAV("./gamedata/sounds/bah_splat.wav");	
	

#ifdef MUSIC
	music = Mix_LoadMUS( "./gamedata/sounds/template_music.ogg" );
	if (!music) {
		printf("Error loading music %s\n", Mix_GetError() );
	} else {
		printf("Music loaded\n");
	}
	//Mix_PlayMusic( music, -1 );
	//musicOn = 1;
#endif 

}

void TransIcelandicExpress::mouseMotion( int xrel, int yrel ) {
	// rotate the camera
	sgVec3 up, right, dir, oldCamPos;
	sgMat4 m;
	float rot_factor = 0.5;
	float camDist = sgLengthVec3( cameraPos );

	sgNormalizeVec3( cameraPos );

	// change heading
	sgSetVec3( up, 0.0, 1.0, 0.0 );
	sgMakeRotMat4( m, (float)xrel * rot_factor, up ) ;

	sgXformVec3( cameraPos, m ) ;

	// change pitch
	sgSetVec3( dir, cameraPos[0], 0.0, cameraPos[2] );
	sgNormalizeVec3( dir );
	sgVectorProductVec3( right, up, dir ); // get the "r" direction	

	sgMakeRotMat4( m, (float)yrel * rot_factor, right ) ;

	sgCopyVec3( oldCamPos, cameraPos );
	sgXformVec3( cameraPos, m ) ;
	
	if ((cameraPos[1] < 0.0 ) || (cameraPos[1] > 0.8)) {
		// don't let the camera go below player or straight down
		sgCopyVec3( cameraPos, oldCamPos );		
	}

	sgScaleVec3( cameraPos, camDist );

}


void TransIcelandicExpress::eventLoop() {
	while (1) {		
    SDL_Event event;

    /* Grab all the events off the queue. */
    while( SDL_PollEvent( &event ) ) {
		
        switch( event.type ) {
		case SDL_MOUSEBUTTONDOWN:
		
			if (event.button.button == SDL_BUTTON_LEFT ) {

				// elevate if we're by a pylon
				int i, j;
				float ii,jj;
				i = int( floor(player->pos[0]));
				j = int( floor(player->pos[2]));
			
				ii = player->pos[0] - i;
				jj = player->pos[2] - j;
				if ( (ii > 0.25) && (ii < 0.75) &&
					 (jj > 0.25) && (jj < 0.75) &&
					 (road[i][j]->pylon) && 
					 (road[i][j]->road) ) {				
					
					elevating = true;
				

				} else {
					jumping = true;
				}
			} else if ((SDL_BUTTON(event.button.button) == SDL_BUTTON(4)) ||
					   (SDL_BUTTON(event.button.button) == SDL_BUTTON(5)) ) {				

				float scale;
				if (SDL_BUTTON(event.button.button) == SDL_BUTTON(5)) {
					scale = 1.1f;
				} else {
					scale = 0.9f;
				}

				sgScaleVec3( cameraPos, scale );
			}
			break;

		case SDL_MOUSEMOTION:			
			mouseMotion( event.motion.xrel, event.motion.yrel );
			
			break;
        case SDL_KEYDOWN:
			int k;

            switch( event.key.keysym.sym ) {
				case SDLK_ESCAPE:
					do_quit();
					break;
				case SDLK_SPACE: // toggle blueprint					
					if (!num_crates) {
						play_sound( SFX_NO_CRATES );
					} else {
						blueprint_on = !blueprint_on;						
					}
					break;

				case SDLK_e:
				case SDLK_RETURN:
					// build building
					if (blueprint) {
						if (!num_crates) {
							play_sound( SFX_NO_CRATES );
						} else {
							//play_sound( SFX_BUILDING );
							buildSomething();	
						}
					}
					break;

				case SDLK_l:
					showLevels = !showLevels;
					break;
				case SDLK_0:										
				case SDLK_1:					
				case SDLK_2:					
				case SDLK_3:					
				case SDLK_4:					
				case SDLK_5:					
				case SDLK_6:					
				case SDLK_7:
				case SDLK_8:
					if (showLevels) {
						k = (event.key.keysym.sym - '0') + levelListNum;
						printf("loadLevel %d\n", k );
						if (k < levelInfo.size()) {
							loadLevel( levelInfo[k].name ); 
						}
						showLevels = false; 
					}
					break;
				case SDLK_9:
					if (showLevels) {
						levelListNum += 9;
						if (levelListNum >= levelInfo.size()) levelListNum = 0;
					}
					break;

				



				// cheats
				case SDLK_c:
					printf("Cheat: get crate\n") ;
					num_crates++;
					break;



				case SDLK_HOME:
					ilutGLScreenie();
					break; 

				case SDLK_h:
					showHelp = !showHelp;
					break;

				case SDLK_p:
					dbgPhysics = !dbgPhysics;
					break;

#ifdef MUSIC
				case SDLK_m:					
					if (!musicOn) {
						printf("Playing\n");
						Mix_PlayMusic( music, -1 );
						musicOn = 1;
					} else {
						printf("Halting\n");
						Mix_HaltMusic();						
						musicOn = 0;
					}
					break;
#endif
				default:
					break;
				}
			break;

			case SDL_QUIT:
				/* Handle quit requests (like Ctrl-c). */
				do_quit();
				break;
			}	
		}

		// check non-instant keys
		cf_moveForward = 0.0;
		cf_moveSideways = 0.0;

		Uint8 *keys;
		keys = SDL_GetKeyState(NULL);
		if (keys[SDLK_w] && !keys[SDLK_s]) {
			cf_moveForward = 1.0;
		}
		if (!keys[SDLK_w] && keys[SDLK_s]) {
			cf_moveForward = -1.0;
		}
		if (keys[SDLK_a] && !keys[SDLK_d]) {
			cf_moveSideways = -1.0;
		}
		if (!keys[SDLK_a] && keys[SDLK_d]) {
			cf_moveSideways = 1.0;
		}		

		computeTimeDelta();
		simulate();
		redraw();
	}
}
