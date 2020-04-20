/* Example:


/Users/joeld/oprojects/fips-deploy/crossroads/osx-xcode-debug/ldjamtool ./assets/TestLevelSF.ogex ./gamedata/TestLevelSF.ldjam
*/


#include <stdio.h>
#include <string.h>

#include "OpenGEX.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "glm-aabb.h"
#include "ldjamfile.h"

// NOTE: Header info for MAX_MESHES always gets written into the header, so don't make this too big.
// Could clean this up later to save a few bytes
#define MAX_MESHES (32)


#define MEGABYTE (1024*1024)
#define SCENEFILE_CONTENT_BUFF_SIZE (10 * MEGABYTE)

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_RT_BIG_ENDIAN
#   define FourCC2Str(fourcc) (const char[]){*((char*)&fourcc), *(((char*)&fourcc)+1), *(((char*)&fourcc)+2), *(((char*)&fourcc)+3),0}
#else
#   define FourCC2Str(fourcc) (const char[]){*(((char*)&fourcc)+3), *(((char*)&fourcc)+2), *(((char*)&fourcc)+1), *(((char*)&fourcc)+0),0}
#endif

using namespace OGEX;

using namespace CPM_GLM_AABB_NS;

// extra descrption for errorCode
const char *DescribeError( ODDL::DataResult errorCode );

void dbgPrintMatrix( const char *label, glm::mat4 m )
{
    printf("mat4 %10s| %3.2f %3.2f %3.2f %3.2f\n"
           "               | %3.2f %3.2f %3.2f %3.2f\n"
           "               | %3.2f %3.2f %3.2f %3.2f\n"
           "               | %3.2f %3.2f %3.2f %3.2f\n",
           label,
           m[0][0], m[0][1], m[0][2], m[0][3],
           m[1][0], m[1][1], m[1][2], m[1][3],
           m[2][0], m[2][1], m[2][2], m[2][3],
           m[3][0], m[3][1], m[3][2], m[3][3] );
}

glm::mat4x4 g_fixCoordsMatrix2 = glm::mat4x4( 1, 0, 0, 0,
                                              0, 0, 1, 0,
                                              0, 1, 0, 0,
                                              0, 0, 0, 1 );
//glm::mat4x4 g_fixCoordsMatrix = glm::rotate( glm::mat4x4(), 90.0f, glm::vec3( 0, 0, 1 ));
glm::mat4x4 g_fixCoordsMatrix = glm::mat4();

// ======================================================================
// ReadEntireFile
// ======================================================================
void *ReadEntireFile( const char *filename, size_t *out_filesz )
{
    FILE *fp = fopen( filename, "r" );
    if (!fp) return NULL;
    
    // Get file size
    fseek( fp, 0L, SEEK_END );
    size_t filesz = ftell(fp);
    fseek( fp, 0L, SEEK_SET );

    void *fileData = malloc( filesz+1 );
    if (fileData)
    {
        size_t result = fread( fileData, filesz, 1, fp );
        
        // result is # of chunks read, we're asking for 1, fread
        // won't return partial reads, so it's all or nothing.
        if (!result)
        {
            free(fileData);
            fileData = NULL;
        }
        else
        {
            // read suceeded, set out filesize
            *out_filesz = filesz;
        }
    }    
    
    return fileData;
}

const Structure *FindSubStructureOfType( Structure *structure, StructureType targetType )
{

    const Structure *subStruct = structure->GetFirstSubnode();

    while (subStruct) 
    {
        if(subStruct->GetStructureType() == targetType) {
            return subStruct;
        }
        subStruct = subStruct->Next();
    }
    return NULL;
}

bool WriteGeom( LDJamFileMeshInfo *meshInfo, 
                OGEX::GeometryObjectStructure *geom,
                const OGEX::MaterialStructure *mtl,
                FILE *ldjamFP, void *contentBuff )
{
    // Use the gex name (usually geometry2 or something) but this should be 
    // replaced with the tkchunk.chunkname extension
    strncpy( meshInfo->m_name, geom->GetStructureName(), 32 );
    meshInfo->m_name[31] = 0;
    
    // HACK, we're not handling decal UVs consitantly
    bool flipDecalUVs = true;
    if (!strcmp( meshInfo->m_name, "gameboard")) {
        printf ("*** Not flipping decal UVs\n");
        flipDecalUVs = false;
    }
    
    if (mtl) {
         const Structure *curr = mtl->GetFirstSubnode();
        while (curr) {
            uint32_t structType = curr->GetStructureType();
            
            if (structType == kStructureTexture) {
                TextureStructure *tex = (OGEX::TextureStructure*)curr;
                
                
                if (tex->GetAttribString()=="diffuse") {
                    const String& stringTexName = tex->GetTextureName();
                    
                    const char *texPath = (const char *)stringTexName+2;
                    
                    // If the texture has a leading path, drop it
                    const char *ch = strrchr(texPath, '/');
                    if (ch) {
                        texPath = ch+1;
                    }
                    
                    strncpy( meshInfo->m_texture, texPath, 32 );
                    meshInfo->m_texture[31] = 0;
                    
                    // HACK for LD
                    printf("MESHNAME: %s\n", meshInfo->m_name );
                    if (!strcmp(meshInfo->m_name,"Crate")) {
                        strcpy(meshInfo->m_texture, "crate_DIF");
                    }
                    
                    char *ext = strstr( meshInfo->m_texture, ".png");
                    if (ext) {
                        *ext = '\0';
                    }
                
                }
                
                printf("geom %s, mtl %s tex %s -- \"%s\"\n",
                       (const char *)geom->GetStructureName(),
                       (const char *)mtl->GetStructureName(),
                       (const char *)tex->GetAttribString(),
                       meshInfo->m_texture
                       );
                
            }
            
            curr = curr->Next();
        }
    }

    // Get the points and figure the bounding box
    OGEX::MeshStructure *mesh = (OGEX::MeshStructure *)FindSubStructureOfType( geom, kStructureMesh );
    if (!mesh) {
        printf("Didnt find mesh struct in geom %s..\n", geom->GetStructureName() );
        return false;
    } 

    // Find the vertex arrays and metadata in mesh
    const Structure *curr = mesh->GetFirstSubnode();
    DataStructure<FloatDataType> *posData = NULL;
    DataStructure<FloatDataType> *nrmData = NULL;
    DataStructure<FloatDataType> *stData = NULL;
    DataStructure<FloatDataType> *stDecalData = NULL;
    DataStructure<UnsignedInt32DataType> *indexData = NULL;
    while (curr) {
        uint32_t structType = curr->GetStructureType();

        if (structType == kStructureVertexArray) {
            OGEX::VertexArrayStructure * vdata = (OGEX::VertexArrayStructure*)curr;
            printf("Vert Array: %s\n", (const char *)vdata->GetArrayAttrib() );
            String arrayAttrib = vdata->GetArrayAttrib();
            if (arrayAttrib == "position") {
                posData = (DataStructure<FloatDataType>*)vdata->GetFirstSubnode();
            } else if (arrayAttrib == "normal") {
                nrmData = (DataStructure<FloatDataType>*)vdata->GetFirstSubnode();
            } else if (arrayAttrib == "texcoord") {
                stData = (DataStructure<FloatDataType>*)vdata->GetFirstSubnode();
            } else if (arrayAttrib == "texcoord[1]") {
                stDecalData = (DataStructure<FloatDataType>*)vdata->GetFirstSubnode();
            }
        } else if (structType == kStructureIndexArray) {
            OGEX::IndexArrayStructure *indexArray = (OGEX::IndexArrayStructure*)curr;
            //uint32_t structType = indexData->GetFirstSubnode()->GetStructureType();
            //printf("Index Data: type is '%s'\n", FourCC2Str( structType ) );
            assert( indexArray->GetFirstSubnode()->GetStructureType() == kDataUnsignedInt32);
            indexData = (DataStructure<UnsignedInt32DataType>*)indexArray->GetFirstSubnode();
            
        } else if (structType == kStructureExtension) {
            
            // Look for chunk specific extensions
            OGEX::ExtensionStructure *extStruct = (OGEX::ExtensionStructure *)curr;
            if (extStruct->GetApplicationString() == "com.tapnik.tunnel") {
                
                if (extStruct->GetTypeString() == "tkchunk.chunk_name") {
                    Structure *nameSubnode = curr->GetFirstSubnode();
                    uint32_t structType = nameSubnode->GetStructureType();
                    if (structType == kDataString) {
                        DataStructure<StringDataType> *stringData = (DataStructure<StringDataType>*)curr->GetFirstSubnode();
                        const String& stringName = stringData->GetDataElement(0);
                        strncpy( meshInfo->m_name, (const char *)stringName, 32 );
                        meshInfo->m_name[31] = 0;
                    }
                }
            }
        }
        curr = curr->Next();
    }

    // Calc bounding box (todo copy vert attribs)
    if (posData) {
        AABB bbox;
        bbox.setNull();        
        uint32_t structType = posData->GetStructureType();
        uint32_t numPoints = posData->GetDataElementCount() / 3;
        
        memset( contentBuff, 0, SCENEFILE_CONTENT_BUFF_SIZE );
        LDJamFileMeshContent *meshContents = (LDJamFileMeshContent*)contentBuff;
        LDJamFileVertex *meshVert = (LDJamFileVertex*)(meshContents + 1);
        
        size_t meshContentSize = sizeof(LDJamFileMeshContent) + sizeof(LDJamFileVertex) * numPoints;
        uint16_t *meshIndices = (uint16_t*)( (uint8_t*)(meshVert) + sizeof(LDJamFileVertex) * numPoints );
        
        // TODO: maybe better to just track this?
        meshInfo->m_contentOffset = ftell( ldjamFP );

        meshContents->m_numVerts = numPoints;

        //printf("POS type %s size %d\n", FourCC2Str( structType ), posData->GetDataElementCount() );
        for (uint32_t i = 0; i < numPoints; i++) {
            glm::vec3 p( posData->GetDataElement(i*3+0),
                         posData->GetDataElement(i*3+1),
                         posData->GetDataElement(i*3+2) );
            
            // Grow bbox
            bbox.extend( p );

            // copy into vertex array
            meshVert->m_pos = p;
            
            if (nrmData != NULL) {
                glm::vec3 nrm( nrmData->GetDataElement(i*3+0),
                               nrmData->GetDataElement(i*3+1),
                               nrmData->GetDataElement(i*3+2) );
                meshVert->m_nrm = nrm;
            }

            if (stData != NULL) {
                
                glm::vec2 st( stData->GetDataElement(i*2+0),
                              1.0f - stData->GetDataElement(i*2+1));
                //printf("STData %d -- %f %f\n", i, st.x, st.y );
                meshVert->m_st0 = st;
                
                // Do we have decal STs?
                if ((stDecalData != NULL) && (stDecalData->GetDataElementCount() >= (int32_t)(i*2+1) )) {
                    glm::vec2 st1;
                    if (flipDecalUVs) {
                        st1 = glm::vec2( stDecalData->GetDataElement(i*2+0),
                                 1.0f - stDecalData->GetDataElement(i*2+1));
                    } else {
                        st1 = glm::vec2( stDecalData->GetDataElement(i*2+0),
                                         stDecalData->GetDataElement(i*2+1));
                    }
                    //printf("ST1Data %d -- %f %f\n", i, st1.x, st1.y );
                    meshVert->m_st1 = st1;
                } else {
                    meshVert->m_st1 = st;  // No decal UVs, just duplicate into ST1 now
                }
                
            }

            meshVert++;
        }

        meshInfo->m_bboxMin = bbox.getMin();
        meshInfo->m_bboxMax = bbox.getMax();

        // Copy in triangle indices
        assert( indexData != NULL);
        meshContents->m_triIndices = indexData->GetDataElementCount();
        uint16_t *currNdx = meshIndices;
        for (int32 i=0; i < indexData->GetDataElementCount(); i++) {
            *currNdx = (uint16_t)indexData->GetDataElement( i );
            currNdx++;
        }
        
        meshContentSize += sizeof(uint16_t) * indexData->GetDataElementCount();

        // HERE: TODO chunk texture data
             
        meshInfo->m_contentLength = meshContentSize;

        // Write the content for this chunk
        fwrite( contentBuff, meshContentSize, 1, ldjamFP );
    }

    return true;
}

bool WriteSceneObj( LDJamFileSceneObject *sceneObj,
                   const OGEX::GeometryNodeStructure *geomNode,
                   FILE *ldjamFP  )
{
    // The sceneObj meshIndex should already be filled in here...
    strncpy( sceneObj->m_name, (const char *)(geomNode->GetNodeName()), 32 );
    sceneObj->m_name[31] = 0;
    
    // Find the vertex arrays and metadata in mesh
    const Structure *curr = geomNode->GetFirstSubnode();
    TransformStructure *transformStruct = NULL;
    while (curr) {
        uint32_t structType = curr->GetStructureType();
        
        if (structType == kStructureTransform) {
            transformStruct = (OGEX::TransformStructure*)curr;
            
            if (transformStruct->GetTransformCount() != 1) {
                printf("WARNING: Expected 1 transform for node %s, got %d?\n", (const char *)(geomNode->GetNodeName()), transformStruct->GetTransformCount() );
            }
        }
        
        curr = curr->Next();
    }
    
    glm::mat4x4 transform = glm::mat4x4(); // identity
    if (transformStruct != NULL) {
        transform = glm::make_mat4( transformStruct->GetTransform() );
        transform = g_fixCoordsMatrix * transform;
        
        dbgPrintMatrix( "transform", transform );
    } else {
        printf("WARNING: Didn't find transform for '%s'\n", (const char *)(geomNode->GetNodeName()) );
    }
    sceneObj->m_transform = transform;
    
    // Write the content for this object
    fwrite( sceneObj, sizeof(LDJamFileSceneObject), 1, ldjamFP );
    
    return true;
}

bool WriteSceneCamera( LDJamFileSceneCamera *camera,
                      const OGEX::CameraNodeStructure *cameraNode,
                      const OGEX::CameraObjectStructure *cameraObject,
                      FILE *ldjamFP  )
{
    strncpy( camera->m_name, (const char *)(cameraNode->GetNodeName()), 32 );
    camera->m_name[31] = 0;
    
    const Structure *curr = cameraNode->GetFirstSubnode();
    TransformStructure *transformStruct = NULL;
    while (curr) {
        uint32_t structType = curr->GetStructureType();
        
        if (structType == kStructureTransform) {
            transformStruct = (OGEX::TransformStructure*)curr;
            
            if (transformStruct->GetTransformCount() != 1) {
                printf("WARNING: Expected 1 transform for camera %s, got %d?\n",
                       (const char *)(cameraNode->GetNodeName()),
                       transformStruct->GetTransformCount() );
            }
        }
        
        curr = curr->Next();
    }
    
    glm::mat4x4 transform = glm::mat4x4(); // identity
    if (transformStruct != NULL) {
        transform = glm::make_mat4( transformStruct->GetTransform() );
        transform = g_fixCoordsMatrix * transform;
        
        dbgPrintMatrix( "transform", transform );
    } else {
        printf("WARNING: Didn't find transform for '%s'\n", (const char *)(cameraNode->GetNodeName()) );
    }
    camera->m_transform = transform;
    
    // Write the content for this object
    fwrite( camera, sizeof(LDJamFileSceneCamera), 1, ldjamFP );
    
    return true;
}



struct MeshInfoTable {
    OGEX::GeometryObjectStructure *ogexGeom;
    LDJamFileMeshInfo *jamfileMesh;
};

// This is slow and bad but it's for a game jam sooo...
// Find the first GeomNode that uses geomObj and return the material from that
const OGEX::MaterialStructure *FindMaterialForGeom( const Structure *structure, OGEX::GeometryObjectStructure *geomTarget )
{
    while (structure)
    {
        uint32_t structType = structure->GetStructureType();
        if (structType == OGEX::kStructureGeometryNode) {
            const OGEX::GeometryNodeStructure *geomNode = (OGEX::GeometryNodeStructure*)structure;
            const OGEX::GeometryObjectStructure *geomObj = (OGEX::GeometryObjectStructure*)geomNode->GetObjectStructure();
            
            if (geomObj == geomTarget) {
                // found it, get the material
                const OGEX::MaterialStructure *mtl = geomNode->GetMaterial(0);
                if (mtl) {
                    return mtl;
                }
            }
        }
        structure = structure->Next();
    }
    return NULL;
}

// ======================================================================
// LDJAM Scene Tool main
// ======================================================================
int main( int argc, char *argv[] ) 
{    
    FILE *ldjamFP = NULL;
    size_t fileHeaderSize = sizeof(LDJamFileHeader) + sizeof(LDJamFileMeshInfo) * MAX_MESHES;
    uint8_t *fileHeader = (uint8_t*)malloc( fileHeaderSize );
    
    // To associate mesh info with geom
    MeshInfoTable meshTable[MAX_MESHES];
    int meshTableSize = 0;

    void *ldjamContentBuff = malloc( SCENEFILE_CONTENT_BUFF_SIZE );

    LDJamFileHeader *header = (LDJamFileHeader*)fileHeader;
    
    LDJamFileMeshInfo *firstMeshInfo = (LDJamFileMeshInfo*)(fileHeader + sizeof(LDJamFileHeader));
    LDJamFileMeshInfo *nextMeshInfo = firstMeshInfo;

    // write out a blank header, fill it in after gathering all the chunks
    memset( fileHeader, 0, fileHeaderSize );

    header->m_fourCC = 'LD48';
    header->m_fileVersion = LDJAMFILE_VERSION;

    printf("%s\n", argv[0] );

    if (argc < 3) {
        printf("Usage: ldjamtool <filename.ogex> <outfile.ldjam>\n");
        return 1;
    }

    // Read the input OpenGex File
    size_t fileSize = 0;
    char *buffer = (char*)ReadEntireFile( argv[1], &fileSize );
    if (!buffer) {
        printf("Error reading input file: %s\n", argv[1] );
        return 1;
    }
    buffer[fileSize] = 0;
    printf("Read %zu bytes\n", fileSize );

    // Parse the OGEX structure and extact the geometry
    OpenGexDataDescription  openGexDataDescription;
    DataResult result = openGexDataDescription.ProcessText( buffer );
    if (result != kDataOkay) 
    {
        printf("Failed to parse ogex, Error Code [%s] on line %d -- %s\n", 
            "????", //FourCC2Str(result), 
			
			openGexDataDescription.GetErrorLine(),
            DescribeError( result ) );

        exit(1);
    }

    // Make the output file
    ldjamFP = fopen( argv[2], "wb");
    if (!ldjamFP) {
        printf("Failed to open ldjam output file '%s'\n", argv[2] );
        exit(1);
    }

    // Write out a placeholder header
    fwrite( fileHeader, fileHeaderSize, 1, ldjamFP );

    const Structure *structure = openGexDataDescription.GetRootStructure()->GetFirstSubnode();

    // Build up scene db
    while (structure)
    {
        uint32_t structType = structure->GetStructureType();
//                if (structType == OGEX::kStructureMetric) {
//                    const OGEX::MetricStructure *metric = (OGEX::MetricStructure*)structure;
//                    printf("Metric: %s\n", (const char *)metric->GetMetricKey() );
//                }
        
        OGEX::GeometryObjectStructure *geomObj = (OGEX::GeometryObjectStructure*)structure;

        // Add all the geometries to the chunk file buffer
        if (structType == OGEX::kStructureGeometryObject) {
            assert( header->m_numChunks < (MAX_MESHES-1) );
            
            // Get the material for this Geom
            const OGEX::MaterialStructure *mtl =  FindMaterialForGeom( openGexDataDescription.GetRootStructure()->GetFirstSubnode(),
                                                                geomObj );
            
            if (WriteGeom( nextMeshInfo, geomObj, mtl, ldjamFP, ldjamContentBuff )) {
                
                // Add to the mesh table so we can find it again
                MeshInfoTable &meshAssoc = meshTable[meshTableSize++];
                meshAssoc.ogexGeom = (OGEX::GeometryObjectStructure*)structure;
                meshAssoc.jamfileMesh = nextMeshInfo;
                
                header->m_numChunks++;
                nextMeshInfo++;
            }
        }

        /*
        const OGEX::MetricStructure *metric = dynamic_cast<const OGEX::MetricStructure *>(structure);
        if (metric) {
                printf("Metric: %s\n", (const char *)metric->GetMetricKey() );
        }

        const OGEX::GeometryNodeStructure *geomNode = dynamic_cast<const OGEX::GeometryNodeStructure *>(structure);
        if (geomNode) {
                printf("Geometry Node: %s\n", geomNode->GetNodeName() );
                //const Map<MeshStructure> *meshMap = geomNode->GetMeshMap();
        }
        
        const OGEX::MaterialStructure *material = dynamic_cast<const OGEX::MaterialStructure *>(structure);
        if (material) {
            printf("Material: %s\n", material->GetMaterialName() );
        }
        */
        structure = structure->Next();
    }
    
    // Now encode all the scene nodes
    printf("Dumping scene objects...\n");
    header->m_sceneObjOffs = ftell( ldjamFP );
    
    structure = openGexDataDescription.GetRootStructure()->GetFirstSubnode();
    while (structure)
    {
        uint32_t structType = structure->GetStructureType();
        //printf(" struct... %s type '%s'\n",
        //       structure->GetStructureName(),
        //       FourCC2Str( structType ) );

          if (structType == OGEX::kStructureGeometryNode) {
              const OGEX::GeometryNodeStructure *geomNode = (OGEX::GeometryNodeStructure*)structure;
              const OGEX::GeometryObjectStructure *geomObj = (OGEX::GeometryObjectStructure*)geomNode->GetObjectStructure();
              
              // Find it in the mesh table
              int meshIndex = -1;
              for (int i=0; i<meshTableSize; i++) {
                  if (meshTable[i].ogexGeom == geomObj) {
                      meshIndex = i;
                      break;
                  }
              }
              
              if (meshIndex == -1) {
                  printf("Geometry Node: %s GEOM NOT FOUND\n", (const char *)geomNode->GetNodeName() );
              } else {
                  printf("Geometry Node: %s GEOM %d [%s]\n",
                         (const char *)geomNode->GetNodeName(), meshIndex,
                         meshTable[meshIndex].jamfileMesh->m_name );
                  
                  LDJamFileSceneObject sceneObj;
                  sceneObj.m_meshIndex = meshIndex;
                  WriteSceneObj(&sceneObj, geomNode, ldjamFP );
                  
                  header->m_numSceneObjs++;
              }
          }
        
        structure = structure->Next();
    }

    
    // Now encode all the cameras
    printf("Dumping cameras...\n");
    header->m_cameraOffs = ftell( ldjamFP );
    
    structure = openGexDataDescription.GetRootStructure()->GetFirstSubnode();
    while (structure)
    {
        uint32_t structType = structure->GetStructureType();
        //printf(" struct... %s type '%s'\n",
        //       structure->GetStructureName(),
        //       FourCC2Str( structType ) );
        
        if (structType == OGEX::kStructureCameraNode) {

            const OGEX::CameraNodeStructure *camNode = (OGEX::CameraNodeStructure*)structure;
            
            // TODO: Find the maching CameraObject
            const OGEX::CameraObjectStructure *camObject = NULL;
            
            printf("Camera Node: %s CAM [%s]\n",
                       (const char *)camNode->GetNodeName(), "???"  );
                
            LDJamFileSceneCamera camera;
            WriteSceneCamera(&camera, camNode, camObject, ldjamFP );
                
            header->m_numCameras++;
        }
        
        structure = structure->Next();
    }
    

    printf("HEADER: %d chunks...\n", header->m_numChunks );
    for (uint32_t i=0; i < header->m_numChunks; i++) {
        LDJamFileMeshInfo *info = firstMeshInfo + i;

        printf("----------------------------\n");
        printf("SceneObj Info Name: '%s'\n", info->m_name );        
        printf("BBOX Min: %3.2f %3.2f %3.2f\n", info->m_bboxMin.x, info->m_bboxMin.y, info->m_bboxMin.z );
        printf("BBOX Max: %3.2f %3.2f %3.2f\n", info->m_bboxMax.x, info->m_bboxMax.y, info->m_bboxMax.z );
        printf("Content Offset: %du Content Len: %du\n", 
			info->m_contentOffset, info->m_contentLength );
    }

    // Rewrite the header
    fseek( ldjamFP, 0L, SEEK_SET );
    fwrite( header, fileHeaderSize, 1, ldjamFP );
    fclose(ldjamFP );

    
    free(buffer);
    free(fileHeader);




}
