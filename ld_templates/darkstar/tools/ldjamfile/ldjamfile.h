#ifndef LDJAMFILE_H
#define LDJAMFILE_H

// LDJamFile is exported by ldjamtool (from OGEX) and ingested
// by runtime. This is the only part that is shared between ldjamtool and
// runtime.
#include <stdint.h>

#include "glm/glm.hpp"


#pragma pack(push, 1)

struct LDJamFileHeader 
{
	uint32_t m_fourCC;    // Expects 'LD48'
    uint32_t m_fileVersion;
	
    uint32_t m_numChunks;
    
    uint32_t m_numSceneObjs;
    uint32_t m_sceneObjOffs;  // After chunks? need to clean this up
};


// Info for each chunk. Infos for all chunks are always loaded with the
// db header.
struct LDJamFileMeshInfo
{
	char m_name[32];	
	glm::vec4 m_tintColor;

	glm::vec3 m_bboxMin;
	glm::vec3 m_bboxMax;

	uint32_t m_contentOffset;
	uint32_t m_contentLength;
};

struct LDJamFileMeshContent
{	
	uint32_t m_numVerts;
	uint32_t m_numTris;
	uint16_t m_triIndices;
};

struct LDJamFileSceneObject
{
    char m_name[32];
    uint32_t m_meshIndex; // Mesh index in the ordering it is in this file
    glm::mat4x4 m_transform;
    
    // TODO: Material
};

struct LDJamFileVertex
{
	glm::vec3 m_pos;
	glm::vec3 m_nrm;
	glm::vec2 m_st0;  // Base texture ST
	glm::vec2 m_st1;  // Lightmap ST
};

#pragma pack(pop)

void LDJamFileVertexTest();

#endif
