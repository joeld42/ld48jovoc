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
import phoenix.Shader;

import snow.api.buffers.Uint8Array;
import snow.system.assets.Assets;

import SceneRender;
import ZillaMover;
import SceneIntersect;

class Main extends luxe.Game {

	var worldShader_ : Shader;
	var hudBatcher_ :Batcher;

	var flyCamera_ : FlyCamera;
	var gameCamera_ : Camera;
	var gameCameraTarget_ : Vector;

	var scene_ : SceneRender;

	var zillaObj_ : SceneObj;
	var zilla_ : Entity;
	var zillaMover_ : ZillaMover;

	var lookatObj_ : SceneObj;
	var testSphereObj_ : SceneObj;
	var testHitObj_ : SceneObj;

	var mouseScreenPos_ : Vector;
	var mouseGroundPos_ : Vector;

	override function config( config : luxe.AppConfig )
	{
		config.render.antialiasing = 8;
		config.render.depth_bits = 24;
		config.render.depth = true;

		preloadResources(config);

		return config;
	}

	// TODO: make this a regular preload parcel and
	// just use the app preload to set up a preloadprogress
	function preloadResources( config : luxe.AppConfig )
	{
		// Configure texture
		var texNames = [ "ground1.jpg", "house.png", "suzilla.png", "axisGadget.png", "uvgrid.png" ];
		for (texName in texNames)
		{
			config.preload.textures.push({ id :  "assets/" + texName,
									clamp_s : ClampType.repeat, 
									clamp_t : ClampType.repeat  });
		}

		config.preload.shaders.push({ id:'world', frag_id:'assets/world.frag.glsl', vert_id:'assets/world.vert.glsl' });
		//config.preload.shaders.push({ id:'dbg_shad', frag_id:'assets/dbg_shad.frag.glsl', vert_id:'default' });
		//config.preload.shaders.push({ id:'shadow', frag_id:'assets/shadow.frag.glsl', vert_id:'assets/shadow.vert.glsl' });

		// TESTLAND
		config.preload.bytes.push({ id : "assets/mesh/MESH_Cube.001.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_GroundMesh.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_SuzillaMesh.dat" });
		
		// DEBUG MESHES
		config.preload.bytes.push({ id : "assets/mesh/MESH_axisGadgetMesh.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_TestSphereMesh.dat" });		

		config.preload.jsons.push( { id : "assets/testland.json" });

	}

	function initCamera()
	{
		// flyCamera_ = new FlyCamera({
  //   		name:'flycam',
  //   		projection: ProjectionType.perspective,
  //   		fov:90,
  //   		near:0.1,
  //   		far:1000,
  //   		cull_backfaces : true,
  //   		aspect:Luxe.screen.w/Luxe.screen.h
		// });

		gameCamera_ = new Camera({
    		name:'gamecam',
    		projection: ProjectionType.perspective,
    		fov:90,
    		near:0.1,
    		far:1000,
    		cull_backfaces : true,
    		aspect:Luxe.screen.w/Luxe.screen.h
		});
    	
    	scene_.sceneCamera_ = gameCamera_;
    	Luxe.renderer.batcher.view = gameCamera_.view;

    	gameCamera_.pos.set_xyz(0,20,15);
    	gameCamera_.rotation.setFromEuler( new Vector( -50.0, 0, 0).radians() );
    	gameCameraTarget_ = new Vector();
    	gameCamera_.view.target = gameCameraTarget_;


	}

    override function ready() 
    {
    	scene_ = new SceneRender();

    	mouseScreenPos_ = new Vector();
    	mouseGroundPos_ = new Vector();    	

 		worldShader_ = Luxe.resources.shader('world');
    	scene_.worldShader_ = worldShader_;
    	//scene_.shadowShader_ = Luxe.resources.shader('shadow');
    	
    	// init camera
    	initCamera();

    	// init lighting
    	Luxe.renderer.clear_color.rgb(0x6ca9d3);
    	var lightDir = new Vector( 1, -0.7, 1.0 );
    	lightDir.normalize();
    	lightDir.invert();
    	worldShader_.set_vector3( "lightDir", lightDir);

		hudBatcher_ = Luxe.renderer.create_batcher({ name:'hud_batcher', layer:4,  no_add : true});

    	scene_.loadScene( "assets/testland.json");
    	scene_.initShadows();

    	zillaObj_ = scene_.findSceneObj("Suzilla");
    	var camPos = new Vector();
    	camPos.copy_from(zillaObj_.xform_.pos);
    	
    	//flyCamera_.pos.set_xyz( camPos.x, camPos.y + 5.0, camPos.z );

    	lookatObj_ = scene_.findSceneObj( "axisGadget" );
    	testSphereObj_ = scene_.findSceneObj( "testSphere" );

    	testSphereObj_.xform_.pos.set_xyz( 0.0, 0.0, 1.0 );
    	testSphereObj_.xform_.scale.set_xyz( 2.0, 2.0, 2.0 );

		testHitObj_ = scene_.addSceneObj( "testHitObj", "MESH_axisGadgetMesh", "axisGadget.png");
			

    	bindInput();
    	setupGame();
    } //ready

 	function bindInput() {

        Luxe.input.bind_key('left', Key.key_a);
        Luxe.input.bind_key('left', Key.left);

        Luxe.input.bind_key('right', Key.key_d);
        Luxe.input.bind_key('right', Key.right);

        Luxe.input.bind_key('up', Key.key_w);
        Luxe.input.bind_key('up', Key.up);

        Luxe.input.bind_key('down', Key.key_s);
        Luxe.input.bind_key('down', Key.down);

        // Luxe.input.bind_key('jump', Key.key_w);
        // Luxe.input.bind_key('jump', Key.up);
        // Luxe.input.bind_key('jump', Key.space);
        // Luxe.input.bind_key('jump', Key.key_z);
        // Luxe.input.bind_key('jump', Key.key_x);

        Luxe.input.bind_key('camera', Key.tab );
    } //bind_input

	override function oninputup( _input:String, e:InputEvent ) {
        trace( 'named input up : ' + _input );
        if (_input=='up') {
        	zillaMover_.forceUp_ = 0.0;
        } else if (_input=='down') {
    		zillaMover_.forceDown_ = 0.0;        
        } else if (_input=='left') {
    		zillaMover_.forceLeft_ = 0.0;
        } else if (_input=='right') {
    		zillaMover_.forceRight_ = 0.0;
        }
    } //oninputup

    override function oninputdown( _input:String, e:InputEvent ) {
        trace( 'named input down : ' + _input );
        if (_input=='up') {
        	zillaMover_.forceUp_ = 1.0;
        } else if (_input=='down') {
    		zillaMover_.forceDown_ = 1.0;        
        } else if (_input=='left') {
    		zillaMover_.forceLeft_ = 1.0;
        } else if (_input=='right') {
    		zillaMover_.forceRight_ = 1.0;
        }
    } //oninputdown

    function setupGame()
    {
    	// Make our zilla and link it to the sceneObj
    	zilla_ = new Entity({ name:'zilla' });
    	zilla_.transform = zillaObj_.xform_;

    	zillaMover_ = new ZillaMover({ name:'zillaMover' });
 		zilla_.add( zillaMover_ );

 		var zillaSize = zillaObj_.boundSphere_.radius_; 		
 		testSphereObj_.xform_.scale.set_xyz( zillaSize, zillaSize, zillaSize ); 
    }

    override function onkeyup( e:KeyEvent ) {

    	// Don't use escape because flycamera needs it to
    	// to grab mouse
        if(e.keycode == Key.key_k) {
		// if(e.keycode == Key.escape) {        	
            Luxe.shutdown();
        } else if (e.keycode == Key.key_z) {
        	// zap player to origin
        	zilla_.pos.set_xyz( 0.0, 0.0, 5.0 );
        } else if (e.keycode == Key.key_n) {
        	testSphereObj_.xform_.pos.z -= 0.1;
		} else if (e.keycode == Key.key_m) {
			testSphereObj_.xform_.pos.z += 0.1;
        }

    } //onkeyup

	override function onpostrender() {

        scene_.drawScene();

        // Draw the hud batcher afterward so the hud scene stays on top        
		hudBatcher_.draw();

    } //onpostrender

	 override function onmousemove( e:MouseEvent ) {
        
	 	mouseScreenPos_.set_xy(e.pos.x, e.pos.y);        
     }

    override function update(dt:Float) 
    {
    	// Update camera 
    	var cameraTarg = Vector.Add( zilla_.pos, new Vector( 0.0, 0.0, 5.0 ) );
    	gameCamera_.pos.set_xyz( zilla_.pos.x + (zilla_.pos.x / 100.0) * 20.0, 
    							 zilla_.pos.y + 20.0, 
    							 zilla_.pos.z - 15.0 );
    	gameCameraTarget_.copy_from( cameraTarg );

    	lookatObj_.xform_.pos.copy_from( gameCameraTarget_ );
    	testSphereObj_.xform_.rotation.copy( zilla_.rotation );
		testSphereObj_.xform_.pos.copy_from( zilla_.pos );

		// trace("----- test ray -----" );
		var testRay = gameCamera_.view.screen_point_to_ray( mouseScreenPos_ );
		var scnray = new SceneRay( testRay.origin, testRay.dir.normalize() );

		//testHitObj_.xform_.pos.copy_from( Vector.Add( scnray.origin_, scnray.dir_ ) );

		var hitTest = testSphereObj_.intersectRayBoundSphere( scnray );

		if (hitTest.hit_) {
			// trace('MOUSE RAY scnRay ${scnray} ${testSphereObj_.boundSphere_} HIT ${hitTest.hitPoint_} ${hitTest.hitPoint_.length}');			
			testHitObj_.xform_.pos.copy_from( hitTest.hitPoint_ );
		} else {
			testHitObj_.xform_.pos.set_xyz( 0,4,0 );
			// trace('MOUSE RAY scnRay ${scnray} ${testSphereObj_.boundSphere_} MISS');
		}


    } //update


} //Main
