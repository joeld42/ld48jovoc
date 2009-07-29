import os, sys
import subprocess

def generate( env, **kw ):
    
    env.ParseConfig( "allegro-config --cflags --libs" )

def exists( env ):
    retcode = subprocess.call( ["allegro-config", "--cflags"] )
    print "allegro exists ", retcode
    return retcode
