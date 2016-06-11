import os, sys
import string
import subprocess
import shutil

PROJECT_DIR = '/Users/joeld/Projects/ld48jovoc/simplejam_safetynet/simplejam2016_soup'

ORYOL_EXPORT_TOOL = '/Users/joeld/Stuff/oryol-test/fips-deploy/oryol-tools/osx-xcode-debug/oryol-export'
ORYOL_EXPORT_CFG = 'test_config.toml'

CRUNCH_TOOL = '/Users/joeld/Toolkits/crunch-osx/bin_osx/crunch'

ASSET_DIR = "assets"
RUNTIME_DATA_DIR = "gamedata"

MESHES = [
	'bowl.fbx',
	'table.fbx'
]

TEXTURES = {
	# 'wood.png' : 'A8R8G8B8'
	'wood.png' : 'DXT5',
	'soupbowl.png' : 'DXT5'
}

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

		# TODO: add timestamp checks
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
		cmd = [ CRUNCH_TOOL,
				'-file', srcFile,
				'-out', destFile,
				'-gamma', '1.0',
				'-' + fmt
				]

		print string.join(cmd, ' ' )
		subprocess.call( cmd )

