#include "Core/Types.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/random.hpp"

#include "SceneObject.h"
#include "LDJamFile.h"

#include "HexTileBuilder.h"

using namespace Oryol;
using namespace Tapnik;

// TODO: figure out the exact bounds of this from
// our max subdivision level
#define SCRATCH_BUFF_SZ (1024*1024*2)
#define NBR_TABLE_SZ (1000)

#define NBR_INVALID  (0xFFFF)

HexTileBuilder::HexTileBuilder() : scratchBuffer( NULL )
{
}

void HexTileBuilder::_SetupScratchBuffer()
{
	if (scratchBuffer == NULL) {
		scratchBuffer = (uint8_t*)Memory::Alloc(SCRATCH_BUFF_SZ);
		nbrTable = (uint16_t*)Memory::Alloc(NBR_TABLE_SZ * sizeof(uint16_t));
	 }
}

// build the base mesh into the scratch buffer
MeshData HexTileBuilder::_BuildBaseMesh(float hexSize)
{
	MeshData mesh = {};

	mesh.numVerts = 7; // six + center
	mesh.numTris = 6;

	size_t vertBufferSize = sizeof(LDJamFileVertex) * mesh.numVerts;
	size_t indexBufferSize = sizeof(uint16_t) * mesh.numTris * 3;
	size_t geomBufferSize = vertBufferSize + indexBufferSize;

	float r = 2.0f;
	float s3 = (sqrt(3.0) * r) / 2.0;
	float r2 = r / 2.0f;
	uint8_t* geomBuffer = scratchBuffer;
	mesh.meshData = geomBuffer;
	LDJamFileVertex* verts = (LDJamFileVertex*)mesh.meshData;
	verts[0].m_pos = glm::vec3(0.0f, 0.0f, 0.0f);

	verts[1].m_pos = glm::vec3(r, 0.0f, 0.0f);
	verts[2].m_pos = glm::vec3(r2, s3, 0.0f);
	verts[3].m_pos = glm::vec3(-r2, s3, 0.0f);
	verts[4].m_pos = glm::vec3(-r, 0.0f, 0.0f);
	verts[5].m_pos = glm::vec3(-r2, -s3, 0.0f);
	verts[6].m_pos = glm::vec3(r2, -s3, 0.0f);

	for (int i = 0; i < mesh.numVerts; i++) {
		glm::vec2 st = glm::vec2(verts[i].m_pos) * glm::vec2(0.25f, -0.25f);
		verts[i].m_nrm = glm::vec3(0.f, 0.f, 1.f);
		verts[i].m_st0 = st + glm::vec2(0.5f, 0.5f);

		verts[i].m_pos *= hexSize / 2.0;
	}

	uint16_t * indexes = (uint16_t*)(geomBuffer + vertBufferSize);
	indexes[0] = 0;
	indexes[1] = 1;
	indexes[2] = 2;

	indexes[3] = 0;
	indexes[4] = 2;
	indexes[5] = 3;

	indexes[6] = 0;
	indexes[7] = 3;
	indexes[8] = 4;

	indexes[9] = 0;
	indexes[10] = 4;
	indexes[11] = 5;

	indexes[12] = 0;
	indexes[13] = 5;
	indexes[14] = 6;

	indexes[15] = 0;
	indexes[16] = 6;
	indexes[17] = 1;

	mesh.verts = verts;
	mesh.indexes = indexes;

	mesh.meshDataSize = geomBufferSize;

	return mesh;
}

LDJamFileVertex HexTileBuilder::_EdgeSplit(LDJamFileVertex vA, LDJamFileVertex vB)
{
	LDJamFileVertex v = {};

	v.m_pos = (vA.m_pos + vB.m_pos) * 0.5f;
	v.m_nrm = (vA.m_nrm + vB.m_nrm) * 0.5f; // normalize later
	v.m_st0 = (vA.m_st0 + vB.m_st0) * 0.5f;
	v.m_st1 = (vA.m_st1 + vB.m_st1) * 0.5f;

	return v;
}

void HexTileBuilder::_ClearNbrTable()
{
	Memory::Fill(nbrTable, sizeof(uint16_t) * NBR_TABLE_SZ, 0xFF);
}

uint16_t HexTileBuilder::_CheckForExistingSplit(uint16_t a, uint16_t b)
{
	if (a > b) {
		uint16_t tmp = a;
		a = b;
		b = tmp;
	}

	uint16_t base = a * 12; //max 6 neighbors for each vert
	assert(base < NBR_TABLE_SZ);

	for (int i = 0; i < 6; i++) {
		uint16_t ndx = nbrTable[base + i*2 + 0];
		if (ndx == NBR_INVALID) return NBR_INVALID;
		if (ndx == b) {
			return nbrTable[base + i*2 + 1];
		}
	}
	return NBR_INVALID;
}

void HexTileBuilder::_AddSplit(uint16_t a, uint16_t b, uint16_t ab)
{
	if (a > b) {
		uint16_t tmp = a;
		a = b;
		b = tmp;
	}

	uint16_t base = a * 12; //max 6 neighbors for each vert
	assert(base < NBR_TABLE_SZ);

	for (int i = 0; i < 6; i++) {
		uint16_t ndx = nbrTable[base + i*2 + 0];
		if (ndx == NBR_INVALID) {
			nbrTable[base + i*2 + 0] = b;
			nbrTable[base + i*2 + 1] = ab;
			return;
		}
	}
	// We should never have a vert with more than 6 neighbors
	assert(0);
}

void HexTileBuilder::_PushTri(uint16_t **indexTop, uint16_t a, uint16_t b, uint16_t c)
{
	uint16* vndx = *indexTop;

	*vndx++ = a;
	*vndx++ = b;
	*vndx++ = c;
	*indexTop = vndx;
}
MeshData HexTileBuilder::_SubdivideMesh(MeshData origMesh)
{
	MeshData resultMesh = {};

	_ClearNbrTable();

	// Start the subdivided mesh after the previous mesh in the scratch buffer
	int maxNewVerts = origMesh.numVerts * 4; // TODO: make smaller once we're merging verts
	resultMesh.meshData = origMesh.meshData + origMesh.meshDataSize;
	resultMesh.verts = (LDJamFileVertex*)resultMesh.meshData;
	resultMesh.indexes = (uint16_t*)(resultMesh.verts + maxNewVerts);

	// Make new mesh
	resultMesh.numTris = 0;

	// start with the original verts
	resultMesh.numVerts = origMesh.numVerts;
	uint16_t* vndx = resultMesh.indexes;
	Memory::Copy(origMesh.verts, resultMesh.verts, sizeof(LDJamFileVertex) * origMesh.numVerts);
	
	int currVertNdx = resultMesh.numVerts;
	for (int tndx = 0; tndx < origMesh.numTris; tndx++)
	{
		// TODO don't split triangle if all edges are flat
		int ndxA = origMesh.indexes[tndx * 3 + 0];
		int ndxB = origMesh.indexes[tndx * 3 + 1];
		int ndxC = origMesh.indexes[tndx * 3 + 2];

		assert(ndxA < origMesh.numTris * 3);
		assert(ndxB < origMesh.numTris * 3);
		assert(ndxC < origMesh.numTris * 3);

		// Subdivide and add verts
		uint16_t ndxAB = _CheckForExistingSplit(ndxA, ndxB);
		Log::Info("Result is %d\n", ndxAB);
		if (ndxAB == NBR_INVALID) {
			ndxAB = currVertNdx++;
			LDJamFileVertex* vAB = resultMesh.verts + ndxAB;
			*vAB = _EdgeSplit(resultMesh.verts[ndxA], resultMesh.verts[ndxB]);
			_AddSplit(ndxA, ndxB, ndxAB);
		}

		uint16_t ndxBC = _CheckForExistingSplit(ndxB, ndxC);
		if (ndxBC == NBR_INVALID) {
			ndxBC = currVertNdx++;
			LDJamFileVertex* vBC = resultMesh.verts + ndxBC;
			*vBC = _EdgeSplit(resultMesh.verts[ndxB], resultMesh.verts[ndxC]);
			_AddSplit(ndxB, ndxC, ndxBC);
		}

		uint16_t ndxCA = _CheckForExistingSplit(ndxC, ndxA);
		if (ndxCA == NBR_INVALID) {
			ndxCA = currVertNdx++;
			LDJamFileVertex* vCA = resultMesh.verts + ndxCA;
			*vCA = _EdgeSplit(resultMesh.verts[ndxC], resultMesh.verts[ndxA]);
			_AddSplit(ndxC, ndxA, ndxCA);
		}		

		Log::Info("Indexes ABC %d %d %d SPlits %d %d %d\n",
			ndxA, ndxB, ndxC,
			ndxAB, ndxBC, ndxCA);

		// Add the split triangles
		_PushTri(&vndx, ndxA, ndxAB, ndxCA);
		_PushTri(&vndx, ndxB, ndxBC, ndxAB);
		_PushTri(&vndx, ndxC, ndxCA, ndxBC);
		_PushTri(&vndx, ndxAB, ndxBC, ndxCA);

		resultMesh.numTris += 4;
	}
	Log::Info("numverts before %d after %d mult %f\n", resultMesh.numVerts, currVertNdx,
		(float)currVertNdx / (float)resultMesh.numVerts );


	resultMesh.numVerts = currVertNdx;
	
	// Now compact everything into the beginning of the
	// scratch buffer (note this destroys origMesh and any intermediate meshes we have built)
	MeshData compactedMesh = {};
	compactedMesh.numVerts = resultMesh.numVerts;
	compactedMesh.numTris = resultMesh.numTris;
	compactedMesh.meshData = scratchBuffer;
	compactedMesh.meshDataSize = (sizeof(LDJamFileVertex) * compactedMesh.numVerts) + (sizeof(uint16_t) * compactedMesh.numTris * 3);
	compactedMesh.verts = (LDJamFileVertex*)compactedMesh.meshData;
	compactedMesh.indexes = (uint16_t*)(compactedMesh.verts + compactedMesh.numVerts);
	// note: possible overlapping
	Memory::Copy(resultMesh.verts, compactedMesh.verts, sizeof(LDJamFileVertex) * compactedMesh.numVerts);
	Memory::Copy(resultMesh.indexes, compactedMesh.indexes, sizeof(uint16_t) * compactedMesh.numTris * 3);

	return compactedMesh;
}

void HexTileBuilder::_DumpOBJFile(const char* filename, MeshData mesh)
{
	FILE* fp = fopen( filename , "wt");
	if (!fp) {
		Log::Warn("Can't open mesh file!\n");
	}
	else {
		for (int i = 0; i < mesh.numVerts; i++) {
			fprintf(fp, "v %f %f %f\n", mesh.verts[i].m_pos[0], mesh.verts[i].m_pos[2], mesh.verts[i].m_pos[1]);
		}

		for (int i = 0; i < mesh.numTris; i++) {
			fprintf(fp, "f %d %d %d\n", mesh.indexes[i * 3 + 0] + 1, mesh.indexes[i * 3 + 1] + 1, mesh.indexes[i * 3 + 2] + 1);
		}
		fclose(fp);
	
	}
}

Tapnik::SceneMesh* HexTileBuilder::BuildHexMesh( Tapnik::Scene *scene, float hexSize)
{
	_SetupScratchBuffer();
	MeshData baseMesh = _BuildBaseMesh( hexSize );

	MeshData subdMesh = baseMesh;
	for (int i = 0; i < 3; i++)
	{
		subdMesh = _SubdivideMesh(subdMesh);
	}
	// Copy the final mesh into the geomBuffer
	uint8_t *geomBuffer = (uint8_t*)Memory::Alloc(subdMesh.meshDataSize);
	Memory::Copy( subdMesh.meshData, geomBuffer, subdMesh.meshDataSize);

#if 1
	static bool didWriteMesh = false;
	if (!didWriteMesh) {
		_DumpOBJFile("c:\\oprojects\\ld45_nothing\\hex_dump.obj", subdMesh );
		didWriteMesh = true;
		Log::Info("Wrote OBJ hex\n");
	}
#endif


	SceneMesh* hexMesh = scene->BuildMesh("Hex_MESH", testTex, subdMesh.meshData,
		subdMesh.numVerts, subdMesh.numTris * 3);

	return hexMesh;
}
