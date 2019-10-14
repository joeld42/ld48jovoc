#include "Core/Types.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/random.hpp"

#include "open-simplex-noise.h"

#include "SceneObject.h"
#include "LDJamFile.h"

#include "HexTileBuilder.h"

using namespace Oryol;
using namespace Tapnik;


osn_context* HexTileBuilder::noiseCtx = NULL;

// TODO: figure out the exact bounds of this from
// our max subdivision level
#define SCRATCH_BUFF_SZ (1024*1024*2)
#define NBR_TABLE_SZ (1000)

#define NBR_INVALID  (0xFFFF)

#define TILE_RES (128)

HexTileBuilder::HexTileBuilder() : scratchBuffer( NULL ), _terrain( NULL)
{
}

void HexTileBuilder::_SetupScratchBuffer()
{
	if (scratchBuffer == NULL) {
		scratchBuffer = (uint8_t*)Memory::Alloc(SCRATCH_BUFF_SZ);
		nbrTable = (uint16_t*)Memory::Alloc(NBR_TABLE_SZ * sizeof(uint16_t));
	 }

	if (noiseCtx == NULL) {
		open_simplex_noise( 1234, &noiseCtx);
	}

	if (_terrain == NULL)
	{
		size_t terrainSz = TILE_RES * TILE_RES * sizeof(TerrainInfo);
		_terrain = (TerrainInfo*)Memory::Alloc(terrainSz);
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

void HexTileBuilder::_ApplyHeight(MeshData& mesh)
{
	for (int i = 0; i < mesh.numVerts; i++)
	{
		glm::vec2 st = mesh.verts[i].m_st0;
		int uu = st.x * TILE_RES;
		int vv = st.y * TILE_RES;
		float h = _terrain[vv * TILE_RES + uu].height;
		mesh.verts[i].m_pos.z = h * 0.2f;
	}
}

void HexTileBuilder::_CalcNormals(MeshData& mesh)
{
	for (int i = 0; i < mesh.numVerts; i++) {
		mesh.verts[i].m_nrm = glm::vec3(0.0f);
	}

	for (int i = 0; i < mesh.numTris; i++) {
		uint16_t ndxA = mesh.indexes[i * 3 + 0];
		uint16_t ndxB = mesh.indexes[i * 3 + 1];
		uint16_t ndxC = mesh.indexes[i * 3 + 2];

		glm::vec3 edge1 = mesh.verts[ndxB].m_pos - mesh.verts[ndxA].m_pos;
		glm::vec3 edge2 = mesh.verts[ndxC].m_pos - mesh.verts[ndxA].m_pos;
		
		glm::vec3 faceNorm = glm::normalize( glm::cross(edge1, edge2) );
		mesh.verts[ndxA].m_nrm += faceNorm;
		mesh.verts[ndxB].m_nrm += faceNorm;
		mesh.verts[ndxC].m_nrm += faceNorm;
	}

	for (int i = 0; i < mesh.numVerts; i++) {
		mesh.verts[i].m_nrm = glm::normalize(mesh.verts[i].m_nrm);
	}
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

	_GenerateTerrain();

	MeshData baseMesh = _BuildBaseMesh( hexSize );
	
	MeshData subdMesh = baseMesh;
	for (int i = 0; i < 3; i++)
	{
		subdMesh = _SubdivideMesh(subdMesh);
		_ApplyHeight(subdMesh);
	}

	_CalcNormals(subdMesh);

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

	// Build the texture maps for this hex
	_BuildTextures();

	SceneMesh* hexMesh = scene->BuildMesh("Hex_MESH", texTileAlbedo, subdMesh.meshData,
		subdMesh.numVerts, subdMesh.numTris * 3);

	return hexMesh;
}

// cosine based palette, 4 vec3 params
glm::vec3 palette(float t, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d)
{
	return a + b * cos(6.28318f * (c * t + d));
}

float HexDist( glm::vec2 p) {
	p = glm::abs(p);

	float c = glm::dot(p, glm::normalize(glm::vec2(1, 1.73)));
	c = glm::max(c, p.x);

	return c;
}


TerrainInfo HexTileBuilder::_EvalTerrain(glm::vec2 uv, float hexId )
{
	TerrainInfo res = {};

	float t = open_simplex_noise3( noiseCtx, uv.x * 6.0, uv.y * 6.0, hexId );

	glm::vec2 uvNorm = (uv * 2.0f) + glm::vec2(-1.0f);
	float hd = HexDist( uvNorm );
	glm::vec3 color = palette( hd, glm::vec3(0.5f, 0.5f, 0.5f),
		glm::vec3(0.5f, 0.5f, 0.5f),
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(0.0f, 0.33f, 0.67f ));

	res.height = t;

	float edge = glm::smoothstep(0.41f, 0.42f, hd);
	res.albedo = glm::mix(glm::vec4(color.r, color.g, color.b, 1.0), glm::vec4(1.0f), edge);
	//res.albedo = glm::mix(glm::vec4(color.r, color.g, color.b, 1.0), glm::vec4(1.0f), fabs( t ));
	
	// TODO::

	return res;
}

void HexTileBuilder::_GenerateTerrain()
{
	float cellId = glm::linearRand(0.0f, 100.0f);

	for (int j = 0; j < TILE_RES; j++) {
		float jj = (float)j / (float)TILE_RES;
		for (int i = 0; i < TILE_RES; i++) {

			float ii = (float)i / (float)TILE_RES;

			int ndx = (j * TILE_RES) + i;
			_terrain[ndx] = _EvalTerrain(glm::vec2(ii, jj), cellId );
		}
	}
}

void HexTileBuilder::_BuildTextures()
{
	const int texDataSize = TILE_RES * TILE_RES * PixelFormat::ByteSize(PixelFormat::RGBA8);
	uint32_t* texData = (uint32_t*)Memory::Alloc(texDataSize);

	for (int j = 0; j < TILE_RES; j++) {
		for (int i = 0; i < TILE_RES; i++) {
			int ndx = (j * TILE_RES) + i;
			TerrainInfo nfo = _terrain[ndx];

			int r = nfo.albedo.r * 255;
			int g = nfo.albedo.g * 255;
			int b = nfo.albedo.b * 255;
			uint32_t c = 0xFF000000 | (r << 16) | (g << 8) | b;
			texData[ndx] = c;
		}
	}

	auto texSetup = TextureSetup::FromPixelData2D( TILE_RES, TILE_RES, 1, PixelFormat::RGBA8);
	texSetup.Sampler.MinFilter = TextureFilterMode::Linear;
	texSetup.Sampler.MagFilter = TextureFilterMode::Linear;
	texSetup.Sampler.WrapU = TextureWrapMode::ClampToEdge;
	texSetup.Sampler.WrapV = TextureWrapMode::ClampToEdge;
	texSetup.ImageData.Sizes[0][0] = texDataSize;
	texTileAlbedo = Gfx::CreateResource(texSetup, texData, texDataSize);

	// TODO: Reuse this for other texture
	Memory::Free(texData);
}
