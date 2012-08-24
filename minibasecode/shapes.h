//
//  gbuff_prim.h
//  luddite_ios
//
//  Created by Joel Davis on 7/29/12.
//  Copyright (c) 2012 Joel Davis. All rights reserved.
//

#ifndef luddite_ios_gbuff_prim_h
#define luddite_ios_gbuff_prim_h

#include <prmath/prmath.hpp>
#include <quadbuff.h>

// -------------------------------
// Common per-vertex attributes
struct DrawVert
{
    vec3f m_pos;
    vec4f m_st;  // (s0, t0, s1, t1)
    vec3f m_nrm;
    // prmath::vec3			tangents[2];
    uint8_t	m_color[4];
};

enum
{
    DrawVertAttrib_POSITION,
    DrawVertAttrib_TEXCOORD,
    DrawVertAttrib_NORMAL,
    //    DrawVertAttrib_TANGENT,
    //    DrawVertAttrib_BITANGENT,
    DrawVertAttrib_COLOR,
};



// Make GBuffs from primitives. Probably more useful for placeholders
// and debugging...

// -------------------------------
//   Primitives
// -------------------------------
QuadBuff<DrawVert> *make_cube( float size=1.0, vec3f center=vec3f(0.0,0.0,0.0) );

QuadBuff<DrawVert> *make_cylinder( int nsegments=12, 
                               float radius=0.5, float height=1.0,
                               vec3f center=vec3f(0.0, 0.0, 0.0 ) );    

void setColorConstant( QuadBuff<DrawVert> *gbuff, const vec4f &color );

#endif
