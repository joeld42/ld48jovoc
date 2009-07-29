# ================================================
#    Toplevel SCons file for ld48jovoc
# ================================================
#
# To build everthing:
#   scons (from this directory)
#
# To build just a single game
#   scons -U  (from that game's directory)
# 
import os, sys

# ================================================
#    Base environment
# ================================================
buildDir = "build"
BuildDir( buildDir, '.', duplicate=0 )

baseEnv = Environment( CPPDEFINES="-DNDEBUG" )
Export( 'baseEnv' )

allTargets = list()

wander = SConscript( os.path.join( buildDir, "miniLD11_wander", "trunk", "SConscript" ) )
allTargets.append( wander )


Default( allTargets )
