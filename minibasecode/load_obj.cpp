//
//  load_obj.cpp
//  ld48jovoc
//
//  Created by Joel Davis on 8/25/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#include <vector>
#include <load_obj.h>

// vert indexes
struct _ObjVert {
    size_t pos, st, nrm;
};

QuadBuff<DrawVert> *load_obj( const char *filename )
{
    // Vert, st and norms are shared for all gbuffs in the
    // obj file
    std::vector<vec3f> verts;
    std::vector<vec3f> nrms;
    std::vector<vec4f> sts;
    
    QuadBuff<DrawVert> *gbuff = new QuadBuff<DrawVert>();
    
    // Read the obj file
    FILE *fp = fopen( filename, "r" );
    char line[1024];
    char token[1024];
    
    if (!fp)
    {
        printf("Can't open OBJ file %s\n", filename );
        return NULL;
    }
    
    while (fgets( line, 1024, fp ))
    {
        // Skip blank lines and comments
        if ((line[0]=='#') || (line[0]=='\n') || (strlen(line)<=1))
        {
            continue;
        }
        
        // Get token at start of line
        sscanf( line, "%s", token );
        
        // "o <objName>" 
        if (!strcmp( token, "o" ))
        {
            sscanf( line, "%*s %s", token );
            printf("Object name: %s\n", token );
        }
        
        // "v <x> <y> <z>"
        else if (!strcmp(token, "v"))
        {
            vec3f v;
            sscanf( line, "%*s %f %f %f", &(v.x), &(v.y), &(v.z) );
            
            verts.push_back( v );
        }
        
        // "vt <s> <t>"
        else if (!strcmp( token, "vt"))
        {
            vec4f st;
            sscanf( line, "%*s %f %f", &(st.x), &(st.y) );
            st.z = 0.0; st.w = 0.0;
            sts.push_back(st);
        }
        
        // "n <x> <y> <z>"
        else if (!strcmp(token, "vn"))
        {
            vec3f n;
            sscanf( line, "%*s %f %f %f", &(n.x), &(n.y), &(n.z) );
            nrms.push_back( n );
        }
        
        else if (!strcmp( token, "f" ))
        {
            // Parse the face vert indexes
            std::vector<_ObjVert> faceVerts;
            char *ch = strtok( line+2, " " );
            while (ch)
            {
                size_t pndx = 0;
                size_t stndx = 0;
                size_t nrmndx = 0;
                
                // Count the slashes
                int slashCount=0;
                for (char *ch2=ch; *ch2; ch2++)
                    if ((*ch2)=='/') slashCount++;
                
                if (slashCount==0)
                {
                    // just pos
                    sscanf( ch, "%lu", &pndx );
                    stndx=1;
                    nrmndx=1;                    
                }
                else if (slashCount==1)
                {
                    // pos/st
                    sscanf( ch, "%lu/%lu", &pndx, &stndx );
                    nrmndx = 1;
                }
                else if (slashCount==2)
                {
                    if (strstr( ch, "//"))
                    {
                        // pos//nrm
                        sscanf( ch, "%lu//%lu", &pndx, &nrmndx );
                        stndx = 1;
                    }
                    else
                    {
                        // pos/st/nrm
                        sscanf( ch, "%lu/%lu/%lu", &pndx, &stndx, &nrmndx );
                    }
                }
                
                // Decrement the index because OBJs are 1-based
                pndx--; stndx--; nrmndx--;
                
                // Add this vert
                _ObjVert vert;
                vert.pos = pndx; vert.st = stndx; vert.nrm = nrmndx;                
                
                faceVerts.push_back( vert );
                
                // Next vert
                ch = strtok( NULL, " " );
            }
            
            // Now we have the whole face in faceVerts, convert it to a
            // fan of triangles for the batch. 
            
            // add dummy STs if none in obj file
            if (sts.size()==0)
            {
                sts.push_back( vec4f(0.0,0.0,0.0,0.0) );
            }
            
            // Add dummy norms if none in obj file
            if (nrms.size()==0)
            {
                nrms.push_back(vec3f(0.0,1.0,0.0) );
            }
            
            // Skip points or single particles
            if (faceVerts.size() >= 3 )
            {                
                for (int b=1; b < (faceVerts.size()-1); b++)
                {                                    
                    DrawVert *tri = gbuff->addTris(1);
                    
                    tri[0].m_pos = verts[faceVerts[0].pos];
                    tri[0].m_st = sts[faceVerts[0].st];
                    tri[0].m_nrm = nrms[faceVerts[0].nrm];
                    
                    tri[1].m_pos = verts[faceVerts[b].pos];
                    tri[1].m_st = sts[faceVerts[b].st];
                    tri[1].m_nrm = nrms[faceVerts[b].nrm];
                    
                    tri[2].m_pos = verts[faceVerts[b+1].pos];
                    tri[2].m_st = sts[faceVerts[b+1].st];
                    tri[2].m_nrm = nrms[faceVerts[b+1].nrm];
                }
            }            
        }
        
        else if (!strcmp( token, "usemtl"))
        {
            // reuse token to get mtl name
            sscanf( line, "%*s %s", token );
            // ignore matierla
        }
        
        // These are lines we don't care about, so silently ignore
        else if ( (!strcmp(token, "mtllib")) ||                 
                 (!strcmp(token, "s")) // "smoothing groups" ignore
                 )
        {
            // pass
        }
        
        else
        {
            printf( "Unknown token in .OBJ file ['%s'], skipping line.\n", token );
        }        
    }    
    
    printf( "Loaded OBJ '%s' : %lu verts %lu sts %lu nrms\n", 
           filename, verts.size(), sts.size(), nrms.size() );
    
    return gbuff;

}
