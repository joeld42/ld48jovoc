
import luxe.Vector;
import luxe.Color;

import phoenix.Batcher;
import phoenix.Texture;
import phoenix.geometry.Geometry;
import phoenix.geometry.Vertex;
import phoenix.geometry.TextureCoord;

class CubeBevelGeom 
{
  public static function makeGeometry( texture:Texture, batcher:Batcher ) : Geometry
  {
     var _CubeBevelVertData : Array<Float> = [
       // Data layout for each line below is:
       // posx, posy, posz, sts, stt, nrmx, nrmy, nrmz		    


      // default
        0.952000,  -0.952000,  -1.000000,    -0.024000,  -0.976000,     0.000000,   0.000000,  -1.000000,
       -0.952000,  -0.952000,  -1.000000,    -0.976000,  -0.976000,     0.000000,   0.000000,  -1.000000,
       -0.952000,   0.952000,  -1.000000,    -0.976000,  -0.024000,     0.000000,   0.000000,  -1.000000,
        0.951999,   0.952000,   1.000001,    -0.024000,   0.976000,    -0.000000,   0.000000,   1.000000,
       -0.952000,   0.952000,   1.000000,    -0.976000,   0.976000,    -0.000000,   0.000000,   1.000000,
       -0.952000,  -0.952000,   1.000000,    -0.976000,   0.024000,    -0.000000,   0.000000,   1.000000,
       -1.000000,   0.952000,   0.952000,     0.024000,   0.976000,    -1.000000,  -0.000000,  -0.000000,
       -1.000000,   0.952000,  -0.952000,     0.976000,   0.976000,    -1.000000,  -0.000000,  -0.000000,
       -1.000000,  -0.952000,  -0.952000,     0.976000,   0.024000,    -1.000000,  -0.000000,  -0.000000,
       -0.952000,   1.000000,  -0.952000,    -0.976000,   0.976000,     0.000000,   1.000000,   0.000000,
       -0.952000,   1.000000,   0.952000,    -0.976000,   0.024000,     0.000000,   1.000000,   0.000000,
        0.951999,   1.000000,   0.952001,    -0.024000,   0.024000,     0.000000,   1.000000,   0.000000,
        1.000000,   0.952000,  -0.951999,    -0.024000,   0.976000,     1.000000,   0.000000,   0.000000,
        0.999999,   0.952000,   0.952001,    -0.976000,   0.976000,     1.000000,   0.000000,   0.000000,
        1.000000,  -0.952000,   0.952000,    -0.976000,   0.024000,     1.000000,   0.000000,   0.000000,
        0.952000,  -0.952000,  -1.000000,    -0.024000,  -0.976000,     0.577300,  -0.577400,  -0.577400,
        1.000000,  -0.952000,  -0.952000,     0.000000,  -0.976000,     0.577300,  -0.577400,  -0.577400,
        0.952000,  -1.000000,  -0.952000,    -0.024000,  -1.000000,     0.577300,  -0.577400,  -0.577400,
        0.952000,  -1.000000,   0.952000,    -0.024000,  -0.976000,     0.577400,  -0.577400,   0.577400,
        1.000000,  -0.952000,   0.952000,     0.000000,  -0.976000,     0.577400,  -0.577400,   0.577400,
        0.952000,  -0.952000,   1.000000,    -0.024000,  -1.000000,     0.577400,  -0.577400,   0.577400,
       -0.952000,  -1.000000,   0.952000,    -0.976000,  -0.976000,    -0.577300,  -0.577300,   0.577400,
       -0.952000,  -0.952000,   1.000000,    -0.976000,  -1.000000,    -0.577300,  -0.577300,   0.577400,
       -1.000000,  -0.952000,   0.952000,    -1.000000,  -0.976000,    -0.577300,  -0.577300,   0.577400,
       -1.000000,  -0.952000,  -0.952000,     0.976000,   0.024000,    -0.577300,  -0.577300,  -0.577400,
       -0.952000,  -0.952000,  -1.000000,     1.000000,   0.024000,    -0.577300,  -0.577300,  -0.577400,
       -0.952000,  -1.000000,  -0.952000,     0.976000,   0.000000,    -0.577300,  -0.577300,  -0.577400,
        1.000000,   0.952000,  -0.951999,    -0.024000,   0.976000,     0.577400,   0.577400,  -0.577400,
        0.952000,   0.952000,  -0.999999,    -0.000000,   0.976000,     0.577400,   0.577400,  -0.577400,
        0.952000,   1.000000,  -0.951999,    -0.024000,   1.000000,     0.577400,   0.577400,  -0.577400,
        0.951999,   1.000000,   0.952001,    -0.024000,   0.024000,     0.577300,   0.577300,   0.577400,
        0.951999,   0.952000,   1.000001,    -0.024000,  -0.000000,     0.577300,   0.577300,   0.577400,
        0.999999,   0.952000,   0.952001,    -0.000000,   0.024000,     0.577300,   0.577300,   0.577400,
       -0.952000,   1.000000,   0.952000,    -0.976000,   0.024000,    -0.577300,   0.577300,   0.577400,
       -1.000000,   0.952000,   0.952000,    -1.000000,   0.024000,    -0.577300,   0.577300,   0.577400,
       -0.952000,   0.952000,   1.000000,    -0.976000,   0.000000,    -0.577300,   0.577300,   0.577400,
       -0.952000,   0.952000,  -1.000000,    -0.976000,  -0.024000,    -0.577400,   0.577300,  -0.577400,
       -1.000000,   0.952000,  -0.952000,    -1.000000,  -0.024000,    -0.577400,   0.577300,  -0.577400,
       -0.952000,   1.000000,  -0.952000,    -0.976000,   0.000000,    -0.577400,   0.577300,  -0.577400,
        1.000000,  -0.952000,   0.952000,    -0.976000,   0.024000,     0.707100,  -0.707100,   0.000000,
        0.952000,  -1.000000,   0.952000,    -0.976000,   0.000000,     0.707100,  -0.707100,   0.000000,
        0.952000,  -1.000000,  -0.952000,    -0.024000,  -0.000000,     0.707100,  -0.707100,   0.000000,
       -0.952000,  -1.000000,  -0.952000,    -0.976000,  -0.024000,     0.000000,  -0.707100,  -0.707100,
       -0.952000,  -0.952000,  -1.000000,    -0.976000,   0.000000,     0.000000,  -0.707100,  -0.707100,
        0.952000,  -0.952000,  -1.000000,    -0.024000,  -0.000000,     0.000000,  -0.707100,  -0.707100,
        0.952000,   0.952000,  -0.999999,    -0.024000,  -0.024000,     0.707100,   0.000000,  -0.707100,
        1.000000,   0.952000,  -0.951999,     0.000000,  -0.024000,     0.707100,   0.000000,  -0.707100,
        1.000000,  -0.952000,  -0.952000,     0.000000,  -0.976000,     0.707100,   0.000000,  -0.707100,
       -0.952000,  -0.952000,   1.000000,    -0.976000,   0.024000,    -0.000000,  -0.707100,   0.707100,
       -0.952000,  -1.000000,   0.952000,    -0.976000,   0.000000,    -0.000000,  -0.707100,   0.707100,
        0.952000,  -1.000000,   0.952000,    -0.024000,  -0.000000,    -0.000000,  -0.707100,   0.707100,
        0.999999,   0.952000,   0.952001,    -0.976000,   0.976000,     0.707100,   0.000000,   0.707100,
        0.951999,   0.952000,   1.000001,    -1.000000,   0.976000,     0.707100,   0.000000,   0.707100,
        0.952000,  -0.952000,   1.000000,    -1.000000,   0.024000,     0.707100,   0.000000,   0.707100,
       -1.000000,  -0.952000,   0.952000,     0.024000,   0.024000,    -0.707100,  -0.707100,  -0.000000,
       -1.000000,  -0.952000,  -0.952000,     0.976000,   0.024000,    -0.707100,  -0.707100,  -0.000000,
       -0.952000,  -1.000000,  -0.952000,     0.976000,   0.000000,    -0.707100,  -0.707100,  -0.000000,
       -0.952000,   0.952000,   1.000000,    -0.976000,   0.976000,    -0.707100,   0.000000,   0.707100,
       -1.000000,   0.952000,   0.952000,    -1.000000,   0.976000,    -0.707100,   0.000000,   0.707100,
       -1.000000,  -0.952000,   0.952000,    -1.000000,   0.024000,    -0.707100,   0.000000,   0.707100,
       -1.000000,   0.952000,  -0.952000,     0.976000,   0.976000,    -0.707100,   0.000000,  -0.707100,
       -0.952000,   0.952000,  -1.000000,     1.000000,   0.976000,    -0.707100,   0.000000,  -0.707100,
       -0.952000,  -0.952000,  -1.000000,     1.000000,   0.024000,    -0.707100,   0.000000,  -0.707100,
        0.951999,   1.000000,   0.952001,    -0.024000,   0.024000,     0.707100,   0.707100,   0.000000,
        0.999999,   0.952000,   0.952001,    -0.000000,   0.024000,     0.707100,   0.707100,   0.000000,
        1.000000,   0.952000,  -0.951999,    -0.000000,   0.976000,     0.707100,   0.707100,   0.000000,
        0.952000,   0.952000,  -0.999999,    -0.024000,  -0.024000,     0.000000,   0.707100,  -0.707100,
       -0.952000,   0.952000,  -1.000000,    -0.976000,  -0.024000,     0.000000,   0.707100,  -0.707100,
       -0.952000,   1.000000,  -0.952000,    -0.976000,   0.000000,     0.000000,   0.707100,  -0.707100,
       -0.952000,   1.000000,   0.952000,    -0.976000,   0.024000,    -0.000000,   0.707100,   0.707100,
       -0.952000,   0.952000,   1.000000,    -0.976000,   0.000000,    -0.000000,   0.707100,   0.707100,
        0.951999,   0.952000,   1.000001,    -0.024000,  -0.000000,    -0.000000,   0.707100,   0.707100,
       -0.952000,   1.000000,  -0.952000,    -0.976000,   0.976000,    -0.707100,   0.707100,  -0.000000,
       -1.000000,   0.952000,  -0.952000,    -1.000000,   0.976000,    -0.707100,   0.707100,  -0.000000,
       -1.000000,   0.952000,   0.952000,    -1.000000,   0.024000,    -0.707100,   0.707100,  -0.000000,
        0.952000,  -1.000000,   0.952000,    -0.024000,  -0.976000,     0.000000,  -1.000000,   0.000000,
       -0.952000,  -1.000000,   0.952000,    -0.976000,  -0.976000,     0.000000,  -1.000000,   0.000000,
       -0.952000,  -1.000000,  -0.952000,    -0.976000,  -0.024000,     0.000000,  -1.000000,   0.000000,
        0.952000,   0.952000,  -0.999999,    -0.024000,  -0.024000,     0.000000,   0.000000,  -1.000000,
        0.952000,  -0.952000,  -1.000000,    -0.024000,  -0.976000,     0.000000,   0.000000,  -1.000000,
       -0.952000,   0.952000,  -1.000000,    -0.976000,  -0.024000,     0.000000,   0.000000,  -1.000000,
        0.952000,  -0.952000,   1.000000,    -0.024000,   0.024000,    -0.000000,   0.000000,   1.000000,
        0.951999,   0.952000,   1.000001,    -0.024000,   0.976000,    -0.000000,   0.000000,   1.000000,
       -0.952000,  -0.952000,   1.000000,    -0.976000,   0.024000,    -0.000000,   0.000000,   1.000000,
       -1.000000,  -0.952000,   0.952000,     0.024000,   0.024000,    -1.000000,  -0.000000,  -0.000000,
       -1.000000,   0.952000,   0.952000,     0.024000,   0.976000,    -1.000000,  -0.000000,  -0.000000,
       -1.000000,  -0.952000,  -0.952000,     0.976000,   0.024000,    -1.000000,  -0.000000,  -0.000000,
        0.952000,   1.000000,  -0.951999,    -0.024000,   0.976000,     0.000000,   1.000000,   0.000000,
       -0.952000,   1.000000,  -0.952000,    -0.976000,   0.976000,     0.000000,   1.000000,   0.000000,
        0.951999,   1.000000,   0.952001,    -0.024000,   0.024000,     0.000000,   1.000000,   0.000000,
        1.000000,  -0.952000,  -0.952000,    -0.024000,   0.024000,     1.000000,   0.000000,   0.000000,
        1.000000,   0.952000,  -0.951999,    -0.024000,   0.976000,     1.000000,   0.000000,   0.000000,
        1.000000,  -0.952000,   0.952000,    -0.976000,   0.024000,     1.000000,   0.000000,   0.000000,
        1.000000,  -0.952000,  -0.952000,    -0.024000,   0.024000,     0.707100,  -0.707100,   0.000000,
        1.000000,  -0.952000,   0.952000,    -0.976000,   0.024000,     0.707100,  -0.707100,   0.000000,
        0.952000,  -1.000000,  -0.952000,    -0.024000,  -0.000000,     0.707100,  -0.707100,   0.000000,
        0.952000,  -1.000000,  -0.952000,    -0.024000,  -0.024000,     0.000000,  -0.707100,  -0.707100,
       -0.952000,  -1.000000,  -0.952000,    -0.976000,  -0.024000,     0.000000,  -0.707100,  -0.707100,
        0.952000,  -0.952000,  -1.000000,    -0.024000,  -0.000000,     0.000000,  -0.707100,  -0.707100,
        0.952000,  -0.952000,  -1.000000,    -0.024000,  -0.976000,     0.707100,   0.000000,  -0.707100,
        0.952000,   0.952000,  -0.999999,    -0.024000,  -0.024000,     0.707100,   0.000000,  -0.707100,
        1.000000,  -0.952000,  -0.952000,     0.000000,  -0.976000,     0.707100,   0.000000,  -0.707100,
        0.952000,  -0.952000,   1.000000,    -0.024000,   0.024000,    -0.000000,  -0.707100,   0.707100,
       -0.952000,  -0.952000,   1.000000,    -0.976000,   0.024000,    -0.000000,  -0.707100,   0.707100,
        0.952000,  -1.000000,   0.952000,    -0.024000,  -0.000000,    -0.000000,  -0.707100,   0.707100,
        1.000000,  -0.952000,   0.952000,    -0.976000,   0.024000,     0.707100,   0.000000,   0.707100,
        0.999999,   0.952000,   0.952001,    -0.976000,   0.976000,     0.707100,   0.000000,   0.707100,
        0.952000,  -0.952000,   1.000000,    -1.000000,   0.024000,     0.707100,   0.000000,   0.707100,
       -0.952000,  -1.000000,   0.952000,     0.024000,   0.000000,    -0.707100,  -0.707100,  -0.000000,
       -1.000000,  -0.952000,   0.952000,     0.024000,   0.024000,    -0.707100,  -0.707100,  -0.000000,
       -0.952000,  -1.000000,  -0.952000,     0.976000,   0.000000,    -0.707100,  -0.707100,  -0.000000,
       -0.952000,  -0.952000,   1.000000,    -0.976000,   0.024000,    -0.707100,   0.000000,   0.707100,
       -0.952000,   0.952000,   1.000000,    -0.976000,   0.976000,    -0.707100,   0.000000,   0.707100,
       -1.000000,  -0.952000,   0.952000,    -1.000000,   0.024000,    -0.707100,   0.000000,   0.707100,
       -1.000000,  -0.952000,  -0.952000,     0.976000,   0.024000,    -0.707100,   0.000000,  -0.707100,
       -1.000000,   0.952000,  -0.952000,     0.976000,   0.976000,    -0.707100,   0.000000,  -0.707100,
       -0.952000,  -0.952000,  -1.000000,     1.000000,   0.024000,    -0.707100,   0.000000,  -0.707100,
        0.952000,   1.000000,  -0.951999,    -0.024000,   0.976000,     0.707100,   0.707100,   0.000000,
        0.951999,   1.000000,   0.952001,    -0.024000,   0.024000,     0.707100,   0.707100,   0.000000,
        1.000000,   0.952000,  -0.951999,    -0.000000,   0.976000,     0.707100,   0.707100,   0.000000,
        0.952000,   1.000000,  -0.951999,    -0.024000,  -0.000000,     0.000000,   0.707100,  -0.707100,
        0.952000,   0.952000,  -0.999999,    -0.024000,  -0.024000,     0.000000,   0.707100,  -0.707100,
       -0.952000,   1.000000,  -0.952000,    -0.976000,   0.000000,     0.000000,   0.707100,  -0.707100,
        0.951999,   1.000000,   0.952001,    -0.024000,   0.024000,    -0.000000,   0.707100,   0.707100,
       -0.952000,   1.000000,   0.952000,    -0.976000,   0.024000,    -0.000000,   0.707100,   0.707100,
        0.951999,   0.952000,   1.000001,    -0.024000,  -0.000000,    -0.000000,   0.707100,   0.707100,
       -0.952000,   1.000000,   0.952000,    -0.976000,   0.024000,    -0.707100,   0.707100,  -0.000000,
       -0.952000,   1.000000,  -0.952000,    -0.976000,   0.976000,    -0.707100,   0.707100,  -0.000000,
       -1.000000,   0.952000,   0.952000,    -1.000000,   0.024000,    -0.707100,   0.707100,  -0.000000,
        0.952000,  -1.000000,  -0.952000,    -0.024000,  -0.024000,     0.000000,  -1.000000,   0.000000,
        0.952000,  -1.000000,   0.952000,    -0.024000,  -0.976000,     0.000000,  -1.000000,   0.000000,
       -0.952000,  -1.000000,  -0.952000,    -0.976000,  -0.024000,     0.000000,  -1.000000,   0.000000 ];

      var geometry = new Geometry({
            texture : texture,
            primitive_type: PrimitiveType.triangles,
            immediate : false,
            batcher : batcher,
            depth : 1 //:todo : optionise
        });

		for (i in 0...132)
		{
			var vp = new Vector( _CubeBevelVertData[i*8 + 0],
                                 _CubeBevelVertData[i*8 + 1],
                                 _CubeBevelVertData[i*8 + 2]);
			var vn = new Vector( _CubeBevelVertData[i*8 + 5],
                                 _CubeBevelVertData[i*8 + 6],
                                 _CubeBevelVertData[i*8 + 7]);

			var _v = new Vertex(vp, new Color(), vn );

        	_v.uv.uv0.set_uv( _CubeBevelVertData[i*8 + 3],
        					  _CubeBevelVertData[i*8 + 4] );

        	geometry.add( _v );
		}

        return geometry;
	}
}

