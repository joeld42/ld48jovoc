import luxe.Input;
import luxe.Mesh;
import luxe.Color;
import luxe.Vector;

import phoenix.geometry.Geometry;
import phoenix.geometry.Vertex;
import phoenix.geometry.TextureCoord;
import phoenix.Batcher;
import phoenix.Texture;
import phoenix.Quaternion;

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

	public function loadScene( sceneName : String )
	{
		var scene = Luxe.resources.json( sceneName );
		// trace( '${scene.asset.json}' );
		var sceneObjs : Array<Dynamic> = scene.asset.json;
		for (obj in sceneObjs)
		{			
			var objName = obj.name;
			
			// var meshName = obj["mesh"];
			// var meshLoc = obj["loc"];
			// var meshRot = obj["rot"]; 			
			var mesh = makeInstance( "assets/mesh/" + obj.mesh + ".dat", 
									 "assets/" + obj.texture );
			mesh.pos.set_xyz( obj.loc[0], obj.loc[1], obj.loc[2] );

			// Do it this way to control rotation order
			// TODO: export rotation order from blender
			var xrot = new Quaternion();
			xrot.setFromEuler(new Vector( -obj.rot[0], 0.0, 0.0));

			var yrot = new Quaternion();
			yrot.setFromEuler(new Vector( 0.0, -obj.rot[2], 0.0));			

			var zrot = new Quaternion();
			zrot.setFromEuler(new Vector( 0.0, 0.0, -obj.rot[1] ));			
			
			//mesh.rotation.multiply( zrot );
			mesh.rotation.multiply( yrot );			
			mesh.rotation.multiply( xrot );
			

			mesh.scale.set_xyz( obj.scl[0],  obj.scl[1],  obj.scl[2] );

			mesh.geometry.locked = true;
		}
	}

	function loadGeometry( meshName : String ) : Geometry
	{
		var geom = new Geometry({
            batcher : null,
            immediate : false,
            primitive_type: PrimitiveType.triangles,
            });
		var meshData = Luxe.resources.bytes(meshName);
		trace( 'loaded bytes ${meshName} result ${meshData}');

    	//var meshData = Luxe.resources.bytes("assets/mesh/MESH_Cube.dat");
		var data = meshData.asset.bytes.toBytes();

		// unpack simple header
		var magic : String = data.getString( 0, 4 );
		var numTris : Int = data.get( 4 );
		// trace('Loaded MESH data: header ${magic} numTris ${numTris}');

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