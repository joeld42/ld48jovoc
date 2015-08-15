import os, sys
import configparser

import bpy, bpy_types
import bmesh
import struct

# e.g.  /Applications/Blender/blender.app/Contents/MacOS/blender -b srcart/forest.blend -P script/export_scene.py -- ld33_export.cfg

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
	print ("headerSize", struct.calcsize('<4sL') )

	uv_layer = mesh.uv_layers['UVMap'].data[:]		
	# print(uv_layer)
	# print ("NUM UV", len(uv_layer))
	# print ("NUM VERT", len(mesh.vertices))
	stndx = 0;
	for face in mesh.polygons:
		verts = face.vertices[:]		

		# for now, only triangles
		assert(len(verts)==3)

		print(len(verts))
		for vndx in verts:
			v = mesh.vertices[vndx]
			uv = uv_layer[stndx]
			stndx += 1
			print(v.co, v.normal, uv.uv )

			# pack up the vert data
			packedVert = struct.pack( '<3f3f4f', 
				v.co[0], v.co[1], v.co[2],
				v.normal[0], v.normal[1], v.normal[2],
				uv.uv[0], uv.uv[1], 0.0, 0.0 );

			packedDataList.append(packedVert)

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


def exportScene( cfg ):
	meshes = {}

	for obj in bpy.data.objects:		
		if type(obj.data) == bpy_types.Mesh and not obj.data.name in meshes:
			meshes[obj.data.name] = obj.data
		
	print ("Exporting Meshes...")
	meshPath = cfg['Paths']['MeshPath']
	for name, mesh in meshes.items():

		meshExportName = os.path.join( meshPath, "MESH_" + name + ".dat" )
		exportMeshObj( mesh, meshExportName )

	for obj in bpy.data.objects:
		locStr = "%f,%f,%f" % tuple(obj.location)
		rotStr = "%f,%f,%f" % tuple(obj.rotation_euler)

		if type(obj.data) == bpy_types.Mesh:
			print ("MESH: ", obj.name, obj.data.name, locStr, rotStr )


def main():
	print ("LD exporter...")

	cfg = getConfig()
	if cfg is None:
		print ("ERROR: Missing config, export stopped")
		return

	exportScene(cfg)

	


if __name__=='__main__':
	main()
