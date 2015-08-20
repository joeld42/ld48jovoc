import luxe.Input;
import luxe.Mesh;
import luxe.Color;
import luxe.Vector;
import luxe.Camera;
import luxe.Rectangle;
import luxe.utils.Random;

import phoenix.geometry.Geometry;
import phoenix.geometry.Vertex;
import phoenix.geometry.TextureCoord;
import phoenix.RenderTexture;
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

	var shadowExtent= 30.0;
	public var shadowCamera_ : Camera;	
	
	public var shadowTexture_ : RenderTexture;
	//public var texShadDepth_ : Texture;
	public var texShadDepth_ : TextureID;


	var testObj_ : SceneObj;

	public function loadScene( sceneName : String )
	{
		var scene = Luxe.resources.json( sceneName );	

		// trace( '${scene.asset.json}' );
		var sceneFileObjs : Array<Dynamic> = scene.asset.json;
		for (obj in sceneFileObjs)
		{			
			var objName = obj.name;
			
			var sceneObj = addSceneObj( objName, obj.mesh, obj.texture );			

			// Do it this way to control rotation order
			// TODO: export rotation order from blender
			// FIXME: use rotation order flags in Quat class..
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

			// sceneObj.xform_.local.scale.set_xyz( obj.scl[0],  obj.scl[1],  obj.scl[2] );
			sceneObj.xform_.pos.set_xyz( obj.loc[0], obj.loc[1], obj.loc[2] );
			sceneObj.xform_.scale.set_xyz(obj.scl[0],  obj.scl[1],  obj.scl[2] );
			sceneObj.xform_.rotation.copy( rot );

			// trace('MESH ${obj.mesh} has ${sceneMesh.instList_.length} matrix ${sceneObj.xform_}');
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

	

	public function initShadows()
	{
		var depthTextureExt = GL.getExtension("WEBKIT_WEBGL_depth_texture");
		trace('depthTextureExt ${depthTextureExt}');

		shadowCamera_ = new Camera({
				name : 'topcam',
				near: -100,
				far: 100,
				aspect: 1.0,
				projection: ProjectionType.ortho,				
			});

		
		shadowCamera_.view.viewport = new Rectangle( -shadowExtent, -shadowExtent,
													 shadowExtent, shadowExtent );
		shadowCamera_.pos.set_xyz(-shadowExtent/2.0,0,0);
    	shadowCamera_.rotation.setFromEuler( new Vector( -90.0, 0, 0).radians() );

		shadowTexture_ = new RenderTexture({ id:'rtt_shadCol', width:512, height:512 });

    	// texShadDepth_ = new Texture({ id:'rtt_shadDepth', width:512, height:512, 
    	// 									format:GL.DEPTH_COMPONENT, 
    	// 									//data_type: GL.UNSIGNED_INT 
    	// 									});    	
		texShadDepth_ = GL.createTexture();
		GL.bindTexture( GL.TEXTURE_2D, texShadDepth_ );
		GL.texParameteri( GL.TEXTURE_2D, GL.TEXTURE_MAG_FILTER, GL.NEAREST);
		GL.texParameteri( GL.TEXTURE_2D, GL.TEXTURE_MIN_FILTER, GL.NEAREST);
		GL.texParameteri( GL.TEXTURE_2D, GL.TEXTURE_WRAP_S, GL.CLAMP_TO_EDGE);
		GL.texParameteri(GL.TEXTURE_2D, GL.TEXTURE_WRAP_T, GL.CLAMP_TO_EDGE);
		GL.texImage2D(GL.TEXTURE_2D, 0, GL.DEPTH_COMPONENT, 512, 512, 0, 
			GL.DEPTH_COMPONENT, GL.UNSIGNED_SHORT, null);
	}

	// TODO: Rearrange this
	public function drawScene()
	{
		Luxe.renderer.target = shadowTexture_;		
		// gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.TEXTURE_2D, depthTexture, 0);
		GL.framebufferTexture2D( GL.FRAMEBUFFER, GL.DEPTH_ATTACHMENT, GL.TEXTURE_2D, 
		 						texShadDepth_, 0 );

		shadowCamera_.pos.set_xyz( sceneCamera_.pos.x - shadowExtent/2.0,
								   sceneCamera_.pos.y, sceneCamera_.pos.z );

	  	GL.clear( GL.COLOR_BUFFER_BIT | GL.DEPTH_BUFFER_BIT );
        GL.clearDepth(1.0);
		shadowCamera_.view.process();
		drawScenePass( shadowCamera_ );

		Luxe.renderer.target = null;
		drawScenePass( sceneCamera_ );

	}

	public function drawScenePass( camera : Camera )
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

	        	modelView.multiplyMatrices( camera.view.view_matrix_inverse, model  );
	        	mvp.multiplyMatrices( camera.view.projection_matrix, modelView );

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

	public function addSceneObj( name : String, meshID : String, texture : String ) : SceneObj
	{
			var sceneObj = new SceneObj(name);						
			sceneObj.xform_ = new Transform();

			var sceneMesh = lookupSceneMesh( "assets/mesh/" + meshID + ".dat", 
									 		 "assets/" + texture );

			sceneMesh.instList_.push( sceneObj );
			//trace('MESH ${obj.mesh} has ${sceneMesh.instList_.length} matrix ${sceneObj.xform_}');
			return sceneObj;
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