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
import Card;

class Main extends luxe.Game {

	var hud_batcher:Batcher;
    var mouse : Vector;
    var dragging : Bool = false;
	
	var gameboard : Gameboard;

	var loadCount : Int = 0;	
	var loaded : Bool = false;

	var hand : Array<Card>;

    override function ready() {

    	mouse = new Vector();
    	hand = new Array<Card>();

		Luxe.renderer.clear_color = new Color().rgb( 0x1b383c );
    	var preload = new Parcel();

    	preload.add_texture( "assets/gameboard_5x5.png");
    	preload.add_texture( "assets/card_test.png");
    	preload.add_texture( "assets/testgrid.png");

		preload.add_text( "assets/gameboard_5x5.obj", true);
		preload.add_text( "assets/cursor.obj", true );

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

    	// init the camera
    	Luxe.camera.pos.set_xyz(0,4.2,7.35);
    	Luxe.camera.rotation.setFromEuler( new Vector( -35.0, 0, 0).radians() );

    	 // create the hud
    	 create_hud();

    	 // load/create the initial hand
    	 init_cards();
    			
		// load/create the game board
    	gameboard = new Gameboard();
    	gameboard.setup( function(_) {
    			// note to self: this gets called once for each
    			// thing the gameboard loads
    			loadCount++;
    		} );
    }

    function init_cards()
    {
    	// deal starting hand
    	while (hand.length < 7)
    	{
    		deal_card( hand.length );
    	}
    }

    function deal_card( handNdx : Int )
    {
    	var card = new Card( hud_batcher );
    	card.handNdx = handNdx;
    	card.returnToHand();
    	hand.push( card );
    }

    override function onkeyup( e:KeyEvent ) {

        if(e.keycode == Key.escape) {
            Luxe.shutdown();
        }

    } //onkeyup

    function checkHitCard( e:MouseEvent, action: Card -> Void )
    {
    	// take action for first hit card
        for (cardNdx in 0...hand.length)
        {
        	// right to left
        	// var card = hand[hand.length-(cardNdx+1)];

        	// left to right
        	var card = hand[cardNdx];

        	if (card.point_inside( e.pos ))
        	{
        		action( card );
        		break;
        	}
        }

    }

    override function onmousedown(e:MouseEvent) {
        mouse = e.pos;
        dragging = true;    

        checkHitCard(e, function ( c:Card ) {
        		c.fakemousedown( e );
        	});
    }

    override function onmouseup(e:MouseEvent) {

        mouse = e.pos;
        dragging = false;

        // Fakemouseup ALL the cards
        for (card in hand)
        {
        	card.fakemouseup(e);
        }

    } //onmouseup

    override function onmousemove(e:MouseEvent) {

        mouse = e.pos;
        if (dragging)
        {
        	for (c in hand)
        	{
        		if (c.lifted)
        		{
        			c.fakemousemove( e );
        		}
        	}
        }

        // tell the gameboard
        if (gameboard != null)
        {
        	gameboard.fakemousemove(e);
    	}

    } //onmousemove

    override function update(dt:Float) 
    {
    	if (loadCount >= 2)
    	{
    		loaded = true;
    	}
    	//var v = (mouse.y / Luxe.screen.h);

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
