#ifndef TK_HEXTILE_BUILDER_H
#define TK_HEXTILE_BUILDER_H

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/random.hpp"

#include "open-simplex-noise.h"

#include "SceneObject.h"

struct LDJamFileVertex;

struct MeshData
{
	uint8_t* meshData;
	size_t meshDataSize;
	int numVerts;
	int numTris;
	LDJamFileVertex* verts;
	uint16_t* indexes;
};

struct TerrainInfo {
	glm::vec4 albedo;
	float height;
};

class HexTileBuilder
{
public:
	HexTileBuilder();

	Tapnik::SceneMesh* BuildHexMesh( Tapnik::Scene *scene, float hexSize);

private:
	uint16_t* nbrTable;
	uint8_t* scratchBuffer;

	TerrainInfo* _terrain;

	Oryol::Id texTileAlbedo;

	void _SetupScratchBuffer();
	MeshData _BuildBaseMesh(float hexSize);
	MeshData _SubdivideMesh(MeshData origMesh);
	void _ApplyHeight(MeshData& mesh);
	void _CalcNormals(MeshData& mesh);
	LDJamFileVertex _EdgeSplit(LDJamFileVertex vA, LDJamFileVertex vB);
	void _PushTri(uint16_t** indexTop, uint16_t a, uint16_t b, uint16_t c);
	void _DumpOBJFile(const char* filename, MeshData meshdata );
	void _GenerateTerrain();
	void _BuildTextures();

	// Terrain stuff
	TerrainInfo _EvalTerrain(glm::vec2 uv, float hexId);

	// Edge resuse table
	void _ClearNbrTable();
	uint16_t _CheckForExistingSplit(uint16_t a, uint16_t b);
	void _AddSplit(uint16_t a, uint16_t b, uint16_t ab);

	static osn_context* noiseCtx;

	

};


#endif