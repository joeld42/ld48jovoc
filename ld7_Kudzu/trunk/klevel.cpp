#include <windows.h>

#include <assert.h>
#include <vector>

#include "kudzu.h"
#include "klevel.h"

#define EPS (0.001)

bool pmatch( const vec2f &a, const vec2f &b )
{
	return ((fabs(a.x-b.x) < EPS) &&			
			(fabs(a.y-b.y) < EPS));
}

bool edge_match( vec2f ea, vec2f eb,
				 vec2f a, vec2f b, vec2f c )
{
	printf(" Check edge %f %f -> %f %f\n", ea.x, ea.y, eb.x, eb.y );
	// check edge AB
	if ( (pmatch(ea, a) && pmatch(eb, b)) ||
		 (pmatch(eb, a) && pmatch(ea, b)) ) {
			 printf("AB: match edge %f %f --> %f %f\n", a.x, a.y, b.x, b.y );
			 return true;
		 }

	// check edge BC
	if ( (pmatch(ea, c) && pmatch(eb, b)) ||
		(pmatch(eb, c) && pmatch(ea, b)) ) {
			printf("BC: match edge %f %f\n", b.x, b.y, c.x, c.y );
			return true;
		}

	// check edge CA
	if ( (pmatch(ea, a) && pmatch(eb, c)) ||
		(pmatch(eb, a) && pmatch(ea, c)) )  {
		printf("CA: match edge %f %f\n", a.x, a.y, c.x, c.y );
		 return true;
	}
	return false;
}

bool pointInTri( const vec2f &pp, KSimTri &tri ) {
	vec3f ab, bc, ca, v1, v2, p;
	bool s1,s2,s3;

	p = vec3f( pp.x, pp.y, 0.0f );

	ab = vec3f( tri.kb.x - tri.ka.x, tri.kb.y - tri.ka.y, 0.0f );
	bc = vec3f( tri.kc.x - tri.kb.x, tri.kc.y - tri.kb.y, 0.0f );
	ca = vec3f( tri.ka.x - tri.kc.x, tri.ka.y - tri.kc.y, 0.0f );
	
	
	v1 = prmath::CrossProduct( ab, bc );
	v2 = prmath::CrossProduct( ab, p - vec3f(tri.kb.x, tri.kb.y, 0.0f) );
	s1 = DotProduct( v1, v2 ) > 0.0f;

	v1 = prmath::CrossProduct( bc, ca );
	v2 = prmath::CrossProduct( bc, p - vec3f(tri.kc.x, tri.kc.y, 0.0f) );
	s2 = DotProduct( v1, v2 ) > 0.0f;

	v1 = prmath::CrossProduct( ca, ab );
	v2 = prmath::CrossProduct( ca, p - vec3f(tri.ka.x, tri.ka.y, 0.0f));
	s3 = DotProduct( v1, v2 ) > 0.0f;

	return ((s1==s2)&&(s2==s3));
}

void KLevel::loadSimMesh( const char *filename )
{
	FILE *fp = fopen( filename, "rt" );	
	assert(fp);

	vec3f v;
	vec2f uv;
	std::vector< vec3f > verts;
	std::vector< vec2f > uvs;

	char line[1000], token[100];
	while (fgets( line, 1000, fp )) {
		
		// strip newline
		if (line[strlen(line)-1]=='\n') line[strlen(line)-1] = 0;
		
		// skip blank lines and comments
		if ((strlen(line)==0) || (line[0]=='#')) continue;

		//printf ("LINE %s\n", line );

		// evaluate line
		sscanf( line, "%s", token );
		if (!strcmp(token,"v")) {
			// vertex
			sscanf( line, "%*s %f %f %f", &v.x, &v.y, &v.z );
			verts.push_back( v );
		} else if (!strcmp(token,"vt")) {
			// kudzu coord
			sscanf( line, "%*s %f %f", &uv.x, &uv.y );
			uvs.push_back( uv );
		} else if (!strcmp( token, "f" )) {
			// must be triangle
			int v1,v2,v3, uv1,uv2,uv3;
			sscanf( line, "%*s %d/%d %d/%d %d/%d",
						&v1, &uv1, &v2, &uv2, &v3, &uv3);
			
			KSimTri t;
			vec3f v;			
			printf("%d %d %d\n", v1, v2, v3 );
			t.a = verts[v1-1]; t.b = verts[v2-1]; t.c = verts[v3-1];
			t.ka = uvs[uv1-1]; t.kb = uvs[uv2-1]; t.kc = uvs[uv3-1];
			t.ablink = -1; t.bclink = -1; t.calink = -1;

			// get normal from triangle
			vec3f a, b;
			a = prmath::Normalize(t.b - t.a);
			b = prmath::Normalize(t.c - t.b);
			t.nrm = prmath::Normalize( prmath::CrossProduct( a, b ));

			m_tri.push_back( t );			
		}
	}
	
	// set up links
	for (size_t i=0; i < m_tri.size(); i++) {
		printf(" --- Triangle %d ---\n", i );
		for (size_t j=0; j < m_tri.size(); j++) {
			if (i==j) continue; // skip self
			
			if (edge_match( m_tri[i].ka, m_tri[i].kb, 
					m_tri[j].ka, m_tri[j].kb, m_tri[j].kc )) {
				printf("Edge %f %f --> %f %f matches triangle %d\n",
					m_tri[i].ka.x, m_tri[i].ka.y,
					m_tri[i].kb.x, m_tri[i].kb.y, j );


				m_tri[i].ablink = j;
			}					

			if (edge_match( m_tri[i].kb, m_tri[i].kc, 
					m_tri[j].ka, m_tri[j].kb, m_tri[j].kc )) {
				m_tri[i].bclink = j;
			}

			if (edge_match( m_tri[i].kc, m_tri[i].ka, 
					m_tri[j].ka, m_tri[j].kb, m_tri[j].kc )) {
				m_tri[i].calink = j;
			}					

			// found all edges of this tri, stop looking
			if ( (m_tri[i].ablink >=0) &&
				 (m_tri[i].bclink >=0) &&
				 (m_tri[i].calink >=0) ) break;
		}
	}
}

vec3f KSimTri::uv2bary( vec2f uv )
{
	vec3f bary;
	float b0;

	b0 =  (kb.x - ka.x) * (kc.y - ka.y) - (kc.x - ka.x) * (kb.y - ka.y);
	bary.x = ((kb.x - uv.x) * (kc.y - uv.y) - (kc.x - uv.x) * (kb.y - uv.y)) / b0;
	bary.y = ((kc.x - uv.x) * (ka.y - uv.y) - (ka.x - uv.x) * (kc.y - uv.y)) / b0;
	bary.z = ((ka.x - uv.x) * (kb.y - uv.y) - (kb.x - uv.x) * (ka.y - uv.y)) / b0;
	//printf("DBG: %f %f %f = %f\n", bary.x, bary.y, bary.z, bary.x + bary.y + bary.z );

	return bary;
}

vec3f KSimTri::pos2bary( vec3f pos )
{
	// project based on normal
	KSimTri t;
	vec3f bary;
	if ((fabs(nrm.x) > fabs(nrm.y)) && (fabs(nrm.x) > fabs(nrm.z)) ) {
		// project along x
		t.ka = vec2f(a.y, a.z);
		t.kb = vec2f(b.y, b.z);
		t.kc = vec2f(c.y, c.z);
		bary = t.uv2bary( vec2f( pos.y, pos.z ));
	} else if ((fabs(nrm.y) > fabs(nrm.x)) && (fabs(nrm.y) > fabs(nrm.z)) ) {
		// project along y
		t.ka = vec2f(a.x, a.z);
		t.kb = vec2f(b.x, b.z);
		t.kc = vec2f(c.x, c.z);
		bary = t.uv2bary( vec2f( pos.x, pos.z ));
	} else {
		// project along z
		t.ka = vec2f(a.x, a.y);
		t.kb = vec2f(b.x, b.y);
		t.kc = vec2f(c.x, c.y);
		bary = t.uv2bary( vec2f( pos.x, pos.y ));
	}
	return bary;
}

vec3f KSimTri::bary2pos( vec3f bary )
{
	return vec3f( a.x*bary.x + b.x*bary.y + c.x*bary.z,
				  a.y*bary.x + b.y*bary.y + c.y*bary.z,
			      a.z*bary.x + b.z*bary.y + c.z*bary.z );

}

vec2f KSimTri::bary2uv( vec3f bary )
{
	return vec2f( ka.x*bary.x + kb.x*bary.y + kc.x*bary.z,
				  ka.y*bary.x + kb.y*bary.y + kc.y*bary.z );
}

// slow but who cares?
vec3f KLevel::k2pos( vec2f kpos )
{
	vec3f bary;
	for (size_t i=0; i < m_tri.size(); i++) {
		bary =  m_tri[i].uv2bary( kpos );
		if ((bary.x > 0) && (bary.y > 0) && (bary.z > 0)) {
		//if (fabs(bary.x + bary.y+bary.z-1.0) < EPS) {
		//if (pointInTri( kpos, m_tri[i] )) {
			//printf("BARY: %f %f %f = %f\n", bary.x, bary.y, bary.z, bary.x+ bary.y+ bary.z );
			// inside triangle
			return m_tri[i].bary2pos( bary );
		}
	}

	return vec3f( 0.0f, 0.0f, 0.0f );
}

vec3f KLevel::k2nrm( vec2f kpos )
{
	vec3f bary;
	for (size_t i=0; i < m_tri.size(); i++) {
		bary =  m_tri[i].uv2bary( kpos );
		if ((bary.x > 0) && (bary.y > 0) && (bary.z > 0)) {
			// get normal from triangle
			vec3f a, b, nrm;
			a = prmath::Normalize(m_tri[i].b - m_tri[i].a);
			b = prmath::Normalize(m_tri[i].c - m_tri[i].b);
			nrm = prmath::Normalize( prmath::CrossProduct( a, b ));
			return nrm;
		}
	}
	return vec3f( 0.0f, 1.0f, 0.0f );
}

// lookup firing info
bool KLevel::getSimInfo( vec2f kpos, float heading, 
						vec3f &pos, vec3f &nrm, vec3f &dir,
						KSimTri &tri, vec3f &bary) {
	
	vec3f bary2;
	for (size_t i=0; i < m_tri.size(); i++) {
		bary =  m_tri[i].uv2bary( kpos );
		if ((bary.x > 0) && (bary.y > 0) && (bary.z > 0)) {

			// get position
			pos = m_tri[i].bary2pos( bary );

			// get cached normal from triangle			
			nrm = m_tri[i].nrm;

			// offset uv to get dir
			vec2f offs;
			vec3f p2;
			offs = vec2f( cos( heading * D2R ), sin( heading * D2R ) );
			bary2 = m_tri[i].uv2bary( kpos + offs * 0.001 );
			p2 = m_tri[i].bary2pos( bary2 );

			dir = prmath::Normalize( p2 - pos );

			tri = m_tri[i];
			return true;
		}
	}

	// node went off the reservation
	return false;
}
