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
import luxe.utils.Maths;
import luxe.tween.Actuate;

import phoenix.Batcher;
import phoenix.geometry.Geometry;
import phoenix.geometry.TextGeometry;

import Gameboard;
import Card;
import Tower;
import Creep;

class Main extends luxe.Game {

	var hud_batcher:Batcher;
    var mouse : Vector;
    var dragging : Bool = false;
	
	var gameboard : Gameboard;

	var loadCount : Int = 0;	
	var loaded : Bool = false;

	var cardTopNames : Array<String>;
	var cardFlipNames : Array<String>;

	var hand : Array<Card>;

	var cachedMeshes : Map<String,Mesh>;

	// status text
	var statusText : TextGeometry;
	var currentStatus : String;

	// Health text
	var health : Int;
	var healthText : TextGeometry;
	var hitMask : Sprite;

    override function config( config:luxe.AppConfig ) {

        config.render.depth = true;
        config.render.depth_bits = 24;
        config.render.antialiasing=8;
        return config;
    }

    override function ready() {

    	mouse = new Vector();
    	hand = new Array<Card>();
    	cachedMeshes = new Map<String,Mesh>();
    	cardTopNames = [ "snowman", "rock"];
    	cardFlipNames = [ "critter" ];

		Luxe.renderer.clear_color = new Color().rgb( 0x1b383c );
    	var preload = new Parcel();

    	preload.add_texture( "assets/gameboard_5x5.png");
    	preload.add_texture( "assets/card_snowman.png");
    	preload.add_texture( "assets/card_rock.png");
    	preload.add_texture( "assets/card_critter.png");
    	preload.add_texture( "assets/testgrid.png");
    	preload.add_texture( "assets/snowman.png");
    	preload.add_texture( "assets/rock.png");
    	preload.add_texture( "assets/critter.png");
    	preload.add_texture( "assets/hit_mask.png");

		preload.add_text( "assets/gameboard_5x5.obj", true);
		preload.add_text( "assets/snowman.obj", true);
		preload.add_text( "assets/rock.obj", true);
		preload.add_text( "assets/cursor.obj", true );
		preload.add_text( "assets/critter.obj", true );

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
            cull_backfaces: true,            
            depth_test : true
    	});

    	// init the camera
    	Luxe.camera.pos.set_xyz(0,4.2,7.35);
    	Luxe.camera.rotation.setFromEuler( new Vector( -35.0, 0, 0).radians() );

    	// create the hud
    	create_hud();

    	// hit mask    	
    	hitMask = new Sprite({
	            name: 'hitmask',	            
	            texture: Luxe.loadTexture('assets/hit_mask.png'),
	            batcher : hud_batcher,
	            pos: new Vector( Luxe.screen.w/2, Luxe.screen.h/2 ),
	            size: new Vector( Luxe.screen.w, Luxe.screen.h ),
	            depth: 1
			});
    	hitMask.visible = false;
    			
    	// init tower meshes
    	init_meshes();

		// load/create the game board
    	gameboard = new Gameboard();
    	gameboard.setup( function(_) {
    			// note to self: this gets called once for each
    			// thing the gameboard loads
    			loadCount++;
    		} );

    	// reset level
        reset_game();

        // trace( 'SCREEN: ${Luxe.screen.w} ${Luxe.screen.h}');
    }

    function reset_game()
    {
    	// load/create the initial hand
    	init_cards();

    	// reset health
    	health = 20;
    	update_health();
    }

    function update_health()
    {
    	healthText.text = 'HP: ${health}';
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
    	var topName = cardTopNames[ Maths.random_int( 0, cardTopNames.length -1 )];
    	var flipName = cardFlipNames[ Maths.random_int( 0, cardFlipNames.length -1 )];
    	var card = new Card( topName, flipName, hud_batcher );
    	card.handNdx = handNdx;
    	card.returnToHand();
    	hand.push( card );
    }

	// just handles removing the card from the hand and stuff    
    function play_card( card : Card )
    {
    	hand.remove( card );
    	card.destroyAll();

    	// re-index hand Indexes
    	var ndx : Int = 0;
    	for (c in hand)
    	{
    		c.handNdx = ndx;
    		ndx++;
    	}
    }

    function init_meshes()
    {
    	// Load the src meshes for the tower objects
    	var srcMeshes: Array<String> = [
    		"snowman", "rock", "critter"
    	];

    	for (meshName in srcMeshes)
    	{
    		// Load the tower resources
			var tex = Luxe.loadTexture('assets/${meshName}.png');     
        	tex.clamp = repeat;  
        	tex.onload = function(t) 
        	{
            	new Mesh({ file:'assets/${meshName}.obj', 
                	      texture: t, 
                    	  onload : function ( m : Mesh ) {                            
                            
                            m.geometry.visible = false;
                            cachedMeshes[meshName] = m;
                            
                            trace('Loaded ${meshName}');
                            loadCount++;
                        }
                      });
        	}
    	}
    }

    function build_tower( placingCard : Card )
    {
        var tname = placingCard.topname;
		var towerMeshSrc = cachedMeshes[ tname ];
		
		var towerEnt = new Tower( placingCard.topname, cloneMesh( towerMeshSrc ) );
		towerEnt.mesh.pos.copy_from(gameboard.ghost.pos);

		// animate in
		towerEnt.mesh.pos.y = -2.0;
		Actuate.tween( towerEnt.mesh.pos, 1.0, { y : 0.0 } );

		gameboard.buildTower( gameboard.cursorBoardX,gameboard.cursorBoardY,towerEnt);

		play_card( placingCard );

		setStatusText('Built ${tname}.');
    }

    function spawn_creep( placingCard : Card )
    {
        var cname = placingCard.flipname;
		var creepMeshSrc = cachedMeshes[ cname ];
		if (creepMeshSrc != null)
		{			
			var creepEnt = new Creep( placingCard.flipname, cloneMesh( creepMeshSrc ) );					
			creepEnt.gameboard = gameboard;
			creepEnt.playerHit = creep_hit;

			// start in the creep cave 
			creepEnt.mesh.pos.set_xyz( 0.0, 0.0, -4.2 );
			creepEnt.setGridTarg( 2, 5 );  // Move onto the open row
		}

    }

    function creep_hit( creep : Creep )
    {
    	setStatusText( 'Hit by ${creep.creepName}! ',  new Color().rgb( 0xff0000) );

    	health -= 5;
    	update_health();

    	Luxe.camera.shake( 0.5 );

    	hitMask.visible = true;
    	hitMask.color.a = 1.0;

    	Actuate.tween( healthText.color, 0.25, { g : 1.0})
    				.reverse()
    				.repeat(5);

    	Actuate.tween( hitMask.color, 0.3, { a : 0.0 } )
    			.delay(0.3)
    			.onComplete( function() {
    					hitMask.visible = false;

    					// var sx = healthText.transform.scale.x;
    					// var sy = healthText.transform.scale.y;
    					// healthText.transform.scale.set_xy(sx*2, sy*2);
    					// Actuate.tween( healthText.transform.scale, 
    					// 	0.25, 
    					// 	{ x : sx, y : sy });
    				});

    	creep.mesh.destroy();
    	creep.destroy();
    }

    function cloneMesh( mesh : Mesh ) : Mesh
    {
        var mesh2 = new Mesh({
            geometry : new Geometry({
            batcher : Luxe.renderer.batcher,
            immediate : false,
            primitive_type: PrimitiveType.triangles,
            texture: mesh.geometry.texture
            })
        });

        for(v in mesh.geometry.vertices) {
            mesh2.geometry.add( v.clone() );
        }

        return mesh2;
    }

    // ====================================================================
    // Input handling
	// ====================================================================

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
        if (e.button == MouseButton.left)
        {
     	   dragging = true;    
    	}

        checkHitCard(e, function ( c:Card ) {
        		c.fakemousedown( e );
        	});
    }

    override function onmouseup(e:MouseEvent) {

        mouse = e.pos;

        // Fakemouseup ALL the cards
        var placingCard = null;
        for (card in hand)
        {
        	if (card.placing)
        	{
        		placingCard = card;
        	}
        	// clear the placing
        	card.fakemouseup(e);
        }

        if (e.button == MouseButton.left)
        {
        	// if we're placing, build Tower entity
        	if ((placingCard != null) && (gameboard.cursorBoardX > -1))
        	{
        		var cell = gameboard.cursorCell;
        		if (!cell.creeptarg)
        		{
					if (gameboard.canBuildHere(cell.gx, cell.gy))
					{
	        			build_tower( placingCard );
	        			spawn_creep( placingCard );
	        		}
        		}
        	}

        	// clear the ghost
        	gameboard.update_ghost( false );
        	dragging = false;

        	if (gameboard.ghost != null)
        	{
        		gameboard.ghost.destroy();
        		gameboard.ghost = null;
        	}
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

        			// Also draw the placement ghost if we're building
        			if ((c.placing) && (gameboard != null))
        			{
        				if (gameboard.ghost == null)
        				{
        					var ghostMeshSrc = cachedMeshes[ c.topname ];
        					gameboard.ghost = cloneMesh( ghostMeshSrc );
        				}

        				gameboard.update_ghost( true );

        				if (gameboard.cursorBoardX != -1)
        				{
        					var cell = gameboard.cursorCell;
        					if (cell.tower != null)
        					{
        						setStatusText( 'Already a ${cell.tower.name} built there...', new Color().rgb( 0xff0000) );
        					}
        					else if (cell.creeptarg)
        					{
        						setStatusText( "There's something in the way...", new Color().rgb( 0xff0000) );	
        					}
        					else if (!gameboard.canBuildHere(cell.gx, cell.gy))
        					{
        						setStatusText( "Home row must be reachable.", new Color().rgb( 0xff0000) );	
        					}
        					else
        					{
        						setStatusText( 'Place the ${c.topname} on an empty space.' );	
        					}
        				}
     	   			}
        		}
        	}
        }
    	

        // tell the gameboard
        if (gameboard != null)
        {
        	gameboard.fakemousemove(e);
    	}

    } //onmousemove


    // ====================================================================
    // Update
	// ====================================================================


    override function update(dt:Float) 
    {
    	if (loadCount >= 4)
    	{    		
    		loaded = true;
    	}

    	// trace('loadCount ${loadCount}');
    	//var v = (mouse.y / Luxe.screen.h);

    } //update

    function setStatusText( text : String, ?_color:Color = null )
    {
    	if (_color == null)
    	{
    		_color = new Color().rgb(0x377178);
    	}

    	if (text != currentStatus)
    	{
	    	currentStatus = text;
	    	statusText.text = text;

	    	// set text color, and fade out
	    	statusText.color = _color;
	    	Actuate.tween( statusText.color, 3.0, { a : 0.0 } ).delay(4.0);
    	}	
    }

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

        statusText = Luxe.draw.text({
            text : "",
            point_size : small_amount * 0.55,
            bounds : new Rectangle(small_amount/2, Luxe.screen.h - small_amount, Luxe.screen.w, small_amount),
            color : new Color().rgb(0x377178),
            batcher : hud_batcher,
            align_vertical : TextAlign.center
        });
        
        healthText = Luxe.draw.text({
            text : "",
            point_size : 20,
            bounds : new Rectangle( Luxe.screen.w - 100, Luxe.screen.h - small_amount, 
            						100, small_amount),
            color : new Color().rgb(0xc23c21),
            batcher : hud_batcher,
            align_vertical : TextAlign.center
        });

        setStatusText('Ludum Dare #31 Game -- Jovoc (joeld42@gmail.com)');
    } //create_hud


} //Main
