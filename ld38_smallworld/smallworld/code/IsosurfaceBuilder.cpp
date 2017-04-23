#include "Pre.h"
#include "Core/Config.h"

#include "glm/gtc/random.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/trigonometric.hpp"

#include "IsosurfaceBuilder.h"


// include this directly to keep the clutter down
#include "IsosurfaceTables.cpp"

using namespace Oryol;

// Adapted from http://paulbourke.net/geometry/polygonise/

typedef struct IsoSurfTriangle {
    glm::vec3 p[3];
} IsoSurfTriangle;

typedef struct IsoSurfGridCell {
    glm::vec3 p[8];
    float val[8];
} IsoSurfGridCell;

glm::vec3 VertexInterp( float isolevel, glm::vec3 p1, glm::vec3 p2, float valp1, float valp2);

// scratch memory to build the triangle list
#define ISO_MAX_TRIANGLES (100000)
IsoSurfTriangle *tempTris = NULL;
float *tempSDF = NULL;

/*
 Given a grid cell and an isolevel, calculate the triangular
 facets required to represent the isosurface through the cell.
 Return the number of triangular facets, the array "triangles"
 will be loaded up with the vertices at most 5 triangular facets.
	0 will be returned if the grid cell is either totally above
 of totally below the isolevel.
 */
int Polygonise(IsoSurfGridCell grid, float isolevel, IsoSurfTriangle *triangles)
{
    int i,ntriang;
    int cubeindex;
    glm::vec3 vertlist[12];
    
    /*
     Determine the index into the edge table which
     tells us which vertices are inside of the surface
     */
    cubeindex = 0;
    if (grid.val[0] < isolevel) cubeindex |= 1;
    if (grid.val[1] < isolevel) cubeindex |= 2;
    if (grid.val[2] < isolevel) cubeindex |= 4;
    if (grid.val[3] < isolevel) cubeindex |= 8;
    if (grid.val[4] < isolevel) cubeindex |= 16;
    if (grid.val[5] < isolevel) cubeindex |= 32;
    if (grid.val[6] < isolevel) cubeindex |= 64;
    if (grid.val[7] < isolevel) cubeindex |= 128;
    
    /* Cube is entirely in/out of the surface */
    if (edgeTable[cubeindex] == 0)
        return(0);
    
    /* Find the vertices where the surface intersects the cube */
    if (edgeTable[cubeindex] & 1)
        vertlist[0] =
        VertexInterp(isolevel,grid.p[0],grid.p[1],grid.val[0],grid.val[1]);
    if (edgeTable[cubeindex] & 2)
        vertlist[1] =
        VertexInterp(isolevel,grid.p[1],grid.p[2],grid.val[1],grid.val[2]);
    if (edgeTable[cubeindex] & 4)
        vertlist[2] =
        VertexInterp(isolevel,grid.p[2],grid.p[3],grid.val[2],grid.val[3]);
    if (edgeTable[cubeindex] & 8)
        vertlist[3] =
        VertexInterp(isolevel,grid.p[3],grid.p[0],grid.val[3],grid.val[0]);
    if (edgeTable[cubeindex] & 16)
        vertlist[4] =
        VertexInterp(isolevel,grid.p[4],grid.p[5],grid.val[4],grid.val[5]);
    if (edgeTable[cubeindex] & 32)
        vertlist[5] =
        VertexInterp(isolevel,grid.p[5],grid.p[6],grid.val[5],grid.val[6]);
    if (edgeTable[cubeindex] & 64)
        vertlist[6] =
        VertexInterp(isolevel,grid.p[6],grid.p[7],grid.val[6],grid.val[7]);
    if (edgeTable[cubeindex] & 128)
        vertlist[7] =
        VertexInterp(isolevel,grid.p[7],grid.p[4],grid.val[7],grid.val[4]);
    if (edgeTable[cubeindex] & 256)
        vertlist[8] =
        VertexInterp(isolevel,grid.p[0],grid.p[4],grid.val[0],grid.val[4]);
    if (edgeTable[cubeindex] & 512)
        vertlist[9] =
        VertexInterp(isolevel,grid.p[1],grid.p[5],grid.val[1],grid.val[5]);
    if (edgeTable[cubeindex] & 1024)
        vertlist[10] =
        VertexInterp(isolevel,grid.p[2],grid.p[6],grid.val[2],grid.val[6]);
    if (edgeTable[cubeindex] & 2048)
        vertlist[11] =
        VertexInterp(isolevel,grid.p[3],grid.p[7],grid.val[3],grid.val[7]);
    
    /* Create the triangle */
    ntriang = 0;
    for (i=0;triTable[cubeindex][i]!=-1;i+=3) {
        triangles[ntriang].p[0] = vertlist[triTable[cubeindex][i  ]];
        triangles[ntriang].p[1] = vertlist[triTable[cubeindex][i+1]];
        triangles[ntriang].p[2] = vertlist[triTable[cubeindex][i+2]];
        ntriang++;
    }
    
    return(ntriang);
}

/*
 Linearly interpolate the position where an isosurface cuts
 an edge between two vertices, each with their own scalar value
 */
glm::vec3 VertexInterp( float isolevel, glm::vec3 p1, glm::vec3 p2, float valp1, float valp2)
{
    float mu;
    glm::vec3 p;
    
    if (fabs(isolevel-valp1) < 0.00001)
        return(p1);
    if (fabs(isolevel-valp2) < 0.00001)
        return(p2);
    if (fabs(valp1-valp2) < 0.00001)
        return(p1);
    mu = (isolevel - valp1) / (valp2 - valp1);
    p.x = p1.x + mu * (p2.x - p1.x);
    p.y = p1.y + mu * (p2.y - p1.y);
    p.z = p1.z + mu * (p2.z - p1.z);
    
    return(p);
}




//------------------------------------------------------------------------------
IsosurfaceBuilder::IsosurfaceBuilder() :
    RandomColors(false),
    //curPrimGroupBaseElement(0),
    //curPrimGroupNumElements(0),
    color(1.0f, 1.0f, 1.0f, 1.0f),
    fx(1.0), fy(1.0), fz(1.0),
    gridRes(64)
{
    dbgPush = 0.0f;
}

float IsosurfaceBuilder::evalSDF( glm::vec3 p, glm::vec4 *color )
{
    glm::vec3 p2 = p;
    p2.y = 0.0;
    
    float dSphere = glm::length(p)-(0.8 + dbgPush);
    
    
    glm::vec3 c = glm::vec3( 0.0, 0.0,0.25);
    float dCyl1 = glm::length( glm::vec3( p.x-c.x, p.z-c.y, 0.0 )) - c.z;
    float dCyl2 = glm::length( glm::vec3( p.x-c.x, p.y-c.y, 0.0 )) - c.z;
    float dCyl3 = glm::length( glm::vec3( p.y-c.x, p.z-c.y, 0.0 )) - c.z;
    
    float dCyl = fmin( dCyl1, fmin( dCyl2, dCyl3));
    
    if (color) {
        if (dSphere < -0.001) {
            // inside the ground
            *color = glm::vec4(0.60,0.35,0.11,1.0);
        } else {
            *color = glm::vec4(0.53,0.77,0.15,1.0);
        }
    }
    
    return fmax( dSphere, -dCyl );
}

glm::vec3 IsosurfaceBuilder::evalNormal( glm::vec3 p )
{
    float e = 0.01;
    float pd = evalSDF( p );
    glm::vec3 ex = glm::vec3( e, 0.0, 0.0 );
    glm::vec3 ey = glm::vec3( 0.0, e, 0.0 );
    glm::vec3 ez = glm::vec3( 0.0, 0.0, e );
    glm::vec3 nrm = glm::vec3( evalSDF(p+ex) - evalSDF( p-ex ),
                               evalSDF(p+ey) - evalSDF( p-ey ),
                               evalSDF(p+ez) - evalSDF( p-ez ) );
    
    if (glm::length(nrm) > 0.0) {
        nrm = glm::normalize( nrm );
    }
    return nrm;
}

Oryol::SetupAndData<Oryol::MeshSetup> IsosurfaceBuilder::Build( glm::vec3 worldSize )
{
    
    printf("Isosurface:BUILD: Flips: %3.0f %3.0f %3.0f ijk1 %d %d %d\n", fx, fy, fz, i1, j1, k1 );
    
    if (!tempTris) {
        tempTris = (IsoSurfTriangle*)Memory::Alloc( sizeof(IsoSurfTriangle) * ISO_MAX_TRIANGLES );
        tempSDF = (float*)Memory::Alloc( sizeof(float) * gridRes * gridRes * gridRes );
        // free this? meh...
    }
    
    // Calc densities
    for (int k=0; k < gridRes; k++) {
        float kk = ((float(k) / float(gridRes)) - 0.5f) * 2.0;
        for (int j=0; j < gridRes; j++) {
            float jj = ((float(j) / float(gridRes)) - 0.5f) * 2.0;
            for (int i=0; i < gridRes; i++) {
                float ii = ((float(i) / float(gridRes)) - 0.5f) * 2.0;
                
                glm::vec3 p = glm::vec3( ii ,jj, kk );
                //glm::vec3 p2 = glm::vec3( ii ,0.0, kk );
                
                // todo: call eval func from caller
                
                float *sdf = tempSDF + (k * gridRes * gridRes) + (j * gridRes) + i;
                *sdf = evalSDF( p );
            }
        }
    }
    
    // Make grid cells
    int numTotalTris = 0;
    IsoSurfTriangle *triList = tempTris;
    IsoSurfGridCell cell;
    float halfSz = 1.0f / float(gridRes-1);
    for (int nk=0; nk < gridRes-1; nk++) {
        
        for (int nj=0; nj < gridRes-1; nj++) {
            
            for (int ni=0; ni < gridRes-1; ni++) {
            
                
                float kk = ((float(nk) / float(gridRes-1)) - 0.5f) * 2.0;
                float jj = ((float(nj) / float(gridRes-1)) - 0.5f) * 2.0;
                float ii = ((float(ni) / float(gridRes-1)) - 0.5f) * 2.0;
                
                int i = ni;
                int j = nk;
                int k = nj;
                
                int i0 = 1-i1;
                int j0 = 1-j1;
                int k0 = 1-k1;
                
                glm::vec3 p = glm::vec3( ii,jj,kk );
                cell.p[0] = p + glm::vec3( halfSz*fx, -halfSz*fy, halfSz*fz );
                cell.val[0] = *(tempSDF + ((k+k1) * gridRes * gridRes) + ((j+j0) * gridRes) + (i+i0));
                
                cell.p[1] = p + glm::vec3( -halfSz*fx, -halfSz*fy, halfSz*fz );
                cell.val[1] = *(tempSDF + ((k+k1) * gridRes * gridRes) + ((j+j0) * gridRes) + (i+i1));
                
                cell.p[2] = p + glm::vec3( -halfSz*fx, -halfSz*fy, -halfSz*fz );
                cell.val[2] = *(tempSDF + ((k+k1) * gridRes * gridRes) + ((j+j1) * gridRes) + (i+i1));
                
                cell.p[3] = p + glm::vec3( halfSz*fx, -halfSz*fy, -halfSz*fz );
                cell.val[3] = *(tempSDF + ((k+k1) * gridRes * gridRes) + ((j+j1) * gridRes) + (i+i0));
                
                cell.p[4] = p + glm::vec3( halfSz*fx, halfSz*fy, halfSz*fz );
                cell.val[4] = *(tempSDF + ((k+k0) * gridRes * gridRes) + ((j+j0) * gridRes) + (i+i0));
                
                cell.p[5] = p + glm::vec3( -halfSz*fx, halfSz*fy, halfSz*fz );
                cell.val[5] = *(tempSDF + ((k+k0) * gridRes * gridRes) + ((j+j0) * gridRes) + (i+i1));
                
                cell.p[6] = p + glm::vec3( -halfSz*fx, halfSz*fy, -halfSz*fz );
                cell.val[6] = *(tempSDF + ((k+k0) * gridRes * gridRes) + ((j+j1) * gridRes) + (i+i1));
                
                cell.p[7] = p + glm::vec3( halfSz*fx, halfSz*fy, -halfSz*fz );
                cell.val[7] = *(tempSDF + ((k+k0) * gridRes * gridRes) + ((j+j1) * gridRes) + (i+i0));
                
                // DBG
                //memcpy( cell.val, pdbg, sizeof(pdbg));
                
                o_assert2( numTotalTris + 5 < ISO_MAX_TRIANGLES, "Ran out of iso triangles!" );
                
                int numTris = Polygonise( cell, 0.0, triList );
                numTotalTris += numTris;
                triList += numTris;
            }
        }
    }
    printf("Rebuild planet: Used %d triangles...\n", numTotalTris );
    
    // Mesh builder does not like zero triangles
    if (numTotalTris == 0) {
        numTotalTris = 1;
    }

    
    // Put triangles into the the mesh builder
    meshBuilder.Layout = Layout;
    meshBuilder.NumVertices = numTotalTris * 3;
    meshBuilder.NumIndices = numTotalTris * 3;
    meshBuilder.IndicesType = IndexType::Index32;
    
    meshBuilder.PrimitiveGroups.Clear();
    meshBuilder.PrimitiveGroups.Add( 0, numTotalTris*3 );
    meshBuilder.Begin();
    
//    const glm::vec3 minRand(-3000.0f, -3000.0f, -3000.0f );
//    const glm::vec3 maxRand( 3000.0f,  3000.0f,  3000.0f );
    
    
    for (int i=0; i < numTotalTris; i++) {
        
        IsoSurfTriangle *isoTri = tempTris + i;
        
        // Swizzle the verts because the marching cubes indexing is fucked up
        glm::vec3 vertA = glm::vec3( isoTri->p[0].x, isoTri->p[0].z, isoTri->p[0].y );
        glm::vec3 vertB = glm::vec3( isoTri->p[1].x, isoTri->p[1].z, isoTri->p[1].y );
        glm::vec3 vertC = glm::vec3( isoTri->p[2].x, isoTri->p[2].z, isoTri->p[2].y );
        
        glm::vec3 nrmA = evalNormal( vertA );
        glm::vec3 nrmB = evalNormal( vertB );
        glm::vec3 nrmC = evalNormal( vertC );
        
        // eval color
        glm::vec4 colorA, colorB, colorC;
        evalSDF( vertA, &colorA );
        evalSDF( vertB, &colorB );
        evalSDF( vertC, &colorC );
        
        vertA = vertA * worldSize;
        vertB = vertB * worldSize;
        vertC = vertC * worldSize;
        
        //printf("nrmA %3.2f %3.2f %3.2f\n", nrmA.x, nrmA.y, nrmA.z );
        
        //printf("TRI A (%3.2f %3.2f %3.2f)\n", vertA.x,
        
        //glm::vec3 nrm = glm::normalize( glm::cross( vertA, vertB ) );
        
        meshBuilder
            .Vertex( (i*3)+0, VertexAttr::Position, vertA.x, vertA.y, vertA.z )
            .Vertex( (i*3)+0, VertexAttr::TexCoord0, 0.0f, 1.0f )
            .Vertex( (i*3)+0, VertexAttr::Normal, nrmA.x, nrmA.y, nrmA.z )
            .Vertex( (i*3)+0, VertexAttr::Color0, colorA.x, colorA.y, colorA.z, colorA.a )
        
            .Vertex( (i*3)+1, VertexAttr::Position, vertB.x, vertB.y, vertB.z )
            .Vertex( (i*3)+1, VertexAttr::TexCoord0, 1.0f, 1.0f )
            .Vertex( (i*3)+1, VertexAttr::Normal, nrmB.x, nrmB.y, nrmB.z )
            .Vertex( (i*3)+1, VertexAttr::Color0, colorB.x, colorB.y, colorB.z, colorB.a )
        
            .Vertex( (i*3)+2, VertexAttr::Position, vertC.x, vertC.y, vertC.z )
            .Vertex( (i*3)+2, VertexAttr::TexCoord0, 1.0f, 0.0f )
            .Vertex( (i*3)+2, VertexAttr::Normal, nrmC.x, nrmC.y, nrmC.z )
            .Vertex( (i*3)+2, VertexAttr::Color0, colorB.x, colorB.y, colorB.z, colorB.a )
        ;
        
        //meshBuilder.Triangle( i, (i*3)+0, (i*3)+1, (i*3)+2 );
        //meshBuilder.Triangle32( i, (i*3)+0, (i*3)+1, (i*3)+2 );
        meshBuilder.Index32( i*3+0, i*3+0 );
        meshBuilder.Index32( i*3+1, i*3+1 );
        meshBuilder.Index32( i*3+2, i*3+2 );
    }
    
    return meshBuilder.Build();
    
    
}
