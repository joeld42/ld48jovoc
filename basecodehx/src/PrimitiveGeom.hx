import luxe.Vector;
import luxe.Color;

import phoenix.Batcher;
import phoenix.Texture;
import phoenix.geometry.Geometry;
import phoenix.geometry.Vertex;
import phoenix.geometry.TextureCoord;

// Would be better to do this as part of phoenix.geometry.* but 
// this is just for testing for now.. 
class PrimitiveGeom 
{
	public static function makeCube( center:Vector, size:Float, texture:Texture, batcher:Batcher ) : Geometry
	{
		var _cubeVertData : Array<Float> = [
		    // Data layout for each line below is:
		    // positionX, positionY, positionZ,     normalX, normalY, normalZ,
		    0.5, -0.5, -0.5,        1.0, 0.0, 0.0,
		    0.5, 0.5, -0.5,         1.0, 0.0, 0.0,
		    0.5, -0.5, 0.5,         1.0, 0.0, 0.0,
		    0.5, -0.5, 0.5,         1.0, 0.0, 0.0,
		    0.5, 0.5, -0.5,         1.0, 0.0, 0.0,
		    0.5, 0.5, 0.5,          1.0, 0.0, 0.0,
		    
		    0.5, 0.5, -0.5,         0.0, 1.0, 0.0,
		    -0.5, 0.5, -0.5,        0.0, 1.0, 0.0,
		    0.5, 0.5, 0.5,          0.0, 1.0, 0.0,
		    0.5, 0.5, 0.5,          0.0, 1.0, 0.0,
		    -0.5, 0.5, -0.5,        0.0, 1.0, 0.0,
		    -0.5, 0.5, 0.5,         0.0, 1.0, 0.0,
		    
		    -0.5, 0.5, -0.5,        -1.0, 0.0, 0.0,
		    -0.5, -0.5, -0.5,       -1.0, 0.0, 0.0,
		    -0.5, 0.5, 0.5,         -1.0, 0.0, 0.0,
		    -0.5, 0.5, 0.5,         -1.0, 0.0, 0.0,
		    -0.5, -0.5, -0.5,       -1.0, 0.0, 0.0,
		    -0.5, -0.5, 0.5,        -1.0, 0.0, 0.0,
		    
		    -0.5, -0.5, -0.5,       0.0, -1.0, 0.0,
		    0.5, -0.5, -0.5,        0.0, -1.0, 0.0,
		    -0.5, -0.5, 0.5,        0.0, -1.0, 0.0,
		    -0.5, -0.5, 0.5,        0.0, -1.0, 0.0,
		    0.5, -0.5, -0.5,        0.0, -1.0, 0.0,
		    0.5, -0.5, 0.5,         0.0, -1.0, 0.0,
		    
		    0.5, 0.5, 0.5,          0.0, 0.0, 1.0,
		    -0.5, 0.5, 0.5,         0.0, 0.0, 1.0,
		    0.5, -0.5, 0.5,         0.0, 0.0, 1.0,
		    0.5, -0.5, 0.5,         0.0, 0.0, 1.0,
		    -0.5, 0.5, 0.5,         0.0, 0.0, 1.0,
		    -0.5, -0.5, 0.5,        0.0, 0.0, 1.0,
		    
		    0.5, -0.5, -0.5,        0.0, 0.0, -1.0,
		    -0.5, -0.5, -0.5,       0.0, 0.0, -1.0,
		    0.5, 0.5, -0.5,         0.0, 0.0, -1.0,
		    0.5, 0.5, -0.5,         0.0, 0.0, -1.0,
		    -0.5, -0.5, -0.5,       0.0, 0.0, -1.0,
		    -0.5, 0.5, -0.5,        0.0, 0.0, -1.0		
		];

		var geometry = new Geometry({
            texture : texture,
            primitive_type: PrimitiveType.triangles,
            immediate : false,
            batcher : batcher,
            depth : 1 //:todo : optionise
        });

		for (i in 0...36)
		{
			var vp = new Vector( (_cubeVertData[i*6 + 0] * size) - center.x,
                                 (_cubeVertData[i*6 + 1] * size) - center.y,
                                 (_cubeVertData[i*6 + 2] * size) - center.z );
			var vn = new Vector( (_cubeVertData[i*6 + 3] * size) - center.x,
                                 (_cubeVertData[i*6 + 4] * size) - center.y,
                                 (_cubeVertData[i*6 + 5] * size) - center.z );
			var sndx: Int;
			var tndx: Int;
			var faceNdx:Int = Math.floor(i/6);
			sndx = 0;
			tndx = 0;
        	switch ( faceNdx ) 
        	{
	            case 0: sndx = 1; tndx = 2;  // face 0, project yz
	            case 1: sndx = 0; tndx = 2;  // face 1, project xz
	            case 2: sndx = 1; tndx = 2;  // face 2, project yz
	            case 3: sndx = 0; tndx = 2;  // face 3, project xz
	            case 4: sndx = 0; tndx = 1;  // face 4, project xy
	            case 5: sndx = 0; tndx = 1;  // face 5, project xy
	        }
	        trace( "faceNdx "+faceNdx + " st " + sndx + " " + tndx );

			var _v = new Vertex(vp, new Color(), vn );

        	_v.uv.uv0.set_uv( _cubeVertData[i*6 + sndx] + 0.5,
                              _cubeVertData[i*6 + tndx] + 0.5 );

        	geometry.add( _v );
		}

        return geometry;
	}
}