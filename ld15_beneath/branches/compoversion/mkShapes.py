import os, sys, shutil, string
import xml.dom.minidom
import Image

SRCART = "srcart"
GAMEDATA = "gamedata"

class Rect( object ):
    def __init__( self, x, y, w, h ):
        self.x = x
        self.y = y
        self.w = w
        self.h = h

class Shape( object ):
    def __init__( self, name, filename ):
        self.name = name
        self.filename = filename

        # Load image
        self.img = Image.open( self.filename )
        print self.filename
        self.img.load()
        self.rect = Rect( 0, 0, self.img.size[0], self.img.size[1] )
        
        # flags
        self.destmap = None
        self.collide = False
        self.blend = False
        self.pattern = False
        self.relief = "none" # ( 'none', 'in', 'out' )


class PackNode:
    def __init__( self, r ):
        self.child = None
        self.rect = r
        self.shape = None
        
    def insert( self, shape ):
        
        # Are we a leaf?
        if self.child:
            # not a leaf -- try first child
            newNode = self.child[0].insert( shape )
            if newNode:
                return newNode

            # didn't fit, try the other child
            return self.child[1].insert( shape )
        else:
            # I'm a leaf (in the wind)
            
            # if we've already got a shape, fail
            if self.shape:
                return None
            
            # will the shape fit??
            if ( (shape.rect.w > self.rect.w) or
                 (shape.rect.h > self.rect.h) ):

                # nope
                return None
            
            # perfect fit?
            if (shape.rect.w == self.rect.w and
                shape.rect.h == self.rect.h ):

                self.shape = shape
                shape.rect = self.rect
                return self
            
            # Split ourself
            dw = self.rect.w - shape.rect.w
            dh = self.rect.h - shape.rect.h
                
            if (dw > dh):
                # Split horizontally
                r0 = Rect( self.rect.x, self.rect.y,
                           shape.rect.w, self.rect.h )
                r1 = Rect( self.rect.x + shape.rect.w, self.rect.y,
                           self.rect.w - shape.rect.w, self.rect.h )
            else:
                r0 = Rect( self.rect.x, self.rect.y,
                           self.rect.w, shape.rect.h )
                r1 = Rect( self.rect.x, self.rect.y + shape.rect.h,
                           self.rect.w, self.rect.h - shape.rect.h )

            # update shape rect
            #shape.rect.x = self.rect.x
            #shape.rect.y = self.rect.y

            self.child = ( PackNode( r0 ), PackNode( r1 ) )
            return self.child[0].insert( shape )
            

def packShapes( shapes, destmap ):
    packSize = 32
    packed = False
    while (not packed) and (packSize < 2048):
        print "Pack trying size ", packSize
        rootR = Rect( 0, 0, packSize, packSize )
        root = PackNode( rootR )

        packed = True
        for s in shapes:
            if not root.insert(s):
                # Fail, didn't fit
                packed = False
                break
            
        if packed:
            break

        packSize *= 2

    # did it work?
    if not packed:
        print "Couldn't fit all shapes into max size"
        sys.exit(1)

    # Now go back and copy the images over
    imgDest = Image.new( "RGBA", ( packSize, packSize ) )
    for s in shapes:
        print s.rect
        imgDest.paste( s.img, ( s.rect.x, s.rect.y ) )
        
    print "Saving ", destmap
    imgDest.save( destmap )
            

def makeTileset( setName, tilePath ):

    print "--- Tileset : ", setName, " ---"

    # These are the different maps which end up 
    # in gamedata
    blendMap = setName + "_blend.png"  
    shapeMap = setName + "_shapes.png" 

    shapes = []

    # look for tiles in this tileset
    for f in os.listdir( tilePath ):

        if os.path.splitext( f )[1] == '.png':
            name, flags = string.split( f, '_' )
            s = Shape( name, os.path.join( tilePath, f) )
            if 'C' in flags:
                s.collide = True
            if 'B' in flags:
                s.blend = True
                s.destmap = blendMap
            if 'P' in flags:
                s.pattern = True
                s.destmap = f
            if 'I' in flags:
                s.relief = 'in'
            elif 'O' in flags:
                s.relief = 'out'
                
            if not s.destmap:
                s.destmap = shapeMap
            
            shapes.append( s )

    # sort the tiles by destmap
    destmaps = {}
    for s in shapes:
        destmaps[ s.destmap ] = destmaps.get( s.destmap, [] ) + [s]

    for d in destmaps.keys():

        print d, len(destmaps[d]), "shapes in map."
        
        # if there's only one map, copy it over
        if (len(destmaps[d])==1):
            s = destmaps[d][0]
            srcFile = s.filename
            destFile = os.path.join( GAMEDATA, setName+"_"+s.name+".png" )
            print srcFile, " --> ", destFile
            #shutil.copyfile( s.filename, os.path.join( GAMEDATA, setName+"_"+s.fi
        # Pack all the maps into gamedata        
        packShapes( destmaps[d], os.path.join( GAMEDATA, d ) )


    # Dump tileset info to xml
    doc = xml.dom.minidom.Document()
    shapeList = doc.createElement( "ShapeList" )
    doc.appendChild( shapeList )

    for s in shapes:
        sElem = doc.createElement( "Shape" )
        sElem.setAttribute( "name", s.name )
        sElem.setAttribute( "map", s.destmap )
        sElem.setAttribute( "blend", str(s.blend) )
        sElem.setAttribute( "pattern", str(s.pattern) )
        sElem.setAttribute( "relief", s.relief )
        sElem.setAttribute( "collide", str( s.collide ) )
        sElem.setAttribute( "rect", ( "%d,%d,%d,%d" % 
                                      (s.rect.x, s.rect.y,
                                       s.rect.w, s.rect.h ) ) )

        shapeList.appendChild( sElem )

    fp = open( os.path.join( GAMEDATA, ("shapes_%s.xml"  % setName) ), "wt" );
    doc.writexml( fp, indent="    ", addindent="  ", 
                  newl="\n", encoding="UTF-8" )
    fp.close()
    
    

if __name__=='__main__':
    
    # look for tilesets
    for f in os.listdir( SRCART ):
        tileSet = os.path.join( SRCART, f )
        if ( f.startswith( "shapes_" ) and 
             os.path.isdir( tileSet )):
            
            makeTileset( f[7:], tileSet )
