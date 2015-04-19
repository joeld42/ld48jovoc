
import luxe.Input;
import luxe.Mesh;
import luxe.Vector;
import luxe.Parcel;
import luxe.ParcelProgress;
import luxe.Color;
import luxe.Input;

import phoenix.geometry.Geometry;
import phoenix.Batcher;


class Main extends luxe.Game {

    // Player stuff
    var inputLeft : Vector;
    var inputRight : Vector;
    var inputUp : Vector;
    var inputDown : Vector;

    // Geometry
	var meshWorld : Mesh;	
	var meshPlayer : Mesh;

    // Everything is ready
    var gameReadySemaphore : Int = 2;

	override function config( config:luxe.AppConfig ) {

        config.render.depth_bits = 24;
        config.render.depth = true;

        return config;

    }

    function assets_loaded(_) {

		Luxe.camera.view.set_perspective({
    		far:1000,
            near:0.1,
            fov: 80.0,
            aspect : Luxe.screen.w/Luxe.screen.h
    	});

    	//move up and back a bit
    	Luxe.camera.pos.set_xyz(0,18,7.5);
    	Luxe.camera.rotation.setFromEuler( new Vector( -70.0, 0, 0).radians() );

        // World environment
        var tex = Luxe.loadTexture('assets/ld32_foodfight_env.png');    		
    	meshWorld = new Mesh({ file:'assets/ld32_foodfight_env.obj', 
    					texture:tex, onload:meshloaded });

		// Player
        var texPlayer = Luxe.loadTexture('assets/tmp_player.png');    		
    	meshPlayer = new Mesh({ file:'assets/ld32_foodfight_tmp_player.obj', 
    						texture:texPlayer, onload:meshloaded });
    } 

	override function ready() {

        inputLeft = new Vector();
        inputRight = new Vector();
        inputUp = new Vector();
        inputDown = new Vector();

		var preload = new Parcel();    	
    	preload.add_texture( "assets/ld32_foodfight_env.png");
    	preload.add_texture( "assets/tmp_player.png");

		preload.add_text( "assets/ld32_foodfight_env.obj" );
		preload.add_text( "assets/ld32_foodfight_tmp_player.obj" );

		new ParcelProgress({
            parcel      : preload,
            background  : new Color(0.4,0.4,0.4,0.85),
            oncomplete  : assets_loaded
        });

        preload.load();

        // bind inputs
        Luxe.input.bind_key( 'fire', Key.space );
        Luxe.input.bind_key( 'fire', Key.key_z );
        Luxe.input.bind_gamepad( 'fire', 0 );

        Luxe.input.bind_key( 'moveleft', Key.left );
        Luxe.input.bind_key( 'moveleft', Key.key_a );

        Luxe.input.bind_key( 'moveright', Key.right );
        Luxe.input.bind_key( 'moveright', Key.key_d );

        Luxe.input.bind_key( 'moveup', Key.up );
        Luxe.input.bind_key( 'moveup', Key.key_w );

        Luxe.input.bind_key( 'movedown', Key.down );
        Luxe.input.bind_key( 'movedown', Key.key_s );


    } // ready

    override function onkeyup( e:KeyEvent ) {

        if(e.keycode == Key.escape) {
            Luxe.shutdown();
        }
	} //onkeyup

	function meshloaded(_) {

        gameReadySemaphore -= 1;

	/*
            //create a second mesh based on the first one
        	mesh2 = new Mesh({
            	geometry : new Geometry({ primitive_type: PrimitiveType.triangles, batcher:Luxe.renderer.batcher }),
            texture : mesh.geometry.texture,
        	});

        	mesh2.geometry.vertices = [].concat(mesh.geometry.vertices);
        	mesh2.transform.pos.set_xy(1,0);
        	*/
    }        

    override function update(dt:Float) {

        // Is everything initted?
        if (gameReadySemaphore > 0) {
            return;
        }

        // update input dir                
        meshPlayer.pos.set_xyz(
            meshPlayer.pos.x + (inputLeft.x + inputRight.x + inputUp.x + inputDown.x) * dt,
            meshPlayer.pos.y + (inputLeft.y + inputRight.y + inputUp.y + inputDown.y) * dt,
            meshPlayer.pos.z + (inputLeft.z + inputRight.z + inputUp.z + inputDown.z) * dt );

        // trace('meshPlayer pos is ${meshPlayer.pos} inputDir is ${inputDir}\n');

    } //update


    // ===============================================
    //   INPUT HANDLERS
    // ===============================================
    override function oninputup( _input:String, e:InputEvent ) {
        trace( 'named input up : ' + _input );
        if ( _input == 'moveleft') {
                inputLeft.set_xyz( 0.0, 0.0, 0.0 );
            } else if (_input=='moveright') {
                inputRight.set_xyz( 0.0, 0.0, 0.0 );
            } else if (_input=='moveup') {
                inputUp.set_xyz( 0.0, 0.0, 0.0 );
            } else if (_input=='movedown') {
                inputDown.set_xyz( 0.0, 0.0, 0.0 );
            }
    } //oninputup

    override function oninputdown( _input:String, e:InputEvent ) {
        trace( 'named input down : ' + _input );
        var moveSpeed = 15.0;
        if ( _input == 'moveleft') {
                inputLeft.set_xyz( -moveSpeed, 0.0, 0.0 );
            } else if (_input=='moveright') {
                inputRight.set_xyz( moveSpeed, 0.0, 0.0 );
            } else if (_input=='moveup') {
                inputUp.set_xyz( 0.0, 0.0, -moveSpeed );
            } else if (_input=='movedown') {
                inputDown.set_xyz( 0.0, 0.0, moveSpeed );                
            }
    } //oninputdown
} //Main
