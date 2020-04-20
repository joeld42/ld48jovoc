import os, sys
import string
import subprocess
import shutil

# FIXME NOW: Make one gamedata directory per target platform

PROJECT_DIR = '/Users/joeld/oprojects/darkstar'

CRUNCH_TOOL = '/Users/joeld/Toolkits/crunch-osx/bin_osx/crunch'
TEXTURETOOL_TOOL = '/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/texturetool'

#BLENDER_TOOL = '/Applications/Blender/blender.app/Contents/MacOS/blender'
#BLENDER_EXPORT_SCRIPT = 'export_scene.py'
#-b srcart/forest.blend -P script/export_scene.py -- ld33_export.cfg

LDJAM_TOOL = '/Users/joeld/oprojects/fips-deploy/darkstar/osx-xcode-debug/ldjamtool'

#'./assets/TestLevelSF.ogex ./gamedata/TestLevelSF.ldjam'

#ASSET_DIR = "assets"
# TMP
ASSET_DIR = "assets"
OGEX_DIR = "assets"
RUNTIME_DATA_DIR = "gamedata"

TEXTURES = {

    'TestTile_basecolor.png' : 'DXT5',
    'TestTile_metallic.png' : 'DXT5',
    'TestTile_normal.png' : 'DXT5',
    'TestTile_roughness.png' : 'DXT5',
}

OGEX_FILES = {
    'TEST_Stuff.ogex',   
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
    print destFile[ len(PROJECT_DIR)+1: ], "up to date."
    return False

if __name__=='__main__':


    # -----------------------------------
    # OGEX Scene Files    
    # -----------------------------------
    # TODO: Extract list of textures automatically from scenes
    for ogex in OGEX_FILES:
        
        srcFile = os.path.join( PROJECT_DIR, OGEX_DIR, ogex )        
        destFile = os.path.join( PROJECT_DIR, RUNTIME_DATA_DIR, os.path.splitext( ogex )[0] + ".ldjam"  )

        if fileNeedsUpdate(srcFile, destFile ):
            cmd = [ LDJAM_TOOL,
                    srcFile,
                    destFile ]

            print string.join(cmd, ' ' )
            subprocess.call( cmd )

    # -----------------------------------
    # Textures
    # -----------------------------------
    for tex, fmt in TEXTURES.iteritems():

        srcFile = os.path.join( PROJECT_DIR, ASSET_DIR, tex )

        texfile = os.path.split( tex )[-1]
        destFile = os.path.join( PROJECT_DIR, RUNTIME_DATA_DIR, os.path.splitext( texfile )[0] + ".dds"  )
        if fileNeedsUpdate(srcFile, destFile ):
            cmd = [ CRUNCH_TOOL,
                    '-file', srcFile,
                    '-out', destFile,
                    '-gamma', '1.0',
                    '-' + fmt
                    ]

            print string.join(cmd, ' ' )
            subprocess.call( cmd )



        # # Also make pvr version for ios
        # destFilePVR = os.path.join( PROJECT_DIR, RUNTIME_DATA_DIR, os.path.splitext( tex )[0] + ".pvr"  )

        # # TODO: better params
        # cmd2 = [TEXTURETOOL_TOOL,
        #         '-m',  # Generate mipmap chain
        #         '-e', 'PVRTC',
        #         '-f', 'PVR',
        #         '--bits-per-pixel-4',
        #         '-o', destFilePVR,
        #         srcFile
        #         ]

        # if fileNeedsUpdate(srcFile, destFilePVR ):
        #     print string.join(cmd2, ' ' )
        #     subprocess.call( cmd2 )

