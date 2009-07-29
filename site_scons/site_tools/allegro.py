import os, sys
import subprocess

import SCons.Warnings
class AllegroToolWarning( SCons.Warnings.Warning ):
    pass
SCons.Warnings.enableWarningClass( AllegroToolWarning )

# allegro-config doesn't work on windows (unless you're
# using mingw or something). 
WINDOWS_ALLEGRO_LOCATION = r"C:\toolkits\allegro-4.2.0"

def generate( env, **kw ):
    if sys.platform=="win32":

        env.Tool('msvc')

        alh_location = os.path.join( WINDOWS_ALLEGRO_LOCATION, "include", "allegro.h" )
        if not os.path.exists( os.path.join( alh_location )):

            SCons.Warnings.warn( AllegroToolWarning,
                                 "[allegro] allegro.h was not found in " + WINDOWS_ALLEGRO_LOCATION +"\n"
                                 "======\nPlease edit the location of your allegro install in\n"+
                                 "site_scons/site_tools/allegro.py\n=====\n\n" )
        
        # Add expected allegro paths
        print "... appending env\n" ;
        env.AppendUnique( CPPPATH=[ os.path.join( WINDOWS_ALLEGRO_LOCATION, "include" )],
                          LIBPATH=[ os.path.join( WINDOWS_ALLEGRO_LOCATION, "lib" )],
                          LIBS = ['alleg'] )
        print "CPPPATH is ", env['CPPPATH']

    else:
        # On a real OS, this is easy
        env.ParseConfig( "allegro-config --cflags --libs" )

    return env

def exists( env ):
    # Why does exists() exist?
    retcode = subprocess.call( ["allegro-config", "--cflags"] )
    print "allegro exists ", retcode
    return retcode
