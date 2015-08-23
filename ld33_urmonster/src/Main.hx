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
import luxe.Particles;

import luxe.components.cameras.FlyCamera;

import phoenix.geometry.Geometry;
import phoenix.geometry.Vertex;
import phoenix.geometry.TextureCoord;
import phoenix.Batcher;
import phoenix.Texture;
import phoenix.Shader;
import phoenix.Quaternion;

import luxe.tween.Actuate;

import snow.api.buffers.Uint8Array;
import snow.system.assets.Assets;

import SceneRender;
import SceneIntersect;
import Building;

class Main extends luxe.Game {

	var worldShader_ : Shader;
	var hudBatcher_ :Batcher;

	var flyCamera_ : FlyCamera;
	var gameCamera_ : Camera;
	var gameCameraTarget_ : Vector;

	var scene_ : SceneRender;

	var zillaObj_ : SceneObj;
	var zilla_ : Entity;	
	var reye_ : SceneObj;
	var leye_ : SceneObj;

	var lookatObj_ : SceneObj;
	var testSphereObj_ : SceneObj;
	var testHitObj_ : SceneObj;

	var mouseScreenPos_ : Vector;
	var mouseGroundPos_ : Vector;

	var extraBatcher_ : Batcher;
	// var hugSprite_ : Sprite;

	var forceUp_ : Float = 0.0;
	var forceDown_ : Float = 0.0;
	var forceLeft_ : Float = 0.0;
	var forceRight_ : Float = 0.0;

	var buildings_ : Array<Building>;

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
		var texNames = [ "hugzilla_land.png", "house.png", "suzilla.png", "axisGadget.png", "uvgrid.png",
						"zilla_dif.png", "office.png", "tower.png" ];
		for (texName in texNames)
		{
			config.preload.textures.push({ id :  "assets/" + texName,
									clamp_s : ClampType.repeat, 
									clamp_t : ClampType.repeat  });
		}

		config.preload.textures.push({ id : "assets/healthbar_half.png"});		

		config.preload.shaders.push({ id:'world', frag_id:'assets/world.frag.glsl', vert_id:'assets/world.vert.glsl' });
		//config.preload.shaders.push({ id:'dbg_shad', frag_id:'assets/dbg_shad.frag.glsl', vert_id:'default' });
		//config.preload.shaders.push({ id:'shadow', frag_id:'assets/shadow.frag.glsl', vert_id:'assets/shadow.vert.glsl' });

		// Level
		config.preload.bytes.push({ id : "assets/mesh/MESH_Cube.001.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_GroundMesh.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_OfficeMesh.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_TowerMesh.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_TowerMesh.dat" });

		// ZILLA
		config.preload.bytes.push({ id : "assets/mesh/MESH_BodyMesh.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_BodyHugMesh.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_EyeMesh.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_FootMesh.dat" });
		
		// DEBUG MESHES
		config.preload.bytes.push({ id : "assets/mesh/MESH_axisGadgetMesh.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_TestSphereMesh.dat" });		

		config.preload.jsons.push( { id : "assets/hugzilla_land.json" });
		config.preload.jsons.push( { id : "assets/monster_export.json" });

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
    		cull_backfaces : false,
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

    	scene_.loadScene( "assets/hugzilla_land.json");
    	scene_.loadScene( "assets/monster_export.json");
    	scene_.initShadows();

    	zillaObj_ = scene_.findSceneObj("Zilla");
    	var camPos = new Vector();
    	camPos.copy_from(zillaObj_.xform_.pos);
    	
    	//flyCamera_.pos.set_xyz( camPos.x, camPos.y + 5.0, camPos.z );

    	lookatObj_ = scene_.findSceneObj( "axisGadget" );
    	testSphereObj_ = scene_.findSceneObj( "testSphere" );
    	testSphereObj_.pickable_ = false;

    	testSphereObj_.xform_.pos.set_xyz( 0.0, -10.0, 0.0 );
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

        Luxe.input.bind_key('hug', Key.key_z);
		Luxe.input.bind_key('hug', Key.space);

        // Luxe.input.bind_key('jump', Key.space);
        // Luxe.input.bind_key('jump', Key.key_z);
        // Luxe.input.bind_key('jump', Key.key_x);

        Luxe.input.bind_key('camera', Key.tab );
    } //bind_input

	override function oninputup( _input:String, e:InputEvent ) {
        // trace( 'named input up : ' + _input );
        if (_input=='up') {
        	forceUp_ = 0.0;
        } else if (_input=='down') {
    		forceDown_ = 0.0;        
        } else if (_input=='left') {
    		forceLeft_ = 0.0;
        } else if (_input=='right') {
    		forceRight_ = 0.0;
        } else if (_input=='hug') {
        	scene_.hugging_ = false;
        }
    } //oninputup

    override function oninputdown( _input:String, e:InputEvent ) {
        // trace( 'named input down : ' + _input );
        if (_input=='up') {
        	forceUp_ = 1.0;
        } else if (_input=='down') {
    		forceDown_ = 1.0;        
        } else if (_input=='left') {
    		forceLeft_ = 1.0;
        } else if (_input=='right') {
    		forceRight_ = 1.0;
        } else if (_input=='hug') {
        	scene_.hugging_ = true;
        }
    } //oninputdown

    function setupGame()
    {
    	// Make our zilla and link it to the sceneObj
    	zillaObj_.pickable_ = false;
    	zilla_ = new Entity({ name:'zilla' });
    	zilla_.transform = zillaObj_.xform_;

		// setup our zilla parts
		leye_ = scene_.findSceneObj( "leye" );
		leye_.xform_.parent = zillaObj_.xform_;
		leye_.pickable_ = false;

		reye_ = scene_.findSceneObj( "reye" );
		reye_.xform_.parent = zillaObj_.xform_;
		reye_.pickable_ = false;

		var lfoot = scene_.findSceneObj( "lfoot" );
		lfoot.xform_.parent = zillaObj_.xform_;
		lfoot.pickable_ = false;

		var rfoot = scene_.findSceneObj( "rfoot" );
		rfoot.xform_.parent = zillaObj_.xform_;
		rfoot.pickable_ = false;

 		var zillaSize = zillaObj_.boundSphere_.radius_; 		
 		testSphereObj_.xform_.scale.set_xyz( zillaSize, zillaSize, zillaSize ); 

		extraBatcher_ = Luxe.renderer.create_batcher({ name:'xtra_batcher',  
			   		//cull_backfaces : false,
					no_add : true });
		extraBatcher_.view = gameCamera_.view;

 		// hugSprite_ = new Sprite({
 		// 	name: "hug",
 		// 	pos : new Vector( 0, 0, 0 ),
 		// 	size : new Vector( 3.0, 1.0, 0.0 ),
 		// 	texture : Luxe.resources.texture("assets/uvgrid.png"),
 		// 	batcher: extraBatcher_
 		// 	});

 		buildings_ = new Array<Building>();
 		trace('setupgame, have ${scene_.buildings_.length} buildings...');
 		for (buildObj in scene_.buildings_)
 		{ 			
 			trace('building ${buildObj.name_}');
 			var sepIndex = buildObj.name_.indexOf(".");
 			var buildType = buildObj.name_.substring( 5, sepIndex );
 			var bldg = new Building( buildObj.name_, buildType, buildObj, extraBatcher_ );
 			buildings_.push( bldg );
 		}

 		// start pos
 		zilla_.pos.set_xyz( -1.4266520826387687,-0.000014674251868949773,-90.06991310268205 );
    }

    override function onkeyup( e:KeyEvent ) {

    	// Don't use escape because flycamera needs it to
    	// to grab mouse
        //if(e.keycode == Key.key_k) {
		if(e.keycode == Key.escape) {        	
            Luxe.shutdown();
        } else if (e.keycode == Key.key_l) {
        	// zap player to origin
        	zilla_.pos.set_xyz( 0.0, 0.0, 5.0 );
        } else if (e.keycode == Key.key_g) {

        	var g = scene_.groundPos( zilla_.pos );
        	trace('GROUND: ${g}');
        }

    } //onkeyup

	override function onpostrender() {

        scene_.drawScene();        

        extraBatcher_.draw();

        // Draw the hud batcher afterward so the hud scene stays on top        
		hudBatcher_.draw();

    } //onpostrender

	 override function onmousemove( e:MouseEvent ) {
        
	 	mouseScreenPos_.set_xy(e.pos.x, e.pos.y);        
     }

     function eyeLookat( eye : SceneObj, focus : Vector )
     {
     	//var eyePos = eye.xform_.pos.clone();     	
     	var focusEyeSpace = focus.clone();

     	//var zillaMat = zilla_.transform.world.matrix.inverse();
     	var zillaMat = zilla_.transform.world.matrix.inverse();
		focusEyeSpace.transform( zillaMat );

		focusEyeSpace.subtract( eye.xform_.pos );

     	//var lookDir = Vector.Subtract( focusEyeSpace, eyePos );	    
     	var lookDir = focusEyeSpace.clone();
      	lookDir.normalize();	
      	/*
      	if (eye.name_ == "reye")
      	{
     		// trace('focusEyeSpace ${focusEyeSpace} eyePos ${eyePos}');
     		// trace('focusEye ${focusEyeSpace} lookat ${lookDir}');

     		var cnt = new Vector( Luxe.screen.w/2, Luxe.screen.h/2 );
			Luxe.draw.ring({
	            x : cnt.x,
	            y : cnt.y,
	            r : 50,            
	            color : new Color(0,0,0,1).rgb(0xffffff),
				immediate:true,
				batcher : hudBatcher_
	        });


			Luxe.draw.line({
	            p0 : cnt,
	            p1 : new Vector( cnt.x + lookDir.x * 200, cnt.y + lookDir.z * 200 ),	            
	            color : new Color(1,0,0),
				immediate:true,
				batcher : hudBatcher_
	        });
     	}
     	*/

     	var frontDir = new Vector(0.0, 0.0, 1.0);

     	if (lookDir.dot( frontDir ) < 0.0)
     	{
	     	var axis = new Vector().cross( lookDir, frontDir );
	     	var ang = Math.asin( axis.length );

	     	// limit eye rotation
	     	if (ang * (180.0/Math.PI) < 45 ) {
		     	axis.normalize();
		     	eye.xform_.rotation.setFromAxisAngle( axis, ang );
	     	}
     	}


     }

    override function update(dt:Float) 
    {    	
    	// Update player
    	updateZilla( dt );
    	
    	// Update camera 
    	var cameraTarg = Vector.Add( zilla_.pos, new Vector( 0.0, 0.0, 5.0 ) );
    	
    	// cameraTarg.y = 0.0;
    	gameCamera_.pos.set_xyz( zilla_.pos.x + (zilla_.pos.x / 100.0) * 8.0, 
    							 zilla_.pos.y + 25.0, 
    							 zilla_.pos.z - 15.0 );
    	gameCameraTarget_.copy_from( cameraTarg );

    	lookatObj_.xform_.pos.copy_from( gameCameraTarget_ );

    	// update player sprite
    	// hugSprite_.pos.set_xyz( zilla_.pos.x, zilla_.pos.y + 6.0, zilla_.pos.z );
    	// hugSprite_.rotation.copy( gameCamera_.rotation );

    	// The focus obj is where the zilla looks at
    	var testRay = gameCamera_.view.screen_point_to_ray( mouseScreenPos_ );
		var scnRay = new SceneRay( testRay.origin, testRay.dir.normalize() );
		// test a plane at roughly eye level
		var result = scnRay.intersectPlane( new Vector(0.0, 1.0, 0.0), new Vector( 0.0, zilla_.pos.y + 4.0, 0.0) );
		
		if (result.hit_)
		{
			// trace( 'focus point ${result.hitPoint_}');
			testHitObj_.xform_.pos.copy_from( result.hitPoint_ );

			eyeLookat( leye_, result.hitPoint_  );
			eyeLookat( reye_, result.hitPoint_  );
		}

    	// Test for highlight object
    	// Spent 3 hours writing this crap and I don't think I'm going to use it...
    	/*
    	var pickObj = scene_.getSceneObjAtScreenPos( mouseScreenPos_ );

    	if (pickObj != null)
    	{
    		testSphereObj_.xform_.pos.copy_from( pickObj.xform_.pos );
    		var sz = pickObj.boundSphere_.radius_;
    		testSphereObj_.xform_.scale.set_xyz( sz, sz, sz );
    	} else {
    		testSphereObj_.xform_.pos.set_xyz( 0.0, -10.0, 0.0 );
    	}
    	*/

    	/*
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
		*/
    } //update

    function updateZilla(dt:Float)
    {
    	var fwd = zilla_.transform.world.matrix.forward();
        fwd.normalize();
        fwd.multiplyScalar( forceUp_ - forceDown_ );

        // var right = entity.transform.world.matrix.right();
        // right.normalize();
        // right.multiplyScalar( forceRight_ - forceLeft_ );

  //   	// Update zilla ground pos
		// var g = scene_.groundPos( zilla_.pos );
		// if (Math.abs( g.y - zilla_.pos.y) < 0.2) {
		// 	zilla_.pos.y = g.y;		
		// }
		var oldPos = zilla_.pos.clone();

        var moveDir = fwd.clone();
        moveDir.normalize();

        moveDir.multiplyScalar( dt * 4.0);
        var newPos = new Vector().copy_from( zilla_.pos );
        //Vector.Add( zilla_.pos, moveDir );

        // check if our new pos doesn't hit anything
        var hitBuilding : Building = null;
		for ( substep in 0...10)
        {
            // Try the new position for collision
            if ((substep % 2)==0) {
                // Even substep, move x
                newPos.x += moveDir.x;
            } else {
                newPos.z += moveDir.z;
            }

	        var isGood = true;
	        for (b in buildings_)
	        {
	        	if (b.checkCollide(newPos, 2.0 ))
	        	{
	        		isGood = false;
	        		hitBuilding = b;
	        		break;	        		
	        	}
	        }

	        if (isGood)
	        {
	     	   zilla_.pos.copy_from( newPos );
	    	} else {
	    		break;
	    	}
	    }

	    if (hitBuilding!=null) {
	    	scene_.hugging_ = true;
	    	if (hitBuilding.takeDamage( 2.0 * dt ))
	    	{
	    		// building dead...
	    		buildings_.remove( hitBuilding );
	    		Actuate.tween( hitBuilding.sceneObj_.xform_.pos, 0.5, {  y : hitBuilding.sceneObj_.xform_.pos.y - hitBuilding.sceneObj_.boundSphere_.radius_ } );
	    		Actuate.tween( hitBuilding.sceneObj_.xform_.rotation, 0.7, { x : Luxe.utils.random.float( -1.0, 1.0 ) });
	    	}
	    } else {
	    	scene_.hugging_ = false;
	    }

	    // janky check for ground hit .. if we can't move
	    // here then just undo	    
		var g = scene_.groundPos( zilla_.pos );
		if (Math.abs( g.y - zilla_.pos.y) < 0.2) {
			zilla_.pos.y = g.y;
		} else {
			// undo move
			zilla_.pos.copy_from( oldPos );
		}

	    // update all the buildings
	    for (b in buildings_)
        {
        	b.update( dt, gameCamera_ );
        }
	        

        var q = new Quaternion();
     	q.setFromAxisAngle( new Vector( 0.0, 1.0, 0.0), (forceLeft_-forceRight_)*4.0*dt );
        zilla_.rotation.multiply(q);
    }


} //Main
