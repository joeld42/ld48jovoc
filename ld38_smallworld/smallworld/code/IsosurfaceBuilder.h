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


class IsosurfaceBuilder {
public:
    /// constructor
    IsosurfaceBuilder();
    
    /// read/write access to vertex layout
    class Oryol::VertexLayout Layout;
    
    /// random-vertex-colors flag
    bool RandomColors;
    
    /// build geometry and clear object state
    Oryol::SetupAndData<Oryol::MeshSetup> Build();
    
    glm::vec4 color;
    
    Oryol::MeshBuilder meshBuilder;
};


#endif
