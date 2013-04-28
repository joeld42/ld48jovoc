import bpy
import os

def dumpGroup( fpOut, grp ):

    fpOut.write( "    <group name='%s'>\n" % grp.name );
    for obj in grp.objects:
        print(obj.name)
        
        #print(obj.location)
        #print(obj.rotation_euler)
        
        objName = obj.name
        if (objName.rfind( ".") != -1):
            objName = objName[:objName.rfind(".")]
            
        locStr = "%f,%f,%f" % tuple(obj.location)
        rotStr = "%f,%f,%f" % tuple(obj.rotation_euler)
        print(rotStr)
        
        fpOut.write( "        <object name='%s' pos='%s' rot='%s'/>\n" % (objName, locStr, rotStr) );
        
    fpOut.write( "   </group>\n" );

def dumpScene( groupNames ):   
    
    print ("----------");
    
    fpOut = open( os.path.expanduser("~/stuff/export.xml"), "w")
    fpOut.write("<sceneObjs>\n")

    for grp in bpy.data.groups:
        if grp.name in groupNames:
            dumpGroup( fpOut, grp )
            
            
    fpOut.write("</sceneObjs>\n\n")        

dumpScene( ['testLevel'] )