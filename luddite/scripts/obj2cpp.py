import os, sys, string

verts = []
texcoords = []
norms = []

def vertData( s ):
    "converts v/st string into vert tuple"
    indices = map( int, string.split(s,'/'))
    pos = verts[indices[0]-1 ]
    st = None
    nrm = None
    
    if (len(indices)>1):
        st = texcoords[ indices[1]-1 ]
        
    if (len(indices)>2):
        nrm = norms[ indices[2]-1]
    
    return (pos, st, nrm)
    
    
# main
objfile = sys.argv[1]
cppfile = objfile.replace( '.obj', '.cpp' )

tris = []

# use file basename without path or ext
meshname = os.path.splitext( os.path.split( objfile )[-1])[0]
print "processing ",meshname


for line in open( sys.argv[1] ):
    line = string.strip( line )
    
    # Skip blank lines and comments
    if not len(line) or line[0]=='#':
        continue
        
    lsplit = string.split( line )
    if lsplit[0] == 'v':
        # vert
        verts.append( (lsplit[1], lsplit[2], lsplit[3] ))
    elif lsplit[0] == 'vt':
        # st coord
        texcoords.append( (lsplit[1], lsplit[2]) )
    elif lsplit[0] == 'n':
        # norm
        norms.append( (lsplit[1], lsplit[2], lsplit[3]) )    
    elif lsplit[0] == 'f':
        # face, make a triangle fan out of it
        vv = map( vertData, lsplit[1:])
        v0 = vv[0]
    
        for e in zip( vv[1:], vv[2:]):
            tris.append( (v0, e[0], e[1]) )
                     
    else:
        print line
        

# Output triangle data
print cppfile
fpOut = open( cppfile, "wt")

fpOut.write("""
// generated from %s

#include "mesh.h"

//#define MESH_%s_NUM_TRI (%d)

MeshVert %s_meshData[%d] = 
{
""" % ( objfile, meshname.upper(), len(tris), meshname, len(tris)*3) )

for tri in tris:
    
    for t in tri:
        pos, st, nrm = t
        if pos and st and nrm:
            fpOut.write( "    { %s, %s, %s,    %s, %s,   %s, %s, %s },\n" %
            ( pos[0], pos[1], pos[2],   st[0], st[1],   nrm[0], nrm[1], nrm[2]) )

        elif pos and st:
            fpOut.write( "    { %s, %s, %s,    %s, %s },\n" %
            ( pos[0], pos[1], pos[2],   st[0], st[1] ) )

        elif pos:    
            fpOut.write( "    { %s, %s, %s },\n" %
            ( pos[0], pos[1], pos[2] ) )
            
    fpOut.write( "\n")
                    
fpOut.write( "\n};\n" )
