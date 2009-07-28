
#include "IceFloe.h"
#include "Game.h"
#include "TIE.h"

// useful funcs
bool ccwN( sgVec2 dir1, sgVec2 dir2 ) {
	return (dir1[0]*dir2[1] - dir2[0]*dir1[1]) > 0;
}

bool ccw( sgVec2 dir1, sgVec2 dir2 ) {
	sgNormalizeVec2( dir1 );
	sgNormalizeVec2( dir2 );
	return ccwN(dir1,dir2);
}

bool ccw( sgVec2 a, sgVec2 b, sgVec2 c ) {
	sgVec2 d1, d2;
	sgSubVec2( d1, b, a );
	sgSubVec2( d2, c, b );
	return ccw( d1, d2 );
}


IceFloe::IceFloe() : SceneObject() {
	
	sinking = false;
	sgSetVec4( landColor,    0.4f, 0.8f, 0.4f, 1.0f );
	tscale = 5.0;
}

void IceFloe::drawGeom() {	
	int i, j;
	float h;

	if ((type==FLOE_BLOCKER)||(type==FLOE_CUSTOM)) return;

	if (breakable) {
		if (health < 0.0) {
			sgSetVec4( dbgDiffColor, 0.5f, 0.3f, 0.3f, 1.0f );
		} else {
			sgSetVec4( dbgDiffColor, 0.0f, health, 1.0f, 1.0f );
		}
	} else {
		sgSetVec4( dbgDiffColor, 0.4f, 0.8f, 1.0f, 1.0f );
	}
	// tmp
	sgSetVec4( dbgDiffColor, 1.0f, 1.0f, 1.0f, 1.0f );

	glEnable( GL_TEXTURE_2D );

	h = height * TransIcelandicExpress::c_RoadLevel;	

	if (type==FLOE_LAND) {
		glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, landColor );
	}

	// draw the sides	
	sgVec3 edge, n, up;
	sgSetVec3( up, 0.0, 1.0, 0.0 );
	glBegin( GL_QUADS );
	float s = 0.0, elen;
	for (j = npnts-1, i = 0; i < npnts; j=i++) {
		sgSetVec3( edge, pnts[j][0] - pnts[i][0], 0.0,
						 pnts[j][2] - pnts[i][2] );

		elen = sgLengthVec3(edge);

		sgNormalizeVec3( edge );
		sgVectorProductVec3( n, edge, up );
	
		glNormal3f( n[0], n[1], n[2] );

		glTexCoord2f( s*tscale, h*tscale );
		glVertex3f( pnts[i][0], h, pnts[i][2] );

		glTexCoord2f( (s+elen)*tscale, h*tscale );
		glVertex3f( pnts[j][0], h, pnts[j][2] );

		glTexCoord2f( (s+elen)*tscale, 0.0 );
		glVertex3f( pnts[j][0], 0.0, pnts[j][2] );

		glTexCoord2f( s*tscale, 0.0 );
		glVertex3f( pnts[i][0], 0.0, pnts[i][2] );						
	}
	glEnd();

	// tesselate the top
	if (!tess.size()) {
		TessTri *tri;

		// first, do we need to reverse the order of the polygon?
		// check the signed area
		float area;
		area = 0.0;
		for (j=npnts-1,i=0; i < npnts; j = i++) {			
			area += (pnts[j][0]-pnts[i][0])*(pnts[j][2] + pnts[i][2])* 0.5;
		}		
				

		// tesselate it
		assert( npnts < 1000 );
		int used[1000];
		int pntsleft = npnts;
		for (i=0; i < npnts; i++) {
			pnts[i][1] = h;
			used[i] = 0;
		}

		int a, b, c;
		if (area < 0.0) {
			a=2; b = 1; c = 0;
		} else {
			a=0; b = 1; c = 2;
		}
		sgVec2 A, B, C;
		int iter = 1000; // should always converge, this is for bugs

		int good = 0;
		while( pntsleft > 2 ) {
			sgSetVec2( A, pnts[a][0], pnts[a][2] );
			sgSetVec2( B, pnts[b][0], pnts[b][2] );
			sgSetVec2( C, pnts[c][0], pnts[c][2] );


			good = 1;
			if (!ccw(A,B,C)) {
				good = 0;
			} else {
				// make sure no verts are inside
				sgVec2 pp;
				bool ca, cb, cc;
				for (i = 0; i < npnts; i++) {
					if ((i==a)||(i==b)||(i==c)||(used[i])) continue;

					sgSetVec2( pp, pnts[i][0], pnts[i][2] );
					ca = ccw( A, B, pp );
					cb = ccw( B, C, pp );
					cc = ccw( C, A, pp );								

					if ((ca==cb) && (cb==cc)) {
						good = 0;
						break;
					} 
				}
			}

			if (good) {
				// remove "ear"
				tri = new TessTri();
				sgCopyVec3( tri->a, pnts[a] );
				sgCopyVec3( tri->b, pnts[b] );
				sgCopyVec3( tri->c, pnts[c] );
				tess.push_back( tri );
				
				used[b] = 1;
				pntsleft--;
				if (pntsleft<=2) break;

				iter = 1000;

				b = c;
				do {
					if (area < 0.0) {
						c--;
						if (c<0) c=npnts-1;
					} else {
						c++;
						if (c==npnts) c=0;					
					}
				} while (used[c]);

			} else {
				
				//assert(iter);
				if (iter==0) {
					printf("Cannot dice polygon\n");
					break; // give up
				}

				iter--;

				// advance around the polygon
				a = b;
				b = c;
				do {
					if (area < 0.0) {
						c--;
						if (c<0) c=npnts-1;
					} else {
						c++;
						if (c==npnts) c=0;					
					}
				} while (used[c]);
			}
		}
		
	} 

	// draw the top
	glNormal3f( 0.0, 1.0, 0.0 );	
	glBegin( GL_TRIANGLES );	
	for (std::vector<TessTri*>::iterator ti = tess.begin();
		 ti != tess.end(); ti++ ) {

		glTexCoord2f( (*ti)->a[0]*tscale, (*ti)->a[2]*tscale );
		glVertex3f( (*ti)->a[0], (*ti)->a[1], (*ti)->a[2] );

		glTexCoord2f( (*ti)->b[0]*tscale, (*ti)->b[2]*tscale );
		glVertex3f( (*ti)->b[0], (*ti)->b[1], (*ti)->b[2] );

		glTexCoord2f( (*ti)->c[0]*tscale, (*ti)->c[2]*tscale );
		glVertex3f( (*ti)->c[0], (*ti)->c[1], (*ti)->c[2] );

	}
	glEnd();	

	glDisable( GL_TEXTURE_2D );
}



bool IceFloe::contains( sgVec3 &pp ) {
	bool inside = false;
	int i,j;
	sgVec3 dp, p;
	float t;

	// put p in local coords
	sgSubVec3( p, pp, pos );

	for (j=npnts-1, i = 0; 
		i < npnts; j = i++ ) {			
		sgSubVec3( dp, pnts[j], pnts[i] );
    
		t = (p[2]-pnts[i][2])/dp[2];
		if ((t>=0.0) && (t < 1.0) && ((pnts[i][0] + (dp[0]*t)) >= p[0])) {			
			inside = !inside;
		}
	}	

	return inside;
} 

void IceFloe::build( int nsrcpnt, sgVec2 *srcpnt ) {
	sgVec2 c;	
	sgSetVec2( c, 0.0, 0.0 );
	int n=0,i;

	// find the centeroid
	for ( i = 0; i < nsrcpnt; i++) {

		c[0] += srcpnt[i][0];
		c[1] += srcpnt[i][1];
		n++;
	}

	if (n>0.00001) {
		c[0] /= n;
		c[1] /= n;
	}

	// make the points local
	pnts = new sgVec3[ nsrcpnt ];
	npnts = nsrcpnt;
	for ( i=0; i < nsrcpnt; i++) {				
		sgSetVec3( pnts[i], srcpnt[i][0] - c[0], 0.0, 
					        srcpnt[i][1] - c[1] );		
	}

	// set location
	sgSetVec3( pos, c[0], 0.0, c[1] );
}
