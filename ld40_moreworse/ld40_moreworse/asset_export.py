import os, sys
import string
import subprocess
import shutil

PROJECT_DIR = '/Users/joeld/oprojects/ld40_moreworse/'

ORYOL_EXPORT_TOOL = '/Users/joeld/Stuff/oryol-test/fips-deploy/oryol-tools/osx-xcode-debug/oryol-export'
ORYOL_EXPORT_CFG = 'test_config.toml'

CRUNCH_TOOL = '/Users/joeld/Toolkits/crunch-osx/bin_osx/crunch'

ASSET_DIR = "assets"
RUNTIME_DATA_DIR = "gamedata"

MESHES = [
	#'ground1.fbx',
	#'ground1_big.fbx',
	#'tree_062.fbx',
	#'tree_062_onemtl.fbx',
]

TEXTURES = {
	# 'wood.png' : 'A8R8G8B8'
	#'ground1.png' : 'DXT5',
	#'tree_062.png' : 'DXT5',
	#'soupbowl.png' : 'DXT5'
	'crate_DIF.png' : 'DXT5',
	'default.png' : 'DXT5',
	'floor_DIF.png' : 'DXT5',
	'robot_DIF.png' : 'DXT5',
	'AxisThing_DIF.png' : 'DXT5',
	'walls_DIF.png' : 'DXT5',
	'person_DIF.png' : 'DXT5',
	'decals.png' : 'DXT5', # TODO: Try A8L8
}

def fileNeedsUpdate( srcFile, destFile ):
	if not os.path.exists( destFile ):
		print "DEST", destFile, "does not exist"
		return True

	destmtime = os.path.getmtime( destFile )
	srcmtime = os.path.getmtime( srcFile )

	if srcmtime >= destmtime:
		return True

	# file is up to date
	return False

if __name__=='__main__':

	# TODO more targets varients
	# target = "web"

	# TODO: Export fbx meshes automatically from blender

	# -----------------------------------
	# Meshes
	# -----------------------------------
	meshConfig = os.path.join (PROJECT_DIR, ASSET_DIR, ORYOL_EXPORT_CFG )
	for mesh in MESHES:
		srcFile = os.path.join( PROJECT_DIR, ASSET_DIR, mesh )
		destFile = os.path.join( PROJECT_DIR, RUNTIME_DATA_DIR, os.path.splitext( mesh )[0] + ".omsh" )

		if fileNeedsUpdate( srcFile, destFile):
			cmd = [ ORYOL_EXPORT_TOOL, 
					'-config', meshConfig,
					'-model', srcFile,
					'-out', destFile ]
			print string.join(cmd, ' ' )
			subprocess.call( cmd )

	# -----------------------------------
	# Textures
	# -----------------------------------
	for tex, fmt in TEXTURES.iteritems():
		
		srcFile = os.path.join( PROJECT_DIR, ASSET_DIR, tex )
		destFile = os.path.join( PROJECT_DIR, RUNTIME_DATA_DIR, os.path.splitext( tex )[0] + ".dds"  )		
		if fileNeedsUpdate(srcFile, destFile ):
			cmd = [ CRUNCH_TOOL,
					'-file', srcFile,
					'-out', destFile,
					'-gamma', '1.0',
					'-' + fmt
					]

			print string.join(cmd, ' ' )
			subprocess.call( cmd )

