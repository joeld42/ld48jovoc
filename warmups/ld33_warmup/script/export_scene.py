import os, sys
import json
import configparser

import bpy, bpy_types
import bmesh
import struct

# TODO:
# - Apply modifiers before export
# - Export list of assets and textures (and code to preload)

# e.g.  /Applications/Blender/blender.app/Contents/MacOS/blender -b srcart/forest.blend -P script/export_scene.py -- ld33_export.cfg


def texturesForObject( obj  ):
	result = []
	for mat_slot in obj.material_slots:
		for mtex_slot in mat_slot.material.texture_slots:
			if mtex_slot:
				if hasattr(mtex_slot.texture , 'image'):
					imgfile = mtex_slot.texture.image.filepath
					result.append( imgfile )
	print(result)
	return result

def exportMeshObj( mesh, meshExportName ):
	
	#DBG
	#if (meshExportName.find('TreeOakMesh')==-1):
	#		return

	# Triangulate the mesh
	bm = bmesh.new()
	bm.from_mesh(mesh)
	bmesh.ops.triangulate( bm, faces=bm.faces )
	bm.to_mesh(mesh)
	bm.free()

	# This could be better..
	# HEADER:
	# 'MESH'          4cc (byte[4])
	# num_triangles   uint32_t 	
	header = struct.pack( '<4sL', str.encode("MESH"), len(mesh.polygons) )

	packedDataList = [ header ]
	uv_layer = mesh.uv_layers['UVMap'].data[:]		
	# print(uv_layer)
	# print ("NUM UV", len(uv_layer))
	# print ("NUM VERT", len(mesh.vertices))
	stndx = 0;
	for face in mesh.polygons:
		verts = face.vertices[:]		

		# for now, only triangles
		assert(len(verts)==3)

		packedTri = []

		for vndx in verts:
			v = mesh.vertices[vndx]
			uv = uv_layer[stndx]
			stndx += 1
			#print(v.co, v.normal, uv.uv )

			# pack up the vert data
			packedVert = struct.pack( '<3f3f4f', 
				v.co[0], v.co[2], v.co[1],
				v.normal[0], v.normal[1], v.normal[2],
				uv.uv[0], uv.uv[1], 0.0, 0.0 );

			packedTri.append(packedVert)

		packedTri.reverse()
		packedDataList += packedTri

	# Write the mesh data
	packedData = b''.join( packedDataList )

	print ("----", meshExportName, len(packedData), len(mesh.polygons) )

	with open( meshExportName, 'bw') as fp:
		fp.write( packedData )



def getConfig():
	# Extract the script arguments
	argv = sys.argv
	
	try:
	    index = argv.index("--") + 1
	except:
	    index = len(argv)
	argv = argv[index:]

	print ("Argv is ", argv)

	# read the export config
	cfg = configparser.ConfigParser()
	if (len(argv)==0):
		return None

	cfg.read(argv[0])
	return cfg


def exportScene( cfg, sceneName ):
	meshes = {}

	for obj in bpy.data.objects:		
		if type(obj.data) == bpy_types.Mesh and not obj.data.name in meshes:

			# Only export objects on layer 0
			if not obj.layers[0]:
				continue

			meshes[obj.data.name] = obj.data
		
	print ("Exporting Meshes...")
	meshPath = cfg['Paths']['MeshPath']
	for name, mesh in meshes.items():

		meshExportName = os.path.join( meshPath, "MESH_" + name + ".dat" )
		exportMeshObj( mesh, meshExportName )

	print ("Exporting Scene...")
	scenePath = cfg['Paths']['ScenePath']
	sceneObjs = []
	for obj in bpy.data.objects:

		# Only export objects on layer 0
		if not obj.layers[0]:
			continue

		if type(obj.data) != bpy_types.Mesh:
			continue			

		sceneObj = { "name" : obj.name,
					"loc"  : (obj.location.x, obj.location.z, obj.location.y),
					"rot"  : tuple(obj.rotation_euler),
					"scl"  : (obj.scale.x, obj.scale.z, obj.scale.y ),
					"mesh" : "MESH_" + obj.data.name }

		textures = texturesForObject( obj )
		print (obj.name, textures )
		if (len(textures) > 1):
			print ("WARNING: Object ", obj.name, " has multiple textures")

		if (len(textures)==0):
			print("WARNING: Object ", obj.name, " missing texture" );
			textures = [ 'missing.png']

		sceneObj['texture'] = os.path.split(textures[0])[-1]



		sceneObjs.append( sceneObj )

		# locStr = "%f,%f,%f" % tuple(obj.location)
		# rotStr = "%f,%f,%f" % tuple(obj.rotation_euler)

		# if type(obj.data) == bpy_types.Mesh:
		# 	print ("MESH: ", obj.name, obj.data.name, locStr, rotStr )
	
	sceneFile = os.path.join( scenePath, sceneName + ".json")
	print (sceneFile)
	with open(sceneFile, 'w') as fp:
		json.dump( sceneObjs, fp, sort_keys=True, indent=4, separators=(',', ': '))


def main():
	print ("LD exporter...")

	# Get the original scene name (todo: make better)	
	for a in range(len(sys.argv)):
		if (sys.argv[a]=='--'):
			break
		if (sys.argv[a]=='-b'):
			sceneName = sys.argv[a+1]
	
	sceneName = os.path.basename( sceneName )
	sceneName = os.path.splitext( sceneName )[0]


	cfg = getConfig()
	if cfg is None:
		print ("ERROR: Missing config, export stopped")
		return

	exportScene(cfg, sceneName )

	


if __name__=='__main__':
	main()
