import Std;
import Math;

import luxe.Input;
import luxe.Sprite;
import luxe.Color;
import luxe.Vector;
import luxe.Mesh;
import luxe.Rectangle;
import luxe.Text;
import luxe.Parcel;
import luxe.ParcelProgress;

import phoenix.Batcher;
import phoenix.Camera;
import phoenix.RenderTexture;
import phoenix.Shader;
import phoenix.Texture;
import phoenix.geometry.PlaneGeometry;
import phoenix.geometry.Geometry;
import phoenix.geometry.Vertex;
import phoenix.geometry.TextureCoord;

import PrimitiveGeom;
import MonkeyGeom;
//import CubeBevelGeom;

class Main extends luxe.Game {

 	public var mouse : Vector;
	var mesh : Mesh;
	var mesh2 : Mesh;
	var mesh3 : Mesh;
	var player : Mesh;

	var cshad : Shader;
	var distort_shader : Shader;

	var target_texture : RenderTexture;
    var batcherScene : Batcher;
    var cameraScene : Camera;  
    var display_sprite : Sprite;
    var winX : Float;
    var winY : Float;
    var score : Text;

	// var loaded : Bool = false;
	var loadCount : Int = 0;

	var red = new Color().rgb(0xf6007b);

	override function config( config:luxe.AppConfig ) {

        config.window.depth_bits = 24;
        return config;
    }

    function setupSceneRenderTarget()
    {
    	trace("setupSceneRenderTarget...");

    	// winX = 512;
    	// winY = 512;
    	winX = Luxe.screen.w;
    	winY = Luxe.screen.h;

    	//create a render target of a fixed size
        target_texture = new RenderTexture( Luxe.resources, new Vector( winX, winY ) );
        
        //create a new batcher to draw from, but don't add to the main rendering
        batcherScene = Luxe.renderer.create_batcher({
            name:'target_batcher',
            no_add : true
        });

            //camera should match the size of the texture if we want it to look right for this example
        batcherScene.view.viewport = new Rectangle(0,0,winX,winY);

            //Create a sprite, but don't add it to the default batcher,
            //add it to our custom batcher here instead.
        // example = new Sprite({
        //     texture : Luxe.loadTexture('assets/image.jpg'),
        //     pos : new Vector(256,256),
        //     size : new Vector(512,512),
        //     batcher : batcher
        // });

    }

    // function _obj_add_vert( v:luxe.importers.obj.Data.Vertex, _scale:Vector ) {

    //     var normal : Vector = new Vector();

    //        if(v.normal != null) {
    //             normal.set_xyz(v.normal.x, v.normal.y, v.normal.z);
    //        }

    //    var _v = new Vertex( new Vector( (v.pos.x * _scale.x) , 
    //    				(v.pos.y * _scale.y), (v.pos.z * _scale.z) ), 
    //    				new Color(), normal );

    //             //todo;multiple uv sets
    //        if(v.uv != null) {
    //            _v.uv.uv0.set_uv( v.uv.u, 1.0 - v.uv.v ); // inverted from texture space
    //        }

    //    geometry.add( _v );

    // } //_obj_add_vert

    function testMakeGeo( texture:Texture, batcher:Batcher ) : Geometry
    {
		var geometry = new Geometry({
            texture : texture,
            primitive_type: PrimitiveType.triangles,
            immediate : false,
            batcher : batcher,
            depth : 1 //:todo : optionise
        });

        var _v1 = new Vertex(new Vector(0.0, 0.0, 0.0 ),
        	                new Color(),
        	                new Vector(0.0, 1.0, 0.0) );
        _v1.uv.uv0.set_uv( 0.0, 0.0 );
        geometry.add( _v1 );

        var _v2 = new Vertex(new Vector( 10.0, 0.0, 0.0 ),
        	                new Color(),
        	                new Vector(0.0, 1.0, 0.0) );
        _v2.uv.uv0.set_uv( 0.0, 1.0 );
        geometry.add( _v2 );

        var _v3 = new Vertex(new Vector(0.0, 0.0, -10.0 ),
        	                new Color(),
        	                new Vector(0.0, 1.0, 0.0) );
        _v3.uv.uv0.set_uv( 1.0, 0.0 );
        geometry.add( _v3 );


        return geometry;
    }

    function ready_OLD() 
    {

		mouse = new Vector();

           //fetch a list of assets to load from the json file
        var json_asset = Luxe.loadJSON('assets/parcel.json');

            //then create a parcel to load it for us
        var preload = new Parcel();
        preload.from_json(json_asset.json);

            //but, we also want a progress bar for the parcel,
            //this is a default one, you can do your own
        // new ParcelProgress({
        //     parcel      : preload,
        //     background  : new Color(1,1,1,0.85),
        //     oncomplete  : assets_loaded
        // });

            //go!
        preload.load();

        trace( "ready finished, preloading now...");

    } //ready

	// function assets_loaded( _ ) 
	override function ready()
	{
		mouse = new Vector();

		trace( "In assets_loaded...");
		Luxe.renderer.clear_color = new Color().rgb( 0x333333 );

    	// setup the render target
    	setupSceneRenderTarget();

    	cameraScene = new Camera();
		cameraScene.set_perspective({
    		far:1000,
            near:0.1,
            aspect : Luxe.screen.w/Luxe.screen.h,
			// aspect : 1.0,
            // cull_backfaces: true,
            cull_backfaces: false,            
            depth_test : true
    	});

  //   		//move up and back a bit
     	cameraScene.pos.set_xyz(0,0.6,10);

     	batcherScene.view = cameraScene;


    	//create the mesh
    	var tex = Luxe.loadTexture('assets/simpletex.png');    	
    	tex.clamp = repeat;    
    	tex.filter = nearest;
    	mesh = new Mesh({ file:'assets/cube_bevel.obj', 
    				  	texture: tex, batcher:batcherScene });
		mesh.pos.set_xyz( 2.0, 0.5, 1.0 );

    	// works
		mesh2 = new Mesh({ file:'assets/cube_bevel.obj', 
			            texture: tex, batcher:batcherScene });

		// doesn't work
    	// mesh2 = new Mesh( { geometry: mesh.geometry, 
    	// 					texture : tex });
    	mesh2.pos.set_xyz( -3.0, 0.5, -4.0 );

		cshad = Luxe.loadShader('assets/cust_frag.glsl');
	    mesh.geometry.shader = cshad;
	    
	 //    ground = new Mesh({ file:'assets/grid10x10.obj', 
		// 	            texture: tex, batcher:batcherScene });
		// ground.pos.set_xyz( 0.0, -0.5, 0.0 );


		//var testGeo = testMakeGeo( tex, batcherScene );

		// var testGeo = PrimitiveGeom.makeCube( new Vector(0.0, 0.0, 0.0), 1.0,
		// 									  tex, batcherScene );

		var testGeo = MonkeyGeom.makeGeometry( tex, batcherScene );
		//var testGeo = CubeBevelGeom.makeGeometry( tex, batcherScene );

		mesh3 = new Mesh({ geometry: testGeo,
							texture: tex,
							batcher: batcherScene
			});
		mesh3.pos.set_xyz( -1.0, 0.5, 0.0 );
		mesh3.rotation.setFromEuler( new Vector( 15.0, 30.0, 0.0).radians(), XYZ );


	 //    player = new Mesh({ file:'assets/player.obj', 
		// 	            texture: tex, batcher:batcherScene });
		// player.pos.set_xyz( 0.0, 0.0, 0.0 );

	    // display sprite shows the scene
	     tex.onload = function(t) {

            display_sprite = new Sprite({
                texture : target_texture,
                size : new Vector(winX, winY),
                pos : Luxe.screen.mid,
                depth: -10
            });

            loadCount += 1;
         };


		var distort_map = Luxe.loadTexture('assets/distort.png');
		distort_map.onload = function(tt) {

	    	distort_shader = Luxe.loadShader('assets/distort.glsl');
	    	display_sprite.shader = distort_shader;

	    		//move to second slot
	    	distort_map.slot = 1;
	    		//set the uniform
	    	distort_shader.set_texture('tex1', distort_map);

	    	loadCount += 1;

    	} //distort map onload




        // Test text objects
        score = new Text({
            no_scene : true,
            text : "score: ??", align : left,
            pos : new Vector(winX-150, 10),
            point_size : 24,
            // color: red
        });

	} //ready

    override function onmousemove( e:MouseEvent ) {
        mouse.set_xy(e.x,e.y);
    } //onmousemove

    override function onmousedown( e:MouseEvent ) {
        mouse.set_xy(e.x,e.y);
    } //onmousedown

    override function onmouseup( e:MouseEvent ) {
        mouse.set_xy(e.x,e.y);
    } //onmouseup

    override function onkeyup( e:KeyEvent ) {
        if(e.keycode == Key.escape) {
            Luxe.shutdown();
        }
    } //onkeyup

    override function onkeydown( e:KeyEvent ) {
        // if(e.keycode == Key.w) {
        //     trace("W pressed..." );
        // }
    } //onkeyup

	override function onprerender() 
	{
		if (loadCount >= 2)
		{
            //Set the current rendering target
		
        	Luxe.renderer.target = target_texture;
        	// Luxe.renderer.camera = cameraScene;

			//clear the texture!
			Luxe.renderer.clear(new Color().rgb(0x4E7288));

			//draw the geometry inside our batcher
			batcherScene.draw();

			//reset the target back to no target (i.e the screen)
        	Luxe.renderer.target = null;
        	// Luxe.renderer.camera = null;
        }

    } //onprerender

	var y = 0.0;
    override function onrender() {
		// Luxe.draw.rectangle({
  //               //this line is important, as each frame it will create new geometry!
  //           immediate : true,
  //           x : mouse.x-85, y : mouse.y,
  //           w : 170,
  //           h : 32,
  //           color : new Color(Math.random(),Math.random(),Math.random(),0.5)
  //       });    	
    }

	override function update(dt:Float) {

		if (loadCount >= 2)
		{
		  	y += 10 * dt;
	        mesh.rotation.setFromEuler(new Vector(0,y,0).radians());

	        mesh3.rotation.setFromEuler(new Vector(0,-y,0).radians());

	  //       var yy = y / 30.0;
	  //       var dist : Float;
	  //       dist  = yy - Math.floor(yy);
			// distort_shader.set_float('distortamount', dist * 2);

	        var yint : Int;
	        yint = Std.int(y);
	        score.text = "Score: " + yint;
		}
    }

}