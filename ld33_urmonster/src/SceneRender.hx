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

import SceneIntersect;


@:publicFields
class SceneObj {
	var name_: String;
	var tintColor : Vector;
	var xform_: Transform;
	var boundSphere_ : SceneBoundSphere; // WARN: local, just a link to mesh bounds
	var pickable_ : Bool;
	var jitter_ = 0.0;

	public function new( name : String )
	{
		name_ = name;
		// var rand = Luxe.utils.random.get();
		tintColor = new Vector( Luxe.utils.random.float( 0.0, 1.0 ),
								Luxe.utils.random.float( 0.0, 1.0 ),
								Luxe.utils.random.float( 0.0, 1.0 ) );
		tintColor = new Vector( 1.0, 1.0, 1.0 );
		jitter_ = 0.0;
	}

	public function intersectRayBoundSphere( ray : SceneRay )
	{
		var model = xform_.world.matrix;
		var modelInv = model.inverse();

		var localRay = new SceneRay( ray.origin_.clone().transform( modelInv ),
									 ray.dir_.clone().transformDirection( modelInv ) );

		// trace('ray is ${ray} localRay is ${localRay}');

		var result = boundSphere_.intersectRay( localRay );
		if (result.hit_)
		{
			result.hitPoint_.transform( model );
			result.hitNormal_.transformDirection( model );

		}
		return result;
	}
}

@:publicFields
class SceneMesh {
	var mesh_ : luxe.Mesh;
	var hugmesh_ : luxe.Mesh;

	var instList_ : Array<SceneObj>;
	var boundSphere_ : SceneBoundSphere;

	function new( mesh : luxe.Mesh ) 
	{		
		mesh_ = mesh; // also contains geo
		instList_ = new Array<SceneObj>();

		boundSphere_ = SceneBoundSphere.BoundsFromGeom( mesh_.geometry);

	}
}

class SceneRender
{
	var meshDB_ = new Map<String,SceneMesh>();
	public var worldShader_ : Shader;
	public var shadowShader_ : Shader;
	public var sceneCamera_ : Camera;

	var shadowExtent= 50.0;
	public var shadowProjMat_ : Matrix;
	public var shadowViewMat_ : Matrix;
	
	public var shadowTexture_ : RenderTexture;
	
	public var texShadDepth_ : TextureID;
	public var texShadDepthWrap : Texture;	
	public var groundMesh_ : SceneMesh;

	public var hugging_ : Bool = false;

	public var buildings_ : Array<SceneObj>;

	public function new()
	{
		buildings_ = new Array<SceneObj>();
	}

	public function loadScene( sceneName : String )
	{
		var scene = Luxe.resources.json( sceneName );			

		// trace( '${scene.asset.json}' );
		var sceneFileObjs : Array<Dynamic> = scene.asset.json;
		for (obj in sceneFileObjs)
		{			
			var objName = obj.name;

			// HACK - don't add the HugMesh inst
			if (objName == 'ZillaHug') {
				continue;
			}

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

			// Game logic crap
			var prefix = objName.substring(0, 5);
			trace('prefix: ${prefix}');
			if (objName.substring(0, 5)=='BLDG_') {
				buildings_.push( sceneObj );					
				trace('${buildings_.length} buildings');
			}
		}

		trace('loadScene done, have ${buildings_.length} ...');
		
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

			mesh.geometry.shader = worldShader_;			

			// add to the mesh db
			sceneMesh = new SceneMesh( mesh );
			meshDB_[meshID] = sceneMesh;

			// HACK : if this is the body, also load the hug mesh
			if (meshID == "assets/mesh/MESH_BodyMesh.dat")
			{
				trace("Loading HugMesh!");
				var meshHugGeo = loadGeometry( "assets/mesh/MESH_BodyHugMesh.dat" );
				var hugMesh = new luxe.Mesh({					
					no_batcher_add: true,
					buffer_based: true,
					object_space: true,
					geometry : meshHugGeo,
					texture : tex
				});		

				hugMesh.geometry.shader = worldShader_;
				sceneMesh.hugmesh_ = hugMesh;
			}

			trace('mesh ${meshID} bound ${sceneMesh.boundSphere_}');
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
								  data.getFloat( vertOffs+8 ));
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

			// if (meshName == "assets/mesh/MESH_TestSphereMesh.dat")
			// {
			// 	trace('vtx ${i} is ${pos.x}, ${pos.y}, ${pos.z} len ${pos.length}');
			// 	trace('nrm ${i} is ${nrm.x}, ${nrm.y}, ${nrm.z} len ${nrm.length}');
			// 	trace('uv ${i} is ${uv.u}, ${uv.v}, ${uv.w} ${uv.t}');
			// }

			geom.add( vtx );
		}

		return geom;
	}

	

	public function initShadows()
	{
		var depthTextureExt = GL.getExtension("WEBKIT_WEBGL_depth_texture");
		trace('depthTextureExt ${depthTextureExt}');

		// DBG : disable for now
		return;

		shadowProjMat_ = new Matrix();
		shadowProjMat_.makeOrthographic( -shadowExtent, shadowExtent, 
										 -shadowExtent, shadowExtent, 
										 20.0, -20.0 );
		

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
			GL.DEPTH_COMPONENT, GL.UNSIGNED_INT, null);
		
		shadowTexture_.bindBuffer();
		GL.framebufferTexture2D( GL.FRAMEBUFFER, GL.DEPTH_ATTACHMENT, GL.TEXTURE_2D, 
		 						texShadDepth_, 0 );
		shadowTexture_.unbindBuffer();

		texShadDepthWrap = new Texture({ id : "texShadDepthWrap", 
			width:512, height:512, texture: texShadDepth_ });
		texShadDepthWrap.slot = 1;
	}

	// TODO: Rearrange this
	public function drawScene()
	{
		//drawShadowPass();
		drawBeautyPass();
	}

	function drawShadowPass()
	{
		// ==== shadow pass =======

		shadowTexture_.bindBuffer();
		shadowTexture_.bindRenderBuffer();

		var lightRot = new Quaternion();
		lightRot.setFromEuler(new Vector( -85.0, 0, 0).radians() );

		shadowViewMat_ = new Matrix();
		shadowViewMat_.identity();
    	shadowViewMat_.makeRotationFromQuaternion(lightRot);
    	shadowViewMat_.multiply( new Matrix().makeTranslation( -sceneCamera_.pos.x, 
					 	 									   -sceneCamera_.pos.y, 
					 	 									   -sceneCamera_.pos.z ) ); 

		// shadowCamera_.transform.pos.set_xyz( sceneCamera_.pos.x, 
		//  									 sceneCamera_.pos.y, 
		//  									 sceneCamera_.pos.z ); 
		GL.enable(GL.DEPTH_TEST);

		GL.depthFunc( GL.LESS );
		GL.depthRange( 0.0, 1.0 );
		GL.clearDepth( 1.0);
	  	GL.clear( GL.COLOR_BUFFER_BIT | GL.DEPTH_BUFFER_BIT );        

        GL.viewport(0, 0, 512, 512 );
		drawScenePass( sceneCamera_, true );

		shadowTexture_.unbindBuffer();
		shadowTexture_.unbindRenderBuffer();
	}

	function drawBeautyPass()
	{
		GL.viewport(0, 0, Luxe.screen.w, Luxe.screen.h );		

		//SHAD
		//worldShader_.set_texture("tex1", texShadDepthWrap );
		Luxe.renderer.target = null;

		drawScenePass( sceneCamera_, false );
	}

	public function drawScenePass( camera : Camera, isShadowPass : Bool )
	{
		var modelView = new Matrix();
		var modelViewLight = new Matrix();
		
		var mvp = new Matrix();
		var mvpLight = new Matrix();

		var normalMatrix = new Matrix();		

		//viewProj.multiplyMatrices( sceneCamera_.view.projection_matrix, sceneCamera_.view.view_matrix_inverse );
		// trace('view matrix is ${sceneCamera_.view.view_matrix}');

		for (sceneMesh in meshDB_)
		{
			var mesh : Mesh = sceneMesh.mesh_;

			if ((hugging_) && (sceneMesh.hugmesh_ != null)) {				
				mesh = sceneMesh.hugmesh_;
			}

			mesh.geometry.texture.bind();

	        for(sceneObj in sceneMesh.instList_) 
	        {
	        	var model = sceneObj.xform_.world.matrix;

	        	// SHAD
        		// modelViewLight.multiplyMatrices( shadowViewMat_, model  );
        		// mvpLight.multiplyMatrices( shadowProjMat_, modelViewLight );

	        	if (!isShadowPass)
	        	{
	        		modelView.multiplyMatrices( camera.view.view_matrix_inverse, model  );

	        		if (sceneObj.jitter_ > 0.0)
	        		{
	        			// HACK
	        			modelView.elements[12] += Luxe.utils.random.float( -1.0, 1.0 ) * sceneObj.jitter_;
	        			modelView.elements[13] += Luxe.utils.random.float( -1.0, 1.0 ) * sceneObj.jitter_;
	        			modelView.elements[14] += Luxe.utils.random.float( -1.0, 1.0 ) * sceneObj.jitter_;
	        		}

	        		mvp.multiplyMatrices( camera.view.projection_matrix, modelView );
	        	} else {
	        		mvp = mvpLight;	        		
	        	}

	        	normalMatrix.getInverse( model );
	        	normalMatrix.transpose();

	        	// if (sceneObj.name_ == "Suzilla" )
	        	// {
	        	// 	trace( 'normalMatrix: ${normalMatrix}' );
	        	// }

	        	var overrideShader : Shader = null;
	        	if (isShadowPass) 
	        	{	        		
	        		overrideShader = shadowShader_;
	        		overrideShader.set_matrix4( "mvp", mvp );
	        	} else {
	        		mesh.geometry.shader.set_vector3( "tintColor", sceneObj.tintColor );
	        		mesh.geometry.shader.set_matrix4( "mvp", mvp );
	        		mesh.geometry.shader.set_matrix4( "normalMatrix", normalMatrix );
	        		// mesh.geometry.shader.set_matrix4("mvpLight", mvpLight );
	        	}
	            Luxe.renderer.batcher.submit_geometry(mesh.geometry, overrideShader );
	        }
    	}
	}

	public function addSceneObj( name : String, meshID : String, texture : String ) : SceneObj
	{
			var sceneObj = new SceneObj(name);						
			sceneObj.xform_ = new Transform();

			var sceneMesh = lookupSceneMesh( "assets/mesh/" + meshID + ".dat", 
									 		 "assets/" + texture );
			sceneObj.boundSphere_ = sceneMesh.boundSphere_;

			// HACK : if this is the ground mesh, keep it around
			if (name=="Ground1") {
				trace("Found ground mesh" );
				groundMesh_ = sceneMesh;
				sceneObj.pickable_ = false;
			}
			else {
				sceneObj.pickable_ = true;
			}

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

	// Given an xz location, this fills in the y value so that 
	// it is snapped to the ground mesh.
	// NOTE: ignores xform (assumes identity)
	public function groundPos( pos : Vector ) : Vector
	{
		var result = pos.clone();
		if (groundMesh_==null) {
			trace("No ground mesh!");
			return result;
		}

		var vndx = 0;
		while (vndx < groundMesh_.mesh_.geometry.vertices.length)
		{			
			var A = groundMesh_.mesh_.geometry.vertices[vndx].pos;
			var B = groundMesh_.mesh_.geometry.vertices[vndx+1].pos;
			var C = groundMesh_.mesh_.geometry.vertices[vndx+2].pos;

			vndx += 3;

			var ab = Vector.Subtract( B, A );
			var ac = Vector.Subtract( C, A );
			var ap = Vector.Subtract( pos, A );

			var dd = ab.x * ac.z - ac.x * ab.z;
    		var v = (ap.x * ac.z - ac.x * ap.z) / dd;
    		var w = (ab.x * ap.z - ap.x * ab.z) / dd;
    		var u = 1.0 - v - w;

    		if ((v > 0.0) && (v < 1.0) &&
				(w > 0.0) && (w < 1.0) &&
				(u > 0.0) && (u < 1.0) )
			{
				result.y = u*A.y + v*B.y + w*C.y;
				return result; 
			}			
		}

		// Didn't find		
		result.y = 0.0;
		return result;
	}

	public function getSceneObjAtScreenPos( pos : Vector ) : SceneObj
	{
		var testRay = sceneCamera_.view.screen_point_to_ray( pos );
		var scnray = new SceneRay( testRay.origin, testRay.dir.normalize() );
		
		for (sceneMesh in meshDB_)
		{
			var mesh : Mesh = sceneMesh.mesh_;
	        for(sceneObj in sceneMesh.instList_) 
	        {
	        	if (!sceneObj.pickable_)
	        	{
	        		continue;
	        	}
	        	var hitTest = sceneObj.intersectRayBoundSphere( scnray );
	        	if (hitTest.hit_)
	        	{
	        		return sceneObj;
	        	}
	        }
	    }
	    return null;
	}

}