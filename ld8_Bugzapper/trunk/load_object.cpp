#include <windows.h>

#include <stdio.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <vector>

#include <prmath/prmath.hpp>
using namespace prmath;

// load obj into a gl display list
GLuint loadObj( const char *filename ) 
{
	FILE *fp = fopen( filename, "r" );
	std::vector<vec3f> verts, norms;
	std::vector<vec2f> texcoord;

	GLuint objID;
	objID = glGenLists( 1 );	

	glNewList( objID,  GL_COMPILE );	
	
	char line[1000];
	char token[100];
	while (!feof(fp)) {
		fgets( line, 1000, fp );
		//printf("line %s\n", line );

		sscanf( line, "%s", token );
		if (!strcmp( token, "v")) {
			vec3f pnt;
			sscanf( line, "%*s %f %f %f\n", &(pnt.x), &(pnt.y), &(pnt.z) );
			verts.push_back( pnt );
		} else if (!strcmp( token, "vt" )) {
			vec2f st;
			sscanf( line, "%*s %f %f\n", &(st.x), &(st.y) );
			st.y = 1.0f - st.y;
			texcoord.push_back( st );
		} else if (!strcmp( token, "vn" )) {
			vec3f nrm;
			sscanf( line, "%*s %f %f %f\n", &(nrm.x), &(nrm.y), &(nrm.z) );
			norms.push_back( nrm );
		} else if (!strcmp( token, "f" )) {
			glBegin( GL_POLYGON );
			
			char *tok = strtok( line+2, " " );
			while (tok) {
				//printf("Tok (%s)\n", tok );
				unsigned int vNdx, stNdx, nrmNdx;
				sscanf( tok, "%d/%d/%d", &vNdx, &stNdx, &nrmNdx );
				vNdx--; stNdx--; nrmNdx--; // objs are 1 indexed
				//printf("vs %d ts %d ns %d\n", 
				//	verts.size(), texcoord.size(), norms.size() );
				//printf("v %d st %d n %d\n", vNdx, stNdx, nrmNdx );
				

				glTexCoord2d( texcoord[stNdx].x, texcoord[stNdx].y );
				glNormal3f( norms[nrmNdx].x, norms[nrmNdx].y, norms[nrmNdx].z );
				glVertex3f( verts[vNdx].x, verts[vNdx].y, verts[vNdx].z );

				tok = strtok( NULL, " " );
			}

			glEnd();
		}
	}

	glEndList();
	return objID;
}