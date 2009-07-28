#include <allegro.h>

#include <tinyxml.h>

#include <vector>

#include "buttonlands.h"
#include "map.h"
#include "random.h"

std::vector<BMLoc> allLocs;
std::vector<BITMAP*> mapIcons;

BITMAP *bmpMtns;

void init_map_locs( const char *filename)
{
	BMLoc loc;

	TiXmlDocument *xmlDoc = new TiXmlDocument( filename );

	if (!xmlDoc->LoadFile() ) {
		printf("ERR! Can't load %s\n", filename );
	}

	TiXmlElement *xLocationSet, *xLoc;
	TiXmlNode *xText;

	xLocationSet = xmlDoc->FirstChildElement( "locations" );
	assert( xLocationSet );

	xLoc = xLocationSet->FirstChildElement( "loc" );

	int ndx = 0;
	while (xLoc) {
		
		loc.name = xLoc->Attribute("name");
		const char *bg = xLoc->Attribute("bg");		
		
		if (!stricmp( bg, "forest")){
			loc.bgType = BG_FOREST;
		} else if (!stricmp( bg, "ruins")) {
			loc.bgType = BG_RUINS;
		} else if (!stricmp( bg, "town")) {
			loc.bgType = BG_TOWN;
		} else {
			printf("WARNING: Unknown location background %s\n", bg );
			loc.bgType = BG_FOREST;
		}

		xText = xLoc->FirstChild();

		if (xText) {
			loc.desc = xText->Value();
		} else {
			loc.desc = std::string("A mysterious (and violent) place.\n" );
		}

		allLocs.push_back( loc );
		xLoc = xLoc->NextSiblingElement( "loc" );
	}


	xmlDoc->Clear();
	delete xmlDoc;


	// Now initialize the icons
	mapIcons.push_back( load_bitmap( "gamedata/mapicon_forest.bmp", NULL ) );
	mapIcons.push_back( load_bitmap( "gamedata/mapicon_ruins.bmp", NULL ) );
	mapIcons.push_back( load_bitmap( "gamedata/mapicon_town.bmp", NULL ) );

	bmpMtns = load_bitmap( "gamedata/map_mtns.bmp", NULL );
}

// fills in ADJ 



struct Edge {
	float x1, y1, x2, y2;
	int count;
};

void circumcenter( Triangle &tri, float &ccx, float &ccy, float &rad ) {

	float d;
	float ax = tri.ax, ay = tri.ay;
	float bx = tri.bx, by = tri.by;
	float cx = tri.cx, cy = tri.cy;
	float ax2 = ax*ax, ay2 = ay*ay;
	float bx2 = bx*bx, by2 = by*by;
	float cx2 = cx*cx, cy2 = cy*cy;

	d = 2.0 * ( ay*cx + by*ax - by*cx - ay*bx - cy*ax + cy*bx );
	if (fabs(d) < 0.000001) {
		ccx = ax; ccy = ay;	    
	    rad = 0.0;
	} else {
		ccx = ( by*ax2 - cy*ax2 - by2*ay + cy2*ay +
			   bx2*cy + ay2*by + cx2*ay - cy2*by -
			cx2*by - bx2*ay + by2*cy - ay2*cy ) / d;


		ccy = ( ax2*cx + ay2*cx + bx2*ax - bx2*cx +
			   by2*ax - by2*cx - ax2*bx - ay2*bx -
			cx2*ax + cx2*bx - cy2*ax + cy2*bx) / d;

	    rad = sqrt( (ccx-ax)*(ccx-ax) + (ccy-ay)*(ccy-ay) );
	}

}

void add_edge( std::vector<Edge> &edge, float x1, float y1, float x2, float y2 ) 
{
	// is edge already there
	for (int i=0; i < edge.size(); i++) {

		int match = 0;
		if ( (fabs( edge[i].x1 - x1 ) < 0.1) &&
			 (fabs( edge[i].y1 - y1 ) < 0.1) &&
			 (fabs( edge[i].x2 - x2 ) < 0.1) &&
			 (fabs( edge[i].y2 - y2 ) < 0.1) ) {

			match = 1;
		} else if ( 
			 (fabs( edge[i].x1 - x2 ) < 0.1) &&
			 (fabs( edge[i].y1 - y2 ) < 0.1) &&
			 (fabs( edge[i].x2 - x1 ) < 0.1) &&
			 (fabs( edge[i].y2 - y1 ) < 0.1) ) {
			match = 1;
		}

		if (match) {
			//printf("Edge matches...count %d\n", edge[i].count );
			edge[i].count++;
			return;
		}
	}

	// didn't find it, add
	Edge e;
	e.count = 1;
	e.x1 = x1; e.y1 = y1;
	e.x2 = x2; e.y2 = y2;
	edge.push_back( e );
}

bool edge_match( int ax1, int ay1, int ax2, int ay2,
				 int bx1, int by1, int bx2, int by2 ) {
	if (  ( ((ax1==bx1) && (ay1==by1) &&
			 (ax2==bx2) && (ay2==by2)) )  ||

			( ((ax1==bx2) && (ay1==by2) &&
			(ax2==bx1) && (ay2==by1)) ) ) {

		return true;
	}
	return false;
}

DualEdge *BMMap::find_dual_edge( std::vector<Triangle> &tris, int andx, int bndx )
{
	int tndx[2], numt, i, a,b,c; 
	Triangle t;
	DualEdge *de;

	// accum tris with edge b,a
	numt = 0;
	for (i = 0; i < tris.size(); i++) {
		t = tris[i];
		
		if (edge_match( (int)t.ax, (int)t.ay, (int)t.bx, (int)t.by, 
			loc[andx].xpos, loc[andx].ypos, loc[bndx].xpos, loc[bndx].ypos ) ||

			edge_match( (int)t.bx, (int)t.by, (int)t.cx, (int)t.cy, 
			loc[andx].xpos, loc[andx].ypos, loc[bndx].xpos, loc[bndx].ypos ) ||

			edge_match( (int)t.cx, (int)t.cy, (int)t.ax, (int)t.ay, 
			loc[andx].xpos, loc[andx].ypos, loc[bndx].xpos, loc[bndx].ypos ) ) {

			tndx[numt++] = i;
		}
		
	}

	if (numt==2) {
		//printf("Found dual edge!\n");
		de = new DualEdge();
		
		i = tndx[0];
		de->x1 = ((int)(tris[i].ax) + (int)(tris[i].bx) + (int)(tris[i].cx)) / 3;
		de->y1 = ((int)tris[i].ay + (int)tris[i].by + (int)tris[i].cy) / 3;

		i = tndx[1];
		de->x2 = ((int)tris[i].ax + (int)tris[i].bx + (int)tris[i].cx) / 3;
		de->y2 = ((int)tris[i].ay + (int)tris[i].by + (int)tris[i].cy) / 3;
		return de;
	} else return NULL;
}

void BMMap::make_edges_delauney(  )
{
	for (int i=0; i < nloc; i++) {
		for (int j=0; j < nloc; j++) {
			adj[i][j].pass = 0;
		}
	}

	// DBG: fill in edges at random
#if 0
	for (i=0; i <10; i++) {
		int l1,l2;
		l1 = random( nloc );
		l2 = random( nloc );
		adj[l1][l2].pass = 1;
		adj[l2][l1].pass = 1;
	}
#endif

	std::vector<Triangle> tris, tris2;
	tris.push_back( Triangle() );
	tris.push_back( Triangle() );
	tris[0].ax = 0; tris[0].ay = 0;
	tris[0].bx = 800; tris[0].by = 0;
	tris[0].cx = 800; tris[0].cy = 600;

	tris[1].ax = 0;   tris[1].ay = 0;
	tris[1].bx = 0;   tris[1].by = 600;
	tris[1].cx = 800; tris[1].cy = 600;

	// edgelist
	std::vector<Edge> edge;

	for (int ndx=0; ndx < nloc; ndx++) {
		
		// init lists
		tris2.erase( tris2.begin(), tris2.end() );
		edge.erase( edge.begin(), edge.end() );
		

		// find all triangle whose circumcenter contains loc ndx
		for (i=0; i < tris.size(); i++) {
			float cx, cy, rad;

			circumcenter( tris[i], cx, cy, rad );
			float d = sqrt( (loc[ndx].xpos - cx) * (loc[ndx].xpos - cx) +
						    (loc[ndx].ypos - cy) * (loc[ndx].ypos - cy) );
			if (d <= rad) {
				// in triangle circumcenter, add to edgelist
				add_edge( edge, tris[i].ax, tris[i].ay, tris[i].bx, tris[i].by );
				add_edge( edge, tris[i].bx, tris[i].by, tris[i].cx, tris[i].cy );
				add_edge( edge, tris[i].cx, tris[i].cy, tris[i].ax, tris[i].ay );
				
			} else {
				// just keep the tri
				tris2.push_back( tris[i] );
				//printf("Keeping tri %i\n", i );
			}
		}

		// add a triangle for every edge appearing once in the list
		for (i=0; i < edge.size(); i++) {
			if ( edge[i].count == 1 ) {
				Triangle t;
				t.ax = loc[ndx].xpos;
				t.ay = loc[ndx].ypos;
				t.bx = edge[i].x1; t.by = edge[i].y1;
				t.cx = edge[i].x2; t.cy = edge[i].y2;
				tris2.push_back( t );

				//printf("constructing tri\n" );
			}
		}

		// update the list
		tris = tris2;				
	}


	// convert the tris to adjacency
	for (i=0; i < tris.size(); i++) {
		int andx, bndx, cndx;

		andx = -1; bndx = -1; cndx = -1;
		for (int j=0; j < nloc; j++) {
			if ( ((int)tris[i].ax == loc[j].xpos) &&
				 ((int)tris[i].ay == loc[j].ypos) ) andx = j;
			
			if ( ((int)tris[i].bx == loc[j].xpos) &&
				 ((int)tris[i].by == loc[j].ypos) ) bndx = j;
			
			if ( ((int)tris[i].cx == loc[j].xpos) &&
				 ((int)tris[i].cy == loc[j].ypos) ) cndx = j;
		}

		if ( (andx > 0) && (bndx >=0 )) {
			adj[andx][bndx].pass = 1;
			adj[bndx][andx].pass = 1;

			if (!adj[andx][bndx].de) {
				DualEdge *de = find_dual_edge( tris, andx, bndx );
				adj[andx][bndx].de = de;
				adj[bndx][andx].de = de;
			}

		}

		if ( (bndx > 0) && (cndx >=0 )) {
			adj[bndx][cndx].pass = 1;
			adj[cndx][bndx].pass = 1;

			if (!adj[bndx][cndx].de) {
				DualEdge *de = find_dual_edge( tris, bndx, cndx );
				adj[bndx][cndx].de = de;
				adj[cndx][bndx].de = de;
			}
		}

		if ( (cndx > 0) && (andx >=0 )) {
			adj[cndx][andx].pass = 1;
			adj[andx][cndx].pass = 1;

			if (!adj[cndx][andx].de) {
				DualEdge *de = find_dual_edge( tris, cndx, andx );
				adj[cndx][andx].de = de;
				adj[andx][cndx].de = de;
			}
		}
	}
	
}

bool BMMap::is_connected()
{
	int ndx=0;
	std::vector<int> nodes;
	int visited[MAX_LOC];

	for (int i=0; i < nloc; i++) {
		visited[i] = 0;
	}

	nodes.push_back( 0 );
	while (nodes.size()) {
		int curr = nodes.back();
		nodes.pop_back();

		visited[curr] = 1;

		for (i =0; i < nloc; i++) {
			if ( (adj[curr][i].pass) && (!visited[i]) ) 
			{
				nodes.push_back( i );
			}
		}
	}

	for ( i=0; i < nloc; i++) {
		if (!visited[i]) return false;
	}

	return true;

}

struct CanRemoveEdge {
	int a, b;
};

void draw_blockers( BITMAP *targ, BITMAP *stamp, int x1, int y1, int x2, int y2 ) 
{
	int num;
	int dx, dy, ndx;	
	dx = x2-x1; dy = y2-y1;

	num = sqrt( dx*dx + dy*dy ) / 6;

	for (int i=0; i < num; i++) {
		float t = frandom();
		ndx = random(6);
		int sz = random( -10, 20 );

		masked_stretch_blit( stamp, targ, 
					 16*ndx, 0,    // stamp start
					 16, 15, 

					 ((x1 + dx*t) - (8+sz/2)) + random(-10, 10), // random along line (jittered)
					 ((y1 + dy*t) - (8+sz/2)) + random(-10, 10),  
					 16 + sz, 16 + sz );
		
	}
}


void draw_road( BITMAP *targ, BITMAP *blankMap, int x1, int y1, int x2, int y2 ) 
{
	int x[4], y[4];
	float c[4];

	float xx = (x2-x1);
	float yy = (y2-y1);

	// bezier control points
	x[0] = x1; y[0] = y1;
	x[1] = x1 + xx*0.25; y[1] = y1 + yy*0.25;
	x[2] = x1 + xx*0.75; y[2] = y1 + yy*0.75;
	x[3] = x2; y[3] = y2;

	// jitter the middle points
	int jitterAmt = 50;
	x[1] += random( -jitterAmt, jitterAmt ); y[1] += random( -jitterAmt, jitterAmt );
	x[2] += random( -jitterAmt, jitterAmt ); y[2] += random( -jitterAmt, jitterAmt );

	//circlefill( targ, x[1], y[1], 5, makecol( 0, 255, 255 ) );
	//circlefill( targ, x[2], y[2], 5, makecol( 0, 255, 255 ) );



	// draw the curve
	// c(t) = p0 * (1-t)^3  + p1*3*t*(1-t)^2 + p2*3*t^2*(1-t) + p3*t^3
	float steps = sqrt( xx*xx + yy*yy ) / 2.0;
	int lastx = x1, lasty = y1;
	float xt, yt;
	for (int pass=0; pass <2; pass++) {
		for (float t=0.0; t < 1.0; t += (1.0/steps) ) {
			c[0] = pow( (1.0-t), 3.0 );
			c[1] = 3.0*t*pow( (1.0-t), 2.0 );
			c[2] = 3.0*pow(t, 2.0)*(1.0-t);
			c[3] = pow( t, 3.0 );

			xt = x[0]*c[0] + x[1]*c[1] + x[2]*c[2] + x[3]*c[3];
			yt = y[0]*c[0] + y[1]*c[1] + y[2]*c[2] + y[3]*c[3];

			//line( targ, lastx, lasty, (int)xt, (int)yt, makecol( 0, 255, 255 ) );

			if (pass==0) {
				circlefill( targ, (int)xt, int(yt), 4+random(2), makecol( 115, 41, 2 ) );		
			} else {
				circlefill( targ, (int)xt, int(yt), 2+random(2), 
					getpixel( blankMap, (int)xt, (int)yt ) );
			}

			lastx = (int)xt; lasty = (int)yt;
		}
	}



}

void BMMap::make_map( int num_loc, float spacing )
{
	int i,j;

	// init bitmaps	
	if (bmpMap) destroy_bitmap( bmpMap );


	bmpMap = load_jpeg( "gamedata/map.jpg" );
	
	BITMAP *blankMap = create_bitmap( 800, 600 );
	blit( bmpMap, blankMap, 0,0, 0,0, 800, 600 );

	nloc = num_loc;

	if (allLocs.size()==0) {
		init_map_locs( "gamedata/maplocs.xml" );
	}

	printf("allLocs size %d\n", allLocs.size() );

	// fill in the map locs
	int reject , count, ndx;
	for (i=0; i < nloc; i++) {

		reject = 1; count = 100;
		while ((reject) && (count)) {
			ndx = random( allLocs.size() );
			count--;

			reject = 0; 
			for (j=0; j < i; j++) {
				if (allLocs[ndx].name==loc[j].name) {
					reject = 1;
					break;
				}
			}

		}

		loc[i] = allLocs[ndx];
		loc[i].index = i;
		loc[i].heroVisited = 0;
	}

	// locate the locs
	for ( i=0; i < nloc; i++) {
		

		int good = 0;
		int x, y;
		float d;
		
				
		good = 0;
		while (!good) {
			x = random( 170, 678 );
			y = random( 75, 450 );			

			good = 1;
			for  ( j=0; j < i; j++) {
			
				float xx, yy;
				xx = (float)(loc[j].xpos - x);
				yy = (float)(loc[j].ypos - y);
				
				d = sqrt( xx*xx + yy*yy ) ;				
				if (d < spacing ) {
					good = 0;					
					break;
				}
			}
		}
				
		loc[i].xpos = x;
		loc[i].ypos = y;
	}


	// make the edges
	make_edges_delauney();


	// remove some edges 
	std::vector<CanRemoveEdge> cre;
	while (1) {
		cre.erase( cre.begin(), cre.end() );

		// which edges can we remove??
		for (i=0; i < nloc; i++) {
			for (j=0; j < i; j++) {

				if (adj[i][j].pass) {
					
					// can we remove i->j?
					adj[i][j].pass = 0;
					adj[j][i].pass = 0;

					if (is_connected()) {
						CanRemoveEdge foo;
						foo.a = i; foo.b = j;
						cre.push_back( foo );
					}


					// put the edge back (for now )
					adj[i][j].pass = 1;
					adj[j][i].pass = 1;

				}

			}
		}

		// no more edges to remove? (TODO: leave SOME loops)

		//printf("Cre size %d\n", cre.size() );
		if (cre.size() == 0 ) {
			break; 
		}

		// do remove the edge
		int ndx = random( cre.size() );
		i = cre[ndx].a;
		j = cre[ndx].b;

		adj[i][j].pass = 0;
		adj[j][i].pass = 0;

		// draw in the blockage
		if (adj[i][j].de) {
			DualEdge *de = adj[i][j].de;

			draw_blockers( bmpMap, bmpMtns, de->x1, de->y1, de->x2, de->y2 );

			//line( bmpMap, de->x1, de->y1, de->x2, de->y2, makecol( 255,0,0) );
		}
	}


	

	// draw the edges
	for ( i=0; i < nloc; i++) {
		for ( j=0; j < i; j++) {

			if (adj[i][j].pass) {


				draw_road( bmpMap, blankMap,
					loc[i].xpos, loc[i].ypos,
					loc[j].xpos, loc[j].ypos );

				//line( bmpMap, 
				//	loc[i].xpos, loc[i].ypos,
				//	loc[j].xpos, loc[j].ypos,
				//	makecol( 0,255,0 ) );

			}

		} 
	}


	// draw the icons on the map
	for ( i=0; i < nloc; i++) {

		BITMAP *ico = mapIcons[loc[i].bgType];

		int f=0;
		if (ico->w > 64) {
			f = random( ico->w / 64 );
		}
		masked_blit( ico, bmpMap, f*64,0, 
			  loc[i].xpos - 32, loc[i].ypos - 32,
			  64, 64 );
	}
	

	destroy_bitmap( blankMap );
}