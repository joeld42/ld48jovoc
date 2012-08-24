//
//  gbuff_prim.cpp
//  luddite_ios
//
//  Created by Joel Davis on 7/29/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#include <quadbuff.h>
#include <shapes.h>

#define NUM_CUBE_VERTS (36)
static float _cubeVertData[NUM_CUBE_VERTS * 6] = 
{
    // Data layout for each line below is:
    // positionX, positionY, positionZ,     normalX, normalY, normalZ,
    0.5f, -0.5f, -0.5f,        1.0f, 0.0f, 0.0f,
    0.5f, 0.5f, -0.5f,         1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.5f,         1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.5f,         1.0f, 0.0f, 0.0f,
    0.5f, 0.5f, -0.5f,         1.0f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.5f,          1.0f, 0.0f, 0.0f,
    
    0.5f, 0.5f, -0.5f,         0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f, -0.5f,        0.0f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f,          0.0f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f,          0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f, -0.5f,        0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f, 0.5f,         0.0f, 1.0f, 0.0f,
    
    -0.5f, 0.5f, -0.5f,        -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,       -1.0f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.5f,         -1.0f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.5f,         -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,       -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, 0.5f,        -1.0f, 0.0f, 0.0f,
    
    -0.5f, -0.5f, -0.5f,       0.0f, -1.0f, 0.0f,
    0.5f, -0.5f, -0.5f,        0.0f, -1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f,        0.0f, -1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f,        0.0f, -1.0f, 0.0f,
    0.5f, -0.5f, -0.5f,        0.0f, -1.0f, 0.0f,
    0.5f, -0.5f, 0.5f,         0.0f, -1.0f, 0.0f,
    
    0.5f, 0.5f, 0.5f,          0.0f, 0.0f, 1.0f,
    -0.5f, 0.5f, 0.5f,         0.0f, 0.0f, 1.0f,
    0.5f, -0.5f, 0.5f,         0.0f, 0.0f, 1.0f,
    0.5f, -0.5f, 0.5f,         0.0f, 0.0f, 1.0f,
    -0.5f, 0.5f, 0.5f,         0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f,        0.0f, 0.0f, 1.0f,
    
    0.5f, -0.5f, -0.5f,        0.0f, 0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,       0.0f, 0.0f, -1.0f,
    0.5f, 0.5f, -0.5f,         0.0f, 0.0f, -1.0f,
    0.5f, 0.5f, -0.5f,         0.0f, 0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,       0.0f, 0.0f, -1.0f,
    -0.5f, 0.5f, -0.5f,        0.0f, 0.0f, -1.0f
};

QuadBuff<DrawVert> *make_cube( float size, vec3f center )
{
    QuadBuff<DrawVert> *gbuff = new QuadBuff<DrawVert>();
    
    DrawVert *cubeVert = gbuff->addVerts( NUM_CUBE_VERTS );
    
    for (int i=0; i < NUM_CUBE_VERTS; ++i)
    {
        cubeVert->m_pos = vec3f( (_cubeVertData[i*6 + 0] * size) - center.x,
                                 (_cubeVertData[i*6 + 1] * size) - center.y,
                                 (_cubeVertData[i*6 + 2] * size) - center.z );
 
        cubeVert->m_nrm = vec3f( (_cubeVertData[i*6 + 3] * size) - center.x,
                                 (_cubeVertData[i*6 + 4] * size) - center.y,
                                 (_cubeVertData[i*6 + 5] * size) - center.z );
        
        cubeVert++;
    }
    
    return gbuff;
}

QuadBuff<DrawVert> *make_cylinder( int nsegments, float radius, 
                       float height, vec3f center )
{
    QuadBuff<DrawVert> *gbuff = new QuadBuff<DrawVert>();
    
    // four triangles for each segment (top cap, bottom cap, middle quad)
    DrawVert *cylVert = gbuff->addVerts( nsegments * 4*3 );
    
    float halfhite = height/2.0;
    vec2f plast;
    vec3f nlast;
    for (int i=0; i <= nsegments; i++)
    {
        size_t ndx = i*12;
        float tval = ((float)i / (float)(nsegments-1)) * M_PI * 2.0;
        
        vec2f pcurr( cos( tval ) * radius, sin(tval)*radius );
        vec3f ncurr( pcurr.x, 0.0, pcurr.y );
        ncurr.Normalize();
        if (i>0)
        {            
            // Top Cap
            cylVert[ndx+0].m_pos = vec3f( 0.0, halfhite, 0.0 );
            cylVert[ndx+0].m_nrm = vec3f( 0.0, 1.0, 0.0 );

            cylVert[ndx+1].m_pos = vec3f( plast.x, halfhite, plast.y );
            cylVert[ndx+1].m_nrm = vec3f( 0.0, 1.0, 0.0 );

            cylVert[ndx+2].m_pos = vec3f( pcurr.x, halfhite, pcurr.y );
            cylVert[ndx+2].m_nrm = vec3f( 0.0, 1.0, 0.0 );

            // Bottom cap
            cylVert[ndx+3].m_pos = vec3f( 0.0, -halfhite, 0.0 );
            cylVert[ndx+3].m_nrm = vec3f( 0.0, -1.0, 0.0 );
            
            cylVert[ndx+4].m_pos = vec3f( plast.x, -halfhite, plast.y );
            cylVert[ndx+4].m_nrm = vec3f( 0.0, -1.0, 0.0 );
            
            cylVert[ndx+5].m_pos = vec3f( pcurr.x, -halfhite, pcurr.y );
            cylVert[ndx+5].m_nrm = vec3f( 0.0, -1.0, 0.0 );   
            
            // Middle part (upper tri)
            cylVert[ndx+6].m_pos = vec3f( pcurr.x, halfhite, pcurr.y );
            cylVert[ndx+6].m_nrm = ncurr;  

            cylVert[ndx+7].m_pos = vec3f( plast.x, halfhite, plast.y );
            cylVert[ndx+7].m_nrm = nlast;  

            cylVert[ndx+8].m_pos = vec3f( pcurr.x, -halfhite, pcurr.y );
            cylVert[ndx+8].m_nrm = ncurr;  

            // Middle part (lower tri)
            cylVert[ndx+9].m_pos = vec3f( pcurr.x, -halfhite, pcurr.y );
            cylVert[ndx+9].m_nrm = ncurr;  
            
            cylVert[ndx+10].m_pos = vec3f( plast.x, -halfhite, plast.y );
            cylVert[ndx+10].m_nrm = nlast;              
            
            cylVert[ndx+11].m_pos = vec3f( plast.x, halfhite, plast.y );
            cylVert[ndx+11].m_nrm = nlast;              
        }
        plast=pcurr;
        nlast=ncurr;
    }
    
    return gbuff;
}

#if 0 
void setColorConstant( QuadBuff<DrawVert> *gbuff, const vec4f &color )
{
    for (eastl::vector<luddite::DrawVert>::iterator vi = gbuff->m_vertData.begin();
         vi != gbuff->m_vertData.end(); ++vi )
    {
        luddite::DrawVert &vert = (*vi);
        vert.m_color[0] = color.r * 0xff;
        vert.m_color[1] = color.g * 0xff;
        vert.m_color[2] = color.b * 0xff;
        vert.m_color[3] = color.a * 0xff;
    }
}
#endif
