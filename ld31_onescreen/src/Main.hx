import luxe.Parcel;
import luxe.Input;
import luxe.Mesh;
import luxe.Color;
import luxe.Vector;
import luxe.Quaternion;
import luxe.Parcel;
import luxe.ParcelProgress;
import luxe.Sprite;
import luxe.Rectangle;
import luxe.Text;

import phoenix.Batcher;

import Gameboard;


class Main extends luxe.Game {

	var hud_batcher:Batcher;
    var mouse : Vector;
    var dragging : Bool = false;
	
	var gameboard : Gameboard;

	var loadCount : Int = 0;

	var testCard : Sprite;

    override function ready() {

    	mouse = new Vector();

		Luxe.renderer.clear_color = new Color().rgb( 0x1b383c );
    	var preload = new Parcel();

    	preload.add_texture( "assets/gameboard_5x5.png");
    	preload.add_texture( "assets/card_test.png");
		preload.add_text( "assets/gameboard_5x5.obj", true);

		new ParcelProgress({
            parcel      : preload,
            background  : new Color(1,1,1,0.85),
            oncomplete  : assets_loaded
        });

        preload.load();
        
        //connect_input();

    } //ready

    function assets_loaded(_)
    {
    	 Luxe.camera.view.set_perspective({
    		far:1000,
            near:0.1,
            aspect : Luxe.screen.w/Luxe.screen.h,
            // cull_backfaces: true,            
            // depth_test : true
    	});

    	 // create the hud
    	 create_hud();

    	 // load the test sprite
    	 var cardSz = 200.0;
    	 var cardTex = Luxe.loadTexture( 'assets/card_test.png' );

    	 for (i in 0...7)
    	 {
	    	 testCard = new Sprite({
	            name: 'card',
	            texture: cardTex,
	            batcher : hud_batcher,
	            pos: new Vector( 250 + i*(600/7.0), 520 - (Math.sin( (3.1415/6)*i )) * 50 ),
	            size: new Vector( cardSz * 0.7117, cardSz ),
	            rotation_z: -15 + ((30.0 / 7) * i),
	            depth: -5
			});
    	}

    		//move up and back a bit
    	Luxe.camera.pos.set_xyz(0,4.2,7.35);
    	Luxe.camera.rotation.setFromEuler( new Vector( -35.0, 0, 0).radians() );

    	gameboard = new Gameboard();
    	gameboard.setup( function(_) {
    			loadCount++;
    		} );
    }

    override function onkeyup( e:KeyEvent ) {

        if(e.keycode == Key.escape) {
            Luxe.shutdown();
        }

    } //onkeyup

    override function onmousedown(e:MouseEvent) {
        mouse = e.pos;
        dragging = true;    
    }

    override function onmouseup(e:MouseEvent) {

        mouse = e.pos;
        dragging = false;

    } //onmouseup

    override function onmousemove(e:MouseEvent) {

        mouse = e.pos;

    } //onmousemove

    override function update(dt:Float) 
    {
    	var v = (mouse.y / Luxe.screen.h);
    	// var boardRot = v * -50;
    	// trace('boardRot ${boardRot}');
    	// Luxe.camera.rotation.setFromEuler( new Vector( -35, -5.0 + (v*10), 0).radians() );

    	// Luxe.camera.pos.set_xyz( -5.0 + (v*10),Luxe.camera.pos.y, Luxe.camera.pos.z );
    	// trace(v * 10);
    } //update

    function create_hud() {

        //For the hud, it has a unique batcher, layer 4 is > the batcher_1, and the default(1)
        hud_batcher = Luxe.renderer.create_batcher({ name:'hud_batcher', layer:4 });

            //Now draw some text and the bar
        var small_amount = Luxe.screen.h * 0.05;

            //draw a bar on the bottom
        Luxe.draw.box({
            x : 0, y : Luxe.screen.h - small_amount,
            w : Luxe.screen.w, h: small_amount,
            color : new Color().rgb(0xf0f0f0),
                //here is the key, we don't store it in the default batcher, we store it in our new custom batcher
            batcher : hud_batcher
        });

        Luxe.draw.text({
            text : 'A HUD!',
            point_size : small_amount * 0.55,
            bounds : new Rectangle(small_amount/2, Luxe.screen.h - small_amount, Luxe.screen.w, small_amount),
            color : new Color().rgb(0xff4b03),
            batcher : hud_batcher,
            align_vertical : TextAlign.center
        });

    } //create_hud


} //Main
