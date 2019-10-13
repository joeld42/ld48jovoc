#ifndef TK_HEXTILE_BUILDER_H
#define TK_HEXTILE_BUILDER_H

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/random.hpp"

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

class HexTileBuilder
{
public:
	HexTileBuilder();

	Tapnik::SceneMesh* BuildHexMesh( Tapnik::Scene *scene, float hexSize);

	Oryol::Id testTex;

private:
	uint16_t* nbrTable;
	uint8_t* scratchBuffer;

	void _SetupScratchBuffer();
	MeshData _BuildBaseMesh(float hexSize);
	MeshData _SubdivideMesh(MeshData origMesh);
	LDJamFileVertex _EdgeSplit(LDJamFileVertex vA, LDJamFileVertex vB);
	void _PushTri(uint16_t** indexTop, uint16_t a, uint16_t b, uint16_t c);
	void _DumpOBJFile(const char* filename, MeshData meshdata );

	void _ClearNbrTable();
	uint16_t _CheckForExistingSplit(uint16_t a, uint16_t b);
	void _AddSplit(uint16_t a, uint16_t b, uint16_t ab);

};


#endif