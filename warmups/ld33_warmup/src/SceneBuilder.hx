import luxe.Input;
import luxe.Mesh;
import luxe.Color;
import luxe.Vector;

import phoenix.geometry.Geometry;
import phoenix.geometry.Vertex;
import phoenix.geometry.TextureCoord;
import phoenix.Batcher;
import phoenix.Texture;

import snow.api.buffers.Uint8Array;

class SceneBuilder
{
	var meshDB_ = new Map<String,Geometry>();

	public function makeInstance( meshName : String, textureName : String ) : Mesh 
	{
		// Get the texture from Luxe resource system
		var tex = Luxe.resources.texture( textureName );

		// Do we already have geo loaded?
		var meshGeo : Geometry = meshDB_.get( meshName );
		if (meshGeo == null) {
			// Need to load the geo
			meshGeo = loadGeometry( meshName );			
			meshDB_[meshName] = meshGeo;
		}

		if (meshGeo==null)
		{
			trace('ERR: could not load mesh ${meshName}');
			return null;
		}

		// Make a copy of the geom
		var mesh = new Mesh({
            geometry : new Geometry({
            batcher : Luxe.renderer.batcher,
            immediate : false,
            primitive_type: PrimitiveType.triangles,
            texture: tex
            })
        });

        for(v in meshGeo.vertices) {
            mesh.geometry.add( v.clone() );
        }

        return mesh;
	}

	function loadGeometry( meshName : String ) : Geometry
	{
		var geom = new Geometry({
            batcher : null,
            immediate : false,
            primitive_type: PrimitiveType.triangles,
            });
		var meshData = Luxe.resources.bytes("assets/mesh/MESH_TreeOakMesh.dat");
    	//var meshData = Luxe.resources.bytes("assets/mesh/MESH_Cube.dat");
		var data = meshData.asset.bytes.toBytes();

		// unpack simple header
		var magic : String = data.getString( 0, 4 );
		var numTris : Int = data.get( 4 );
		trace('Loaded MESH data: header ${magic} numTris ${numTris}');

		var meshVerts : Array<Vertex>;		
		var headerOffs = 8; // size of file header
		var meshColor = new Color( 1.0, 1.0, 1.0 );
		for (i in 0...numTris*3)
		{
			var vertOffs = headerOffs + (10*4)*i;
			var pos = new Vector( data.getFloat( vertOffs+0 ), 
								  data.getFloat( vertOffs+4 ), 
								  data.getFloat( vertOffs+8 ) );
			var nrm = new Vector( data.getFloat( vertOffs+12 ), 
								  data.getFloat( vertOffs+16 ), 
								  data.getFloat( vertOffs+20 ) );
			var uv = new TextureCoord(  data.getFloat( vertOffs+24 ), 
								  data.getFloat( vertOffs+28 ), 
								  data.getFloat( vertOffs+32 ),
								  data.getFloat( vertOffs+36 ) );
			var uvset = new TextureCoordSet();
			uvset.uv0 = uv;

			var vtx = new Vertex( pos, meshColor );
			vtx.normal = nrm;
			vtx.uv = uvset;

			//trace('vtx ${i} is ${pos.x}, ${pos.y}, ${pos.z}');
			//trace('nrm ${i} is ${nrm.x}, ${nrm.y}, ${nrm.z}');
			//trace('uv ${i} is ${uv.u}, ${uv.v}, ${uv.w} ${uv.t}');

			geom.add( vtx );
		}

		return geom;
	}

}