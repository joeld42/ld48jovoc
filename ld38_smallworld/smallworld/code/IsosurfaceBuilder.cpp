#include "Pre.h"
#include "Core/Config.h"

#include "glm/gtc/random.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/trigonometric.hpp"

#include "IsosurfaceBuilder.h"

using namespace Oryol;

//------------------------------------------------------------------------------
IsosurfaceBuilder::IsosurfaceBuilder() :
    RandomColors(false),
    //curPrimGroupBaseElement(0),
    //curPrimGroupNumElements(0),
    color(1.0f, 1.0f, 1.0f, 1.0f)
{
        // empty
}


Oryol::SetupAndData<Oryol::MeshSetup> IsosurfaceBuilder::Build()
{
    const int numTri = 100;
    
    meshBuilder.Layout = Layout;
    meshBuilder.NumVertices = numTri * 3;
    meshBuilder.NumIndices = numTri * 3;
    meshBuilder.IndicesType = IndexType::Index16;
    
    meshBuilder.PrimitiveGroups.Clear();
    meshBuilder.PrimitiveGroups.Add( 0, numTri*3 );
    meshBuilder.Begin();
    
    const glm::vec3 minRand(-3000.0f, -3000.0f, -3000.0f );
    const glm::vec3 maxRand( 3000.0f,  3000.0f,  3000.0f );
    
    
    for (int i=0; i < numTri; i++) {
        
        glm::vec3 randA = glm::linearRand(minRand, maxRand);
        glm::vec3 randB = glm::linearRand(minRand, maxRand);
        glm::vec3 randC = glm::linearRand(minRand, maxRand);
        
        glm::vec3 nrm = glm::normalize( glm::cross( randA, randB ) );
        
        meshBuilder
            .Vertex( (i*3)+0, VertexAttr::Position, randA.x, randA.y, randA.z )
            .Vertex( (i*3)+0, VertexAttr::TexCoord0, 0.0f, 1.0f )
            .Vertex( (i*3)+0, VertexAttr::Normal, nrm.x, nrm.y, nrm.z )
        
            .Vertex( (i*3)+1, VertexAttr::Position, randB.x, randB.y, randB.z )
            .Vertex( (i*3)+1, VertexAttr::TexCoord0, 1.0f, 1.0f )
            .Vertex( (i*3)+1, VertexAttr::Normal, nrm.x, nrm.y, nrm.z )
        
            .Vertex( (i*3)+2, VertexAttr::Position, randC.x, randC.y, randC.z )
            .Vertex( (i*3)+2, VertexAttr::TexCoord0, 1.0f, 0.0f )
            .Vertex( (i*3)+2, VertexAttr::Normal, nrm.x, nrm.y, nrm.z )
        ;
        
        meshBuilder.Triangle( i, (i*3)+0, (i*3)+1, (i*3)+2 );
    }
    
    return meshBuilder.Build();
    
    
}
