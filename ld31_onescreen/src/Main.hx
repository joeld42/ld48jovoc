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
import phoenix.Texture;
import phoenix.geometry.Geometry;
import phoenix.geometry.TextGeometry;
import phoenix.Shader;
import hxsw.HXSW;
import snow.render.opengl.GL;

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

	// Shaders
	var glsw : HXSW;
	var baseShader : Shader;
	var ghostShader : Shader;
	var tval : Float = 0.0;

	var hand : Array<Card>;

	var cachedMeshes : Map<String,Mesh>;

	var cardBackTex : Texture;

	// status text
	var statusText : TextGeometry;
	var currentStatus : String;

	// Health text
	var health : Int;
	var healthText : TextGeometry;
	var hitMask : Sprite;
	var gameOver : Sprite;
	var playAgain : Sprite;

	// Creeps
	var creeps : Array<Creep>;

    override function config( config:luxe.AppConfig ) {

        config.render.depth = true;
        config.render.depth_bits = 24;
        config.render.antialiasing=8;
        return config;
    }

    override function ready() {

    	mouse = new Vector();
    	hand = new Array<Card>();
    	creeps = new Array<Creep>();

    	cachedMeshes = new Map<String,Mesh>();
    	cardTopNames = [ "snowman", "rock", "sring", "sstone"];
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
    	preload.add_texture( "assets/victory.png");
    	preload.add_texture( "assets/bosscreep.png");
    	preload.add_texture( "assets/hit_mask.png");
    	preload.add_texture( "assets/gameover.png");
    	preload.add_texture( "assets/bullet.png");
    	preload.add_texture( "assets/sring.png");
    	preload.add_texture( "assets/sstone.png");
    	preload.add_texture( "assets/cardback.png");

		preload.add_text( "assets/gameboard_5x5.obj", true);
		preload.add_text( "assets/snowman.obj", true);
		preload.add_text( "assets/rock.obj", true);
		preload.add_text( "assets/cursor.obj", true );
		preload.add_text( "assets/critter.obj", true );
		preload.add_text( "assets/victory.obj", true );
		preload.add_text( "assets/bosscreep.obj", true );
		preload.add_text( "assets/sring.obj", true );
		preload.add_text( "assets/sstone.obj", true );

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

    	// load the shaders
    	init_shaders();

    	// var v = new Vector( 1.0, 0.2, 0.5);
    	// v.normalize();
    	// var c = new Vector(1.0, 0.5234375, 0.23046875);
    	// c.multiplyScalar( 0.2 );
    	// trace('v ${v.x}, ${v.y}, ${v.z}   c ${c.x}, ${c.y}, ${c.z}' );

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

    	// Load the card back
    	cardBackTex = Luxe.loadTexture( "assets/cardback.png");
    	cardBackTex.generate_mipmaps();
    	cardBackTex.filter = FilterType.mip_linear_linear;

    	 // set up group for batcher
    	 Luxe.renderer.batcher.add_group(1,

    		// pre-render
            function(b:Batcher){
                Luxe.renderer.blend_mode(BlendMode.src_alpha, BlendMode.one);
                // Luxe.renderer.state.disable( GL.DEPTH_TEST );
            },

            // post-render
            function(b:Batcher){
                 Luxe.renderer.blend_mode(BlendMode.src_alpha, BlendMode.one_minus_src_alpha);
                 // Luxe.renderer.state.enable( GL.DEPTH_TEST );
            }
        );

    	 // connect events
    	 connect_events();

    	// init tower meshes
    	init_meshes();

		// load/create the game board
    	gameboard = new Gameboard();
    	gameboard.setup( function(_) {
    			// note to self: this gets called once for each
    			// thing the gameboard loads
    			loadCount++;
    		}, baseShader );

    	// reset level
        reset_game();

        // trace( 'SCREEN: ${Luxe.screen.w} ${Luxe.screen.h}');
    }

    function init_shaders()
    {
    	        // Load shaders
        glsw = new HXSW({
            path : './assets/',
            ext : '.glsl',
            logging : false,
            load_effect : load_effect
        });

        
        var prefix : String = "";

        // Uncomment this when targeting web
        prefix = "precision mediump float;\n\n";

        var vsid = "leviathan.base.vertex";
        var vert = glsw.get( vsid );        

		var fsid = "leviathan.base.fragment";
        var frag = prefix + glsw.get(fsid);       

        baseShader = new Shader( Luxe.resources );
		baseShader.id = "shader.base" + Luxe.utils.uniqueid();
		baseShader.from_string( vert, frag, vsid, fsid, false );

		 var fsid_ghost = "leviathan.ghost.fragment";		 
         var frag_ghost = prefix + glsw.get(fsid_ghost);

		ghostShader = new Shader( Luxe.resources );
		ghostShader.id = "shader.ghost" + Luxe.utils.uniqueid();
		ghostShader.from_string( vert, frag_ghost, vsid, fsid_ghost, true );        
    }

    function connect_events()
    {
    	Luxe.events.listen('tower.shoot', onTowerShoot );
    	Luxe.events.listen('tower.spawncard', onTowerSpawnCard );

    	Luxe.events.listen('creep.killed', onCreepKilled );
    }

    function reset_game()
    {
    	trace(">>> RESET GAME ");

    	// clear game over button if present
    	if (gameOver != null)
    	{
    		gameOver.destroy();
    		playAgain.destroy();
    		gameOver = null;
    		playAgain = null;
    	}

    	// trace('hand len ${hand.length}');
    	while (hand.length > 0)
    	{   
    		var c = hand.pop();    		
    		//trace('Reset: hand len is ${hand.length} card is ${c.name}'); 		

    		c.destroyAll();
    		c.destroy();
    	}

    	// reset board
    	gameboard.clearBoard();

    	// load/create the initial hand
    	init_cards();

    	// reset health
    	health = 20;
    	update_health();
    }

    function update_health()
    {
    	if (health <= 0)
    	{
    		health = 0;
    		game_over();
    	}
    	healthText.text = 'HP: ${health}';
    }

    function game_over()
    {
    	gameOver = new Sprite({
	            name: 'gameover',	            
	            texture: Luxe.loadTexture('assets/gameover.png'),
	            batcher : hud_batcher,
	            pos: new Vector( Luxe.screen.w/2, Luxe.screen.h/2 ),
	            rotation_z : -5.0
			});
    	Actuate.tween( gameOver, 3.0, { rotation_z : 5.0 })
    			.reflect()
    			.repeat()
    			.ease(luxe.tween.easing.Cubic.easeInOut);

    	playAgain = new Sprite({
	            name: 'playagain',	            
	            texture: Luxe.loadTexture('assets/again.png'),
	            batcher : hud_batcher,
	            pos: new Vector( Luxe.screen.w/2, (Luxe.screen.h/2) + 100 ),
	            depth : 10
			});    		

    	// clear out the creeps now
		while (creeps.length > 0)
    	{
    		var c = creeps.pop();
    		c.mesh.destroy();
    		c.destroy();    		
    	}
		
    }

    function init_cards()
    {
    	// get one victory card
    	deal_victory_card( 0 );

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

    function deal_victory_card( handNdx : Int )
    {
    	var card = new Card( "victory", "bosscreep", hud_batcher );
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
    		"snowman", "rock", "critter", "bosscreep", "victory",
    		"sring", "sstone"
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
		towerEnt.mesh.geometry.shader = baseShader;
		towerEnt.mesh.pos.copy_from(gameboard.ghost.pos);

		// animate in
		// towerEnt.mesh.pos.y = -2.0;
		towerEnt.mesh.scale.set_xyz( 0.01, 0.01, 0.01 );
		Actuate.tween( towerEnt.mesh.scale, 1.0, { x : 1.0, y : 1.0, z : 1.0 } )
					.ease(luxe.tween.easing.Cubic.easeInOut);

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
			creepEnt.mesh.geometry.shader = baseShader;
			creepEnt.gameboard = gameboard;
			creepEnt.playerHit = creep_hit;

			// start in the creep cave 
			creepEnt.mesh.pos.set_xyz( 0.0, 0.0, -4.2 );
			creepEnt.setGridTarg( 2, 5 );  // Move onto the open row

			creeps.push( creepEnt );
		}

    }

    function creep_hit( creep : Creep )
    {
    	setStatusText( 'Hit by ${creep.creepName}! ',  new Color().rgb( 0xff0000) );
    	trace('Hit ${creep.creepName} HP ${creep.health}');

    	health -= creep.damage;
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

    	creeps.remove( creep );
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

	function onTowerSpawnCard( tower : Tower )
	{
		var cardY : Float;
		if (tower.towerName=='sring')
		{
			cardY = 0.35;
		}
		else
		{
			cardY = 0.7;
		}
		var cardPickup = new Sprite({
	    		name: 'cardspawn',
	    		name_unique : true,
	    		texture: cardBackTex,
	    		pos: new Vector( tower.mesh.pos.x, -0.5, tower.mesh.pos.z ),
	    		size: new Vector( 0.4, 0.4 ),
	    		depth: 10, // high depth makes sprites draw last
	    		rotation_z : -5.0
	    		//group: 1
    		});
		Actuate.tween( cardPickup.pos, 1.5, { y : cardY } );
		Actuate.tween( cardPickup, 1.0, { rotation_z : 5.0 })
    			.reflect()
    			.repeat()
    			.ease(luxe.tween.easing.Cubic.easeInOut);

    	tower.spawnCard = cardPickup;
	}

	function onTowerShoot( tower : Tower )
	{
		// trace('Main: got tower shoot event ${tower.name}');

    	// find closest enemy
    	var closestEnemy : Creep = null;
    	var closestDist = -1.0;
    	for (c in creeps)
    	{
    		var cd = Vector.Subtract( tower.mesh.pos, c.mesh.pos );
    		var d = cd.length;
    		if ((d < tower.shootRange) && ((closestDist < 0.0) || (d < closestDist)))
    		{
    			closestEnemy = c;
    			closestDist = d;
    		}
    	}

    	if (closestEnemy != null)
    	{

		 	// Bullet
    		var bullet = new Sprite({
	    		name: 'bullet',
	    		name_unique : true,
	    		texture: Luxe.loadTexture('assets/bullet.png'),
	    		pos: tower.mesh.pos.clone(),
	    		size: new Vector( 0.4, 0.4 ),
	    		depth: 10, // high depth makes bullets draw last
	    		group: 1
    		});
    		
    		bullet.pos.y = 1.0;
    		Actuate.tween( bullet.pos, closestDist / tower.bulletSpeed, 
    				{ x : closestEnemy.targetPos.x, 
    			    	y : closestEnemy.mesh.pos.y + 0.5, 
    			    	z : closestEnemy.targetPos.z })
    			.onComplete( function(){
    					closestEnemy.takeDamage( tower.shootDamage );
    					bullet.destroy();
    				});

    	}
    	// else
    	// {
    	// 	trace("no enemies in range...");
    	// }
	}

	function removeCreep( creep : Creep )
	{		
		var creepCell = gameboard.cell( creep.targetX, creep.targetY );
		if (creepCell != null)
		{
			creepCell.creeptarg = false;
		}
    	creeps.remove( creep );
    	creep.mesh.destroy();
    	creep.destroy();		
	}

	function onCreepKilled( creep : Creep )
	{
		// TODO: some kind of feedback or explosion
		removeCreep(creep);
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

    	if (gameOver != null)
    	{
    		if (playAgain.point_inside(e.pos))
    		{
    			// reset!
    			reset_game();
    		}
    		return;
    	}

        mouse = e.pos;
        if (e.button == MouseButton.left)
        {
     	   dragging = true;    
    	}

    	// Check to see if we hit a spawn card icon
    	for (c in gameboard.cells)
    	{
    		if ((c.tower!=null) && (c.tower.spawnCard!=null))
    		{
    			var sc = c.tower.spawnCard;
    			var cardPos = Luxe.camera.world_point_to_screen( sc.pos );
    			var cardRect = new Rectangle( cardPos.x - 20, cardPos.y - 30, 40, 60 );
    			if (cardRect.point_inside(e.pos))
    			{
    				if (hand.length < 7)
    				{
	    				// trace('Hit card...');	    				
	    				c.tower.spawnCard = null;
	    				deal_card( hand.length );

						// size: new Vector( 0.4, 0.4 ),
	    				Actuate.tween( sc.size, 0.5, { x : 0.8, y : 0.8 } );
	    				Actuate.tween( sc.color, 0.6, { a : 0.0 } )
	    						.onComplete( function() {
	    								sc.destroy();
	    							});
	    				// sc.destroy();
    				}
    				else
    				{
    					setStatusText( "Hand Limit Reached.", new Color().rgb( 0xff0000) );		
    				}
    			}
    			// trace('spawnCard at ${cardPos.x} ${cardPos.y}');

    		}
    	}

        checkHitCard(e, function ( c:Card ) {
        		c.fakemousedown( e );
        	});
    }

    override function onmouseup(e:MouseEvent) {

		if (gameOver != null)
    	{
    		return;
    	}

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
					if (gameboard.canBuildHere(placingCard.topname, cell.gx, cell.gy))
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


    	if (gameOver != null)
    	{
    		return;
    	}

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
        					gameboard.ghost.geometry.shader = ghostShader;
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
        					else if (!gameboard.canBuildHere( c.topname, cell.gx, cell.gy))
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
    	if (ghostShader != null)
    	{
    		tval += dt;
    		// ghostShader.set_float( "tval", tval );
    		ghostShader.set_float( "tval", tval );
    		//ghostShader.set_vector2( "tval", new Vector( tval, tval) );
    	}

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

    // load effect for shaders
    function load_effect( path:String ) {
        trace( '>>> load_effect: ${path}');
        return Luxe.loadText( path ).text;
    }


} //Main
