#ifndef ISOSURFACE_H
#define ISOSURFACE_H

// based on the code from paul bourke, matthew ward, etc...
// http://paulbourke.net/geometry/polygonise/

// derived from the Oryol ShapeBuilder
#include "Pre.h"
#include "Assets/Gfx/MeshBuilder.h"
#include "Core/Containers/Array.h"
#include "Core/Types.h"
#include "Gfx/Gfx.h"

#include "glm/mat4x4.hpp"
#include "glm/vec4.hpp"

#include "open-simplex-noise.h"

class IsosurfaceBuilder {
public:
    /// constructor
    IsosurfaceBuilder();
    
    /// read/write access to vertex layout
    class Oryol::VertexLayout Layout;
    
    /// random-vertex-colors flag
    bool RandomColors;
    
    /// build geometry and clear object state
    Oryol::SetupAndData<Oryol::MeshSetup> Build( glm::vec3 worldSize );
    
    glm::vec4 color;
    
    float fx, fy, fz;
    int i1, j1, k1;
    
    float dbgPush;
    float pdbg[8];
    
    int gridRes; // resolution of grid, e.g. 64x64x64
    
    // FIXME: this should live in the planet
    int damageRes;
    float *damage;
    void clearDamage();
    void addDamage( glm::vec3 p, float radius );
    float lookupDamage( glm::vec3 p );    
    struct osn_context *noiseCtx;
    
    float evalSDF( glm::vec3 p, glm::vec4 *color=NULL );
    glm::vec3 evalNormal( glm::vec3 p );
    
    
    Oryol::MeshBuilder meshBuilder;
};


#endif
