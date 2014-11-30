import os, sys, string
import argparse

# For example:
# python obj2sk.py -i ../assets/cube_bevel.obj -n CubeBevel > ../src/CubeBevelGeom.hx

__doc__="""
obj2sk -- Converts a .obj mesh into snowkit code, since the mesh 
loader is kind of wonky when using web. This might even be better 
for small demos and stuff, but it's not a very good idea for large
projects.
"""

CODE_PREFACE = """
import luxe.Vector;
import luxe.Color;

import phoenix.Batcher;
import phoenix.Texture;
import phoenix.geometry.Geometry;
import phoenix.geometry.Vertex;
import phoenix.geometry.TextureCoord;

class %(name)sGeom 
{
  public static function makeGeometry( texture:Texture, batcher:Batcher ) : Geometry
  {
     var _%(name)sVertData : Array<Float> = [
       // Data layout for each line below is:
       // posx, posy, posz, sts, stt, nrmx, nrmy, nrmz		    
"""

CODE_BODY = """
      var geometry = new Geometry({
            texture : texture,
            primitive_type: PrimitiveType.triangles,
            immediate : false,
            batcher : batcher,
            depth : 1 //:todo : optionise
        });

		for (i in 0...%(vert)d)
		{
			var vp = new Vector( _%(name)sVertData[i*8 + 0],
                                 _%(name)sVertData[i*8 + 1],
                                 _%(name)sVertData[i*8 + 2]);
			var vn = new Vector( _%(name)sVertData[i*8 + 5],
                                 _%(name)sVertData[i*8 + 6],
                                 _%(name)sVertData[i*8 + 7]);

			var _v = new Vertex(vp, new Color(), vn );

        	_v.uv.uv0.set_uv( _%(name)sVertData[i*8 + 3],
        					  _%(name)sVertData[i*8 + 4] );

        	geometry.add( _v );
		}

        return geometry;
	}
}
"""

# A group of faces which share a material
class MtlGroup:
	def __init__( self, name="unnamed"):
		self.name=name
		self.triverts = []

	def addVert( self, pos, st, nrm ):
		self.triverts.append( (pos, st, nrm) )

class ObjConverter:

	def __init__( self, objfile, join ):
		self.verts = []

		# I use four-component STs because sometimes I store two sets of texture coordinates in the sts
		self.sts = []
		self.nrms = []

		self.mtlgroups = {}

		# use basename as the default name
		self.name = os.path.splitext( os.path.basename( objfile ))[0]

		self.joinMode = join

		self.loadObj( objfile )

	def findMtl( self, name ):

		if self.mtlgroups.has_key( name ):
			return self.mtlgroups[name]
		else:
			mtl = MtlGroup( name )
			self.mtlgroups[name] = mtl

			return mtl

	def _parseFace( self, flist, mtlGroup ):

		# Ignore points and edges
		if (len(flist)<3):
			return

		face = []

		for fvert in flist:
			slashCount = fvert.count( '/')

			pndx = 0
			stndx = 0
			nrmndx = 0

			if slashCount==0:
				# just pos
				pndx = int(fvert)
				stndx = 1
				nrmndx = 1
			elif slashCount==1:
				# pos/st
				p, st = string.split( fvert, '/')
				pndx = int(p)
				stndx = int(st)
				nrmndx = 1
			elif slashCount==2:
				
				# pos//nrm or pos/st/nrm
				p, st, nrm = string.split( fvert, '/')
				pndx = int(p)
				if st=='':
					stndx == 1
				else:
					stndx = int(st)
				nrmndx = int(nrm)

			# decrement the indices because OBJ is 1-based
			pndx -= 1
			stndx -= 1
			nrmndx -= 1

			# Add to our local face
			face.append( (pndx, stndx, nrmndx) )

		# Add dummy sts  and norms if none present
		if len(self.sts)==0:
			self.sts.append( (0.0, 0.0, 0.0, 0.0))
	
		if len(self.nrms)==0:
			self.nrms.append( (0.0, 1.0, 0.0))			

		# split face into a triangle fan		
		for bndx in range(1, len(face)-1):
			# triangle
			a = face[0]
			b = face[bndx]
			c = face[bndx+1]
			
			mtlGroup.addVert( self.verts[ a[0] ], self.sts[a[1]], self.nrms[a[2]] )
			mtlGroup.addVert( self.verts[ b[0] ], self.sts[b[1]], self.nrms[b[2]] )
			mtlGroup.addVert( self.verts[ c[0] ], self.sts[c[1]], self.nrms[c[2]] )




	def loadObj( self, objfile ):

		currMtl = None
		for line in open( objfile ):

			# Skip comments and whitespace
			line = string.strip(line)
			if line[0]=='#' or len(line)==0:
				continue

			lsplit = string.split(line," ")
			tok = lsplit[0]
			if tok == "o":
				# ObjName
				self.name = lsplit[1]
			elif tok == "v":
				# v <x> <y> <z> -- vertex
				vert = (lsplit[1], lsplit[2], lsplit[3])
				self.verts.append( vert )
			elif tok == "vt":
				# vt <s> <t> -- texture coord
				st = (lsplit[1], lsplit[2], "0.0", "0.0")
				self.sts.append( st )
			elif tok == "vn":
				# vn <x> <y> <z> -- vertex normal
				nrm = (lsplit[1], lsplit[2], lsplit[3])
				self.nrms.append( nrm )
			elif tok == "f":
				# face 
				if not currMtl:
					currMtl = self.findMtl('default')

				self._parseFace( lsplit[1:], currMtl )
			elif tok == 'usemtl':
				if not self.joinMode:
					mtl = lsplit[1]
					currMtl = self.findMtl( mtl )
			elif tok in ['mtllib', 's']:
				# don't care about these
				# 's' smoothing groups, ignore
				pass
			else:
				print "Unknown token in OBJ file", tok, ", skipping..."

	def writeSKCode( self ):

		# TODO: output multiple geoms for mutliple mtlgroups

		# Upcase for type name
		self.name = self.name[0].upper() + self.name[1:]
		codeInfo = { 'name' : self.name }

		print CODE_PREFACE % codeInfo

		mtlcount = 0
		totalVert = 0
		numMtls = len(self.mtlgroups.keys())
		for mtlname, mtl in self.mtlgroups.iteritems():
			print "\n      // "+mtlname
			count = 0
			mtlcount += 1
			for v in mtl.triverts:
				count += 1
				if ( (count==len(mtl.triverts)) and (mtlcount==numMtls) ):
					marker = " ];"
				else:
					marker = ","

				print "      %10s, %10s, %10s,   %10s, %10s,   %10s, %10s, %10s%s" % ( v[0][0], v[0][1], v[0][2],  # pos
																	  v[1][0], v[1][1],           # st
																	  v[2][0], v[2][1], v[2][2],  # nrm
																	  marker )
			totalVert += count

		codeInfo[ 'vert' ] = totalVert
		print CODE_BODY % codeInfo


if __name__=='__main__':

	parser = argparse.ArgumentParser( description="Convert an .obj mesh to snowkit Geometry code")
	parser.add_argument( '-i', '--infile', help="input .OBJ file" )
	parser.add_argument( '-n', '--name', help="name of class to create (otherwise taken from obj file)" )
	parser.add_argument( '-j', '--join', help="Join multiple materials into one Geometry", action='store_true')

	args = parser.parse_args()
	if args.infile:
		converter = ObjConverter( args.infile, args.join )		

		# Override name if given
		if (args.name):
			converter.name = args.name
		

		converter.writeSKCode()

