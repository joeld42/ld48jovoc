

import luxe.Input;
import luxe.Mesh;
import luxe.Color;
import luxe.Vector;
import luxe.Parcel;
import luxe.ParcelProgress;
import luxe.Sprite;
import luxe.Camera;
import luxe.Rectangle;
import luxe.Entity;

import luxe.components.cameras.FlyCamera;

import phoenix.geometry.Geometry;
import phoenix.geometry.Vertex;
import phoenix.geometry.TextureCoord;
import phoenix.Batcher;
import phoenix.Texture;
// import phoenix.Camera;

import phoenix.Shader;

import snow.api.buffers.Uint8Array;
import snow.system.assets.Assets;

import SceneBuilder;
import Rotate;

// notes: small things that would be useful
//  -- create a resource on the fly
//  -- build a shader from source

class Main extends luxe.Game {

	var meshGround_ : Mesh;
	var meshTree_ : Mesh;	
	var meshPlayer_ : Mesh;
	var meshOak_ : Mesh;

	var worldShader_ : Shader;

	var builder_ = new SceneBuilder();

	var hudBatcher_ :Batcher;

	var flyCamera_ : FlyCamera;

	var testEntiity_ : Entity;

	var shadowBuffSprite_ : Sprite;

	override function config( config : luxe.AppConfig )
	{
		// config.render.antialiasing = 8;
		config.render.depth_bits = 24;
		config.render.depth = true;

		// var texNames = [ "ground_swirl.png", "img_tree.png", "player.png", "pine.png", 
		// 				 "missing.png", "rock.png", "stone.png"];


		var texNames = [ "stone.png", "ground_swirl.png"];

		for (texName in texNames)
		{
			config.preload.textures.push({ id :  "assets/" + texName,
									clamp_s : ClampType.repeat, 
									clamp_t : ClampType.repeat  });
		}

		config.preload.texts.push({ id :  "assets/warmup.glsl" });
		config.preload.shaders.push({ id:'world', frag_id:'assets/world.frag.glsl', vert_id:'assets/world.vert.glsl' });
		config.preload.shaders.push({ id:'dbg_shad', frag_id:'assets/dbg_shad.frag.glsl', vert_id:'default' });
		config.preload.shaders.push({ id:'shadow', frag_id:'assets/shadow.frag.glsl', vert_id:'assets/shadow.vert.glsl' });

		// TEST SHAPE
		config.preload.bytes.push({ id : "assets/mesh/MESH_Grid.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_Cylinder.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_Cylinder.002.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_Cylinder.003.dat" });

		// SMALL FOREST SCENE
		config.preload.texts.push({ id :  "assets/grid10x10.obj" });
		// config.preload.texts.push({ id :  "assets/tree_fir.obj" });
		//config.preload.texts.push({ id :  "assets/player.obj" });

		// config.preload.bytes.push({ id : "assets/mesh/MESH_Cube.dat" });

		// config.preload.bytes.push({ id : "assets/mesh/MESH_Brown_Cliff_Mesh.dat" });
		// config.preload.bytes.push({ id : "assets/mesh/MESH_Brown_Cliff_Top_Mesh.dat" });
		// config.preload.bytes.push({ id : "assets/mesh/MESH_Cube.dat" });
		// config.preload.bytes.push({ id : "assets/mesh/MESH_Ground.dat" });
		// config.preload.bytes.push({ id : "assets/mesh/MESH_Rock1Mesh.dat" });
		// config.preload.bytes.push({ id : "assets/mesh/MESH_TreeOakMesh.dat" });
		// config.preload.bytes.push({ id : "assets/mesh/MESH_TreePineMesh.dat" });

		// config.preload.jsons.push( { id : "assets/forest_small_test.json" });
		config.preload.jsons.push( { id : "assets/test_shapes.json" });

		return config;
	}

    override function ready() {

    	// Luxe.camera.view.set_perspective({
    	// 	far:1000,
     //        near:0.1,
     //        fov: 90,
     //        cull_backfaces : false,
     //        aspect : Luxe.screen.w/Luxe.screen.h
    	// });

    	flyCamera_ = new FlyCamera({
    		name:'flycam',
    		projection: ProjectionType.perspective,
    		fov:90,
    		near:0.1,
    		far:1000,
    		cull_backfaces : true,
    		aspect:Luxe.screen.w/Luxe.screen.h
		});

 		worldShader_ = Luxe.resources.shader('world');
    	builder_.testShader_ = worldShader_;
    	builder_.shadowShader_ = Luxe.resources.shader('shadow');
    	

    	var lightDir = new Vector( 0.5, -1.0, 0.0 );
    	lightDir.normalize();
    	lightDir.invert();
    	worldShader_.set_vector3( "lightDir", lightDir);
    	trace( 'Shader is ${worldShader_}' );

    	builder_.sceneCamera_ = flyCamera_;
    	
		Luxe.renderer.batcher.view = flyCamera_.view;

		//move up and back a bit
    	// flyCamera_.pos.set_xyz(0,20,15);
    	flyCamera_.rotation.setFromEuler( new Vector( -50.0, 0, 0).radians() );

    	// var groundTex = Luxe.resources.texture( 'assets/ground_swirl.png');
    	// groundTex.generate_mipmaps();
    	// groundTex.clamp_s = groundTex.clamp_t = ClampType.repeat;
    	//  // groundTex.filter = mip_linear_linear;
    	// meshGround_ = new Mesh({ file : 'assets/grid10x10.obj', texture : groundTex } );    	    	
    	// meshGround_.pos.set_xyz( 0.0, -2.0, 0.0 );
    	// meshGround_.geometry.locked = true;    	

    	// builder_.loadScene( "assets/forest_small_test.json");
    	builder_.loadScene( "assets/test_shapes.json");
    	builder_.initShadows();

		hudBatcher_ = Luxe.renderer.create_batcher({ name:'hud_batcher', layer:4,  no_add : true});

		var shadowSpriteSize = 256;
		var shadowTex = new Texture({ id : "shadowSpriteTex", width:512, height:512, texture: builder_.texShadDepth_ });
		shadowBuffSprite_ = new Sprite({
    	 	batcher : hudBatcher_,
            // texture : builder_.shadowTexture_,
            // texture : builder_.texShadDepth_,
            texture : Luxe.resources.texture("assets/stone.png"),
            size : new Vector(shadowSpriteSize, shadowSpriteSize),
            //pos : Luxe.screen.mid,
            pos : new Vector( 10 + shadowSpriteSize/2, 10+shadowSpriteSize/2 )
        });
        var dbgShadShader = Luxe.resources.shader('dbg_shad');
        dbgShadShader.set_texture('tex1', shadowTex );
        shadowBuffSprite_.depth = -100;
        shadowBuffSprite_.shader = dbgShadShader;

  //       // var shadowTex2 = new Texture({ id : "shadowSpriteTex", 
  //       // 	width:512, height:512, texture: builder_.shadowTexture_ });
		// var shadowBuffSprite2 = new Sprite({
  //   	 	batcher : hudBatcher_,
  //           // texture : builder_.shadowTexture_,
  //           // texture : builder_.texShadDepth_,
  //           texture : builder_.shadowTexture_,
  //           size : new Vector(shadowSpriteSize, shadowSpriteSize),
  //           //pos : Luxe.screen.mid,
  //           pos : new Vector( 30 + shadowSpriteSize + shadowSpriteSize/2, 10 +shadowSpriteSize/2 )
  //       });
  //       shadowBuffSprite_.depth = -100;

		// set up test entity
		var testObj_ = builder_.findSceneObj( "Cylinder.003");
		testEntiity_ = new Entity({ transform: testObj_.xform_ });		
 		var rotator = new Rotate({ name:'rotator' });
 		testEntiity_.add( rotator );

 		// Add a buncha objects for testing
 		makeTestGrid();
    }

	function makeTestGrid() 
    {    	
    	var count = 100;
    	var countSide = Math.sqrt(count);
    	var center = (countSide * 3.0) / 2.0;
    	for (i in 0...count)
    	{
			var obj = builder_.addSceneObj( "TestObj_" + Std.string(i), "MESH_Cylinder", "stone.png");
			var ii : Float = (Std.int(i/countSide) * 4) - center;
			var jj : Float = (Std.int(i%countSide) * 4) - center;
			if (Std.int(i/10)%2 > 0 ) {
				jj += 2.0;
			}
			obj.xform_.pos.set_xyz( ii, 0, jj ); 			
		}    	
    }

	override function onpostrender() {

        builder_.drawScene();

        // Draw the hud batcher afterward so the hud scene stays on top        
		hudBatcher_.draw();

    } //onrender

	// load effect for shaders
    function load_effect( path:String ) {
        trace( '>>> load_effect: ${path}');
        return Luxe.resources.text( path ).asset.text;
    }

    override function onkeyup( e:KeyEvent ) {

        if(e.keycode == Key.escape) {
            Luxe.shutdown();
        }

    } //onkeyup

    var rot = 0.0;
    override function update(dt:Float) {    	
    	rot += 90*dt;
    	//builder_.update( dt );
    	//meshOak_.rotation.setFromEuler(new Vector(0,rot,0).radians());
    } //update


} //Main
