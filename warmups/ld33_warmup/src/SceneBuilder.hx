import luxe.Input;
import luxe.Mesh;
import luxe.Color;
import luxe.Vector;
import luxe.Camera;

import luxe.utils.Random;

import phoenix.geometry.Geometry;
import phoenix.geometry.Vertex;
import phoenix.geometry.TextureCoord;
import phoenix.Batcher;
import phoenix.Texture;
import phoenix.Quaternion;
import phoenix.Shader;
import phoenix.Transform;
import phoenix.Matrix;

import snow.api.buffers.Uint8Array;
import snow.modules.opengl.GL;

@:publicFields
class SceneObj {
	var name_: String;
	var tintColor : Vector;
	var xform_: Transform;
	
	public function new( name : String )
	{
		name_ = name;
		// var rand = Luxe.utils.random.get();
		tintColor = new Vector( Luxe.utils.random.float( 0.0, 1.0 ),
								Luxe.utils.random.float( 0.0, 1.0 ),
								Luxe.utils.random.float( 0.0, 1.0 ) );
	}
}

@:publicFields
class SceneMesh {
	var mesh_ : luxe.Mesh;
	var instList_ : Array<SceneObj>;

	function new( mesh : luxe.Mesh ) 
	{		
		mesh_ = mesh; // also contains geo
		instList_ = new Array<SceneObj>();
	}
}

class SceneBuilder
{
	var meshDB_ = new Map<String,SceneMesh>();
	public var testShader_ : Shader;
	public var sceneCamera_ : Camera;

	var testObj_ : SceneObj;

	public function loadScene( sceneName : String )
	{
		var scene = Luxe.resources.json( sceneName );	

		// trace( '${scene.asset.json}' );
		var sceneFileObjs : Array<Dynamic> = scene.asset.json;
		for (obj in sceneFileObjs)
		{			
			var objName = obj.name;
			
			var sceneObj = new SceneObj( objName);			
			sceneObj.name_ = objName;
			sceneObj.xform_ = new Transform();
			// var meshName = obj["mesh"];
			// var meshLoc = obj["loc"];
			// var meshRot = obj["rot"]; 			
			var sceneMesh = lookupSceneMesh( "assets/mesh/" + obj.mesh + ".dat", 
									 		 "assets/" + obj.texture );
			//sceneObj.xform_.makeTranslation( obj.loc[0], obj.loc[1], obj.loc[2] );
			

			// Do it this way to control rotation order
			// TODO: export rotation order from blender
			var xrot = new Quaternion();
			xrot.setFromEuler(new Vector( -obj.rot[0], 0.0, 0.0));

			var yrot = new Quaternion();
			yrot.setFromEuler(new Vector( 0.0, -obj.rot[2], 0.0));			

			var zrot = new Quaternion();
			zrot.setFromEuler(new Vector( 0.0, 0.0, -obj.rot[1] ));			
			
			var rot = new Quaternion();
			//rot.multiply( zrot );
			rot.multiply( yrot );			
			rot.multiply( xrot );			

			// var mrot = new Matrix();
			// mrot.makeRotationFromQuaternion( rot );
			//sceneObj.xform_.multiply( mrot );

			// var mscl = new Matrix();
			// mscl.makeScale(obj.scl[0],  obj.scl[1],  obj.scl[2] );
			//sceneObj.xform_.multiply( mscl );

			// sceneObj.xform_.local.scale.set_xyz( obj.scl[0],  obj.scl[1],  obj.scl[2] );
			sceneObj.xform_.pos.set_xyz( obj.loc[0], obj.loc[1], obj.loc[2] );
			sceneObj.xform_.scale.set_xyz(obj.scl[0],  obj.scl[1],  obj.scl[2] );
			sceneObj.xform_.rotation.copy( rot );

			sceneMesh.instList_.push( sceneObj );
			trace('MESH ${obj.mesh} has ${sceneMesh.instList_.length} matrix ${sceneObj.xform_}');
			// mesh.scale.set_xyz( obj.scl[0],  obj.scl[1],  obj.scl[2] );

			// mesh.geometry.locked = true;
		}


		testObj_ = findSceneObj( "Cylinder.003");
	}

	function lookupSceneMesh( meshID : String, textureName : String ) : SceneMesh
	{		
		// Do we already have a mesh list
		var sceneMesh : SceneMesh = meshDB_.get( meshID );
		if (sceneMesh == null) {
			// Need to load the src geo
			var meshGeo = loadGeometry( meshID );

			// Get the texture from Luxe resource system
			var tex = Luxe.resources.texture( textureName );

			var mesh = new luxe.Mesh({					
					no_batcher_add: true,
					buffer_based: true,
					object_space: true,
					geometry : meshGeo,
					texture : tex
				});		

			mesh.geometry.shader = testShader_;

			// add to the mesh db
			sceneMesh = new SceneMesh( mesh );
			meshDB_[meshID] = sceneMesh;
		}


		if (sceneMesh==null)
		{
			trace('ERR: could not load mesh ${meshID}');
			return null;
		}

		return sceneMesh;
	}

	function loadGeometry( meshName : String ) : Geometry
	{
		var geom = new Geometry({
            batcher : null,
            immediate : false,
            primitive_type: PrimitiveType.triangles,
            buffer_based: true,
            object_space: true,
            });
		var meshData = Luxe.resources.bytes(meshName);
		trace( 'loaded bytes ${meshName} result ${meshData}');

    	//var meshData = Luxe.resources.bytes("assets/mesh/MESH_Cube.dat");
		var data = meshData.asset.bytes.toBytes();

		// unpack simple header
		var magic : String = data.getString( 0, 4 );
		var numTris : Int = data.get( 4 );

		// for some reason my Haxe.io.bytes is missing getInt32.. 
		var A : Int = data.get( 4 );
		var B : Int = data.get( 5 );
		var C : Int = data.get( 6 );
		var D : Int = data.get( 7 );
		numTris = (D << 24) | (C << 16) | (B << 8) | A;
		//trace('ABCD ${A} ${B} ${C} ${D}  ');

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

	// TODO: Rearrange this
	public function drawScene()
	{
		GL.enable(GL.DEPTH_TEST);
		var modelView = new Matrix();
		var mvp = new Matrix();

		var normalMatrix = new Matrix();		

		//viewProj.multiplyMatrices( sceneCamera_.view.projection_matrix, sceneCamera_.view.view_matrix_inverse );
		// trace('view matrix is ${sceneCamera_.view.view_matrix}');

		for (sceneMesh in meshDB_)
		{
			var mesh : Mesh = sceneMesh.mesh_;
			mesh.geometry.texture.bind();

	        for(sceneObj in sceneMesh.instList_) 
	        {
	        	var model = sceneObj.xform_.world.matrix;

	        	modelView.multiplyMatrices( sceneCamera_.view.view_matrix_inverse, model  );
	        	mvp.multiplyMatrices( sceneCamera_.view.projection_matrix, modelView );

	        	normalMatrix.getInverse( model );
	        	//normalMatrix.copy( sceneObj.xform_ );
	        	normalMatrix.transpose();

	        	mesh.geometry.shader.set_vector3( "tintColor", sceneObj.tintColor );
	        	mesh.geometry.shader.set_matrix4( "mvp", mvp );
	        	mesh.geometry.shader.set_matrix4( "normalMatrix", normalMatrix );

	            Luxe.renderer.batcher.submit_geometry(mesh.geometry, model );
	        }
    	}
	}

	public function findSceneObj( name : String ) : SceneObj
	{
		for (sceneMesh in meshDB_)
		{
			var mesh : Mesh = sceneMesh.mesh_;
	        for(sceneObj in sceneMesh.instList_) 
	        {
	        	if (sceneObj.name_ == name) {
	        		return sceneObj;
	        	}
	        }
	    }
	    return null;
	}
	
	public function update( dt : Float )
	{		
		var q = new Quaternion();
		q.setFromAxisAngle( new Vector( 1.0, 0.0, 0.0), 1.2*dt );
		testObj_.xform_.rotation.multiply(q );

		// trace('rotation ${testObj_.xform_.rotation}');
	}

}