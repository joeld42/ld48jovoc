
import luxe.Input;
import luxe.Mesh;
import luxe.Vector;
import luxe.Parcel;
import luxe.ParcelProgress;
import luxe.Color;
import luxe.Input;
import luxe.Text;
import luxe.utils.Maths;

import luxe.collision.ShapeDrawerLuxe;
import luxe.collision.Collision;
import luxe.collision.shapes.*;
import luxe.collision.data.*;

import phoenix.Camera;
import phoenix.geometry.Geometry;
import phoenix.Batcher;

import luxe.tween.Actuate;

class Bullet {
    public var mesh : Mesh;
    public var vel : Vector;

    public function new ( _mesh : Mesh ) {
        mesh = _mesh;
        vel = new Vector();
    }
}

class Enemy {
    public var mesh : Mesh;
    public var targetPos : Vector;
    public var goalPos : Vector;
    public var exitPos : Vector;
    public var exiting : Bool = false;
    public var health : Float = 10.0;
    public var shape : Circle;

    public function new ( _mesh : Mesh ) {
        mesh = _mesh;
        targetPos = new Vector();
        goalPos = new Vector();
        exitPos = new Vector();
        shape = new Circle( _mesh.pos.x, _mesh.pos.z, 0.4 );
    }
}

class Main extends luxe.Game {

    // Player stuff
    var inputLeft : Vector;
    var inputRight : Vector;
    var inputUp : Vector;
    var inputDown : Vector;
    var health : Int = 5;

    // Camera
    var cameraTarget : Vector;


    // Game World
    var walls : Array<Shape>;
    var enemyWalls : Array<Shape>;
    var playerShape : Shape;
    var grinderShape : Shape;
    var bullets : Array<Bullet>;
    var splatterMeshes : Array<Mesh>;
    var enemies : Array<Enemy>;
    var navPoints : Array<Vector>;

    var nextWaveTimeout : Float;
    var nextSpawnTimeout : Float;
    var spawnCount : Int;
    var spawnLoc : Vector;    


    // Geometry
    var playerDir : Vector;
    var strafeDir : Vector;
	var meshWorld : Mesh;	
	var meshPlayer : Mesh;
    var meshGrinder : Mesh;
    var meshBulletSrc : Mesh;
    var meshEnemySrc : Mesh;
    var firing : Bool = false;
    var shootyTimeout : Float;

    // Everything is ready
    var gameReadySemaphore : Int = 5;

    // Debug and HUD tools
    var hud_batcher:Batcher;
    var shape_batcher:Batcher;
    var shape_view : Camera;
    var drawer: ShapeDrawerLuxe;
    var desc : Text;

	override function config( config:luxe.AppConfig ) {

        config.render.depth_bits = 24;
        config.render.depth = true;
        config.render.antialiasing=8;

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
    						texture:texPlayer, onload: function (m:Mesh) {
                                    m.pos.set_xyz( 0.0, 0.0, 5.0 );
                                    playerShape.position.set_xy( m.pos.x, m.pos.z );
                                    meshloaded(m);
                                } });

        // Bullet
        var texBullet = Luxe.loadTexture('assets/peas.png');          
        meshBulletSrc = new Mesh({ file:'assets/ld32_foodfight_cube.obj', 
                            texture:texBullet, onload: function (m : Mesh ) {
                                m.geometry.visible = false;
                                meshloaded(m);
                             }});

        // Enemy
        var texEnemy = Luxe.loadTexture('assets/ld32_foodfight_tmp_enemy.png');          
        meshEnemySrc = new Mesh({ file:'assets/ld32_foodfight_tmp_enemy.obj', 
                            texture:texEnemy, onload: function (m : Mesh ) {
                                m.geometry.visible = false;
                                meshloaded(m);
                             }});

        // Grinder
        var texGrinder = Luxe.loadTexture('assets/ld32_foodfight_grinder.png');          
        meshGrinder = new Mesh({ file:'assets/ld32_foodfight_grinder.obj', 
                            texture:texGrinder, onload: function (m : Mesh ) {                                
                                meshloaded(m);
                             }});                            

        // Create the HUD
        create_hud();
    } 

    function create_hud() {

        //For the hud, it has a unique batcher, layer 4 is > the batcher_1, and the default(1)
        hud_batcher = Luxe.renderer.create_batcher({ name:'hud_batcher', layer:4 });

        desc = new Text({
            pos: new Vector(10,10),
            point_size: 18,
            batcher: hud_batcher,
            text: 'Hello there this is ludumdare'
        });

        // Shape batcher
        shape_batcher = Luxe.renderer.create_batcher({ name:'shape_batcher', layer:5 });

        shape_view = new Camera();  
        
        
        //shape_view.zoom = 0.9;

        trace( 'shape_view center is ${shape_view.center}\n');
        trace( 'shape_view pos is ${shape_view.pos}\n');
        trace( 'shape_view zoom is ${shape_view.zoom}\n');
        trace( 'shape_view viewport is ${shape_view.viewport}\n');
        shape_view.pos.set_xy( -shape_view.viewport.w/2.0, -shape_view.viewport.h/2.0 );
        shape_view.zoom = shape_view.viewport.h/25;
        shape_batcher.view = shape_view;
        drawer = new ShapeDrawerLuxe( {
                 batcher: shape_batcher
            });
    }

	override function ready() {

        inputLeft = new Vector();
        inputRight = new Vector();
        inputUp = new Vector();
        inputDown = new Vector();

        playerDir = new Vector( 0.0, 0.0, -1.0 );
        strafeDir = playerDir.clone();
        bullets = new Array<Bullet>();
        enemies = new Array<Enemy>();
        splatterMeshes = new Array<Mesh>();
        cameraTarget = new Vector();    

		var preload = new Parcel();    	
    	preload.add_texture( "assets/ld32_foodfight_env.png");
    	preload.add_texture( "assets/tmp_player.png");
        preload.add_texture( "assets/ld32_foodfight_tmp_enemy.png");
        preload.add_texture( "assets/peas.png");
        preload.add_texture( "assets/ld32_foodfight_grinder.png");

		preload.add_text( "assets/ld32_foodfight_env.obj" );
		preload.add_text( "assets/ld32_foodfight_tmp_player.obj" );
        preload.add_text( "assets/ld32_foodfight_tmp_enemy.obj" );
        preload.add_text( "assets/ld32_foodfight_cube.obj" );
        preload.add_text( "assets/ld32_foodfight_grinder.obj" );

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

        // Create the game world
        grinderShape = new Circle( 0.0, 0.0, 2.0 );
        walls = [
            // confusing: it's center, w, h
            Polygon.rectangle( -11.0, 0.0, 2.0, 25.0),
            Polygon.rectangle(  11.0, 0.0, 2.0, 25.0),
            Polygon.rectangle(  0.0, -11.0, 25.0, 2.0),
            Polygon.rectangle(  0.0, 11.0, 25.0, 2.0),
            
            grinderShape,

            // tables
            new Circle( -3.0, 5.5, 1.8 ),
            new Circle(  3.0, 5.5, 1.8 ),
            Polygon.rectangle(  0.0, -5.3, 7.2, 1.8),
            Polygon.rectangle(  6.7, 0.0, 1.8, 7.2 ),
            Polygon.rectangle(  -6.4, 0.0, 1.8, 7.2 )
        ];

        enemyWalls = walls.slice( 4 ); // enemy walls 
        enemyWalls.push( Polygon.rectangle( -5.56, -11, 9, 2.0 ));
        enemyWalls.push( Polygon.rectangle( 5.56, -11, 9, 2.0 ));
        enemyWalls.push( Polygon.rectangle( -5.56, 11, 9, 2.0 ));
        enemyWalls.push( Polygon.rectangle( 5.56, 11, 9, 2.0 ));

        enemyWalls.push( Polygon.rectangle( -11.0, -5.56, 2.0, 9.0 ));
        enemyWalls.push( Polygon.rectangle( -11.0,  5.56, 2.0, 9.0 ));        
        enemyWalls.push( Polygon.rectangle( 11.0, -5.56, 2.0, 9.0 ));
        enemyWalls.push( Polygon.rectangle( 11.0,  5.56, 2.0, 9.0 ));        

        playerShape = new Circle( 0.0, 0.0, 0.4 );

        navPoints = [
            new Vector(  0.0, -8.0 ),
            new Vector( -6.6, -7.2 ), 
            new Vector(  6.6, -7.2 ),
            new Vector( -8.7, 0.0 ),
            new Vector(  8.7, 0.0 ),
            new Vector( -4.6, -4.2 ),
            new Vector(  4.6, -4.2 ),
            new Vector(  0.0, -3.0 ),
            new Vector( -3.8,  0.0 ),
            new Vector(  3.8,  0.0 ),
            new Vector( -5.0, 4.0 ),
            new Vector(  5.0, 4.0 ),
            new Vector(  0.0, 3.0 ),
            new Vector( -6.6, 7.2 ), 
            new Vector(  6.6, 7.2 ),
            new Vector(  0.0, 8.4 )
        ];

        // init game
        resetGame();

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

    function spawnEnemy( )
    {
        // // DBG put bullets at nav points
        // for (np in navPoints) {
        //     var dot = cloneMesh( meshBulletSrc );
        //     dot.pos.set_xyz( np.x, 0.0, np.y );
        // }

        var enemy = new Enemy(cloneMesh( meshEnemySrc ));
        enemy.mesh.pos.copy_from( spawnLoc );
        enemy.exitPos.copy_from( randomSpawnPos() );
        enemy.goalPos.copy_from( meshPlayer.pos );
        updateEnemyTarget( enemy );
        enemies.push( enemy );
    }

    function fireUnconventionalWeapon( dt:Float )
    {
        trace('FIRE!');
        var bullet = new Bullet(cloneMesh( meshBulletSrc ));
        
        bullet.mesh.pos.set_xyz( meshPlayer.pos.x, 1.0, meshPlayer.pos.z );
        bullet.vel.set_xyz( strafeDir.x + Maths.random_float( -0.1, 0.1 ),
            strafeDir.y + Maths.random_float( -0.1, 0.1 ),
            strafeDir.z + Maths.random_float( -0.1, 0.1 ) );

        bullets.push( bullet );        

        // kick-back (doesn't properly consider collision but whatevers...)
        var kickVec = Vector.Multiply( bullet.vel, -0.2 );
        meshPlayer.pos.add( kickVec );
        meshPlayer.pos.y = 0.0;
        playerShape.position.set_xy( meshPlayer.pos.x, meshPlayer.pos.z );

        shootyTimeout = 0.1;

        Luxe.camera.shake( 0.1 );
    }

    override function update(dt:Float) {

        // Is everything initted?
        if (gameReadySemaphore > 0) {
            return;
        }

        // Collisions and movement
        var playerDead = false;
        var oldPlayerPos = meshPlayer.pos.clone();

        // update input dir (the 0.1 is for substeps)                       
        var inputDir = new Vector(
            (inputLeft.x + inputRight.x + inputUp.x + inputDown.x) * dt * 0.2,
            (inputLeft.y + inputRight.y + inputUp.y + inputDown.y) * dt * 0.2,
            (inputLeft.z + inputRight.z + inputUp.z + inputDown.z) * dt * 0.2 );

        for ( substep in 0...10)
        {
            // Try the new position for collision
            if ((substep % 2)==0) {
                // Even substep, move x
                playerShape.position.set_xy( playerShape.position.x + inputDir.x,
                                             playerShape.position.y  );
            } else {
                // odd substep, move y
                playerShape.position.set_xy( playerShape.position.x,
                                             playerShape.position.y + inputDir.z );
            }

            var collisions = Collision.shapeWithShapes( playerShape, walls);
            if (collisions.length>0) {

                // Collided, just use the first collider            
                var coll = collisions[0];
                var sep = coll.separation;

                // trace('COLLIDE: ${coll} ${sep} ${collisions.length}');

                if (coll.shape2 == grinderShape)
                {
                    // OOps.. hit the grinder.. u ded
                    playerDead = true;
                }

                var mover_separated_pos = new Vector( coll.shape1.position.x + sep.x*1.01, 
                                                      coll.shape1.position.y + sep.y*1.01 );

                playerShape.position.set_xy( mover_separated_pos.x, mover_separated_pos.y );
            }
            else 
            {
                // no collision here, use the new shape
                meshPlayer.pos.set_xyz( playerShape.position.x, 0.0, playerShape.position.y );
            }
        }

        // commit new player position
        playerShape.position.set_xy( meshPlayer.pos.x, meshPlayer.pos.z );

         // check if hit enemies
         for ( ee in enemies) {
            
            ee.shape.position.set_xy( ee.mesh.pos.x, ee.mesh.pos.z );

            var  coll = Collision.shapeWithShape( ee.shape, playerShape ) ;
            if (coll != null) {
                
                health--;
                Luxe.camera.shake( 1.0 );

                if (health <= 0) {
                    playerDead = true;
                }

                var sep = coll.separation;
                var knockbackDir = Vector.Subtract( meshPlayer.pos, ee.mesh.pos );

                meshPlayer.pos.add( knockbackDir );
                playerShape.position.set_xy( meshPlayer.pos.x, meshPlayer.pos.z );
            }
         }

        // Check for game over
        if (playerDead) {
            // TODO: need better splat effect
            Luxe.camera.shake( 3.0 );
            resetGame( );
        }

        // Update enemies
        var exitedEnemies : Array<Enemy> = new Array<Enemy>();

        for (ee in enemies) {
            var edir = Vector.Subtract( ee.targetPos, ee.mesh.pos );            

            var espeed = 5.0 * dt;
            var edist = edir.length;
            edir.normalize();

            edir.multiplyScalar( espeed );
            if (edist < espeed)
            {                
                // reached target, is this our goal?
                var goalDir = Vector.Subtract( ee.goalPos, ee.mesh.pos );
                if (goalDir.length < 0.1) {
                    trace('Reached goal pos...');
                    // Are we already seeking exit?
                    if (ee.exiting) {
                        exitedEnemies.push( ee );
                    } else {
                        // Not yet, make the exit our new goal
                        ee.exiting = true;
                        ee.goalPos.copy_from( ee.exitPos );    
                        
                        updateEnemyTarget(ee);
                    }
                    
                } else {
                    // No.. pick the closest reachable nav point
                    // to our goal
                    updateEnemyTarget(ee);
                }
                
            } else {
                ee.mesh.pos.add( edir ); 
            }
        }

        // clean up exited enemies
        for ( ee in exitedEnemies) {
            enemies.remove( ee );
            ee.mesh.destroy();            
        }

        // Spawn new enemies
        updateSpawns( dt );

        // calc player direction
        var testPlayerDir = new Vector( meshPlayer.pos.x - oldPlayerPos.x,
                                        meshPlayer.pos.y - oldPlayerPos.y,
                                        meshPlayer.pos.z - oldPlayerPos.z );
        if (testPlayerDir.lengthsq > 0.0) {
            playerDir.copy_from( testPlayerDir );
            playerDir.normalize();
        }

        // Shooty shooty
        if(Luxe.input.inputpressed('fire')) {
            trace('fire pressed in update');
            fireUnconventionalWeapon( dt );
            // spawnEnemy();
            firing = true;
        }

        if(Luxe.input.inputreleased('fire')) {
            trace('fire released in update');
            firing = false;
        }

        if (!firing) {     
            strafeDir.copy_from( playerDir );
        } else {    
            shootyTimeout -= dt;            
            if (shootyTimeout <= 0.0) {
                fireUnconventionalWeapon(dt);
            }
        }

        // Update bullets
        var expiredBullets = new Array<Bullet>();
        var bulletShape = new Circle( 0.0, 0.0, 0.25 );
        for ( b in bullets) {
            b.mesh.pos.set_xyz( b.mesh.pos.x + b.vel.x,
                b.mesh.pos.y + b.vel.y,
                b.mesh.pos.z + b.vel.z );

            if (b.mesh.pos.lengthsq > 250.0) {
                expiredBullets.push( b );
            } else {
                
                bulletShape.position.set_xy( b.mesh.pos.x, b.mesh.pos.z );

                // check enemies
                for (ee in enemies) {
                    // enemy shape was updated already above
                     if (Collision.shapeWithShape(ee.shape,bulletShape)!=null) {
                        trace("Hit enemy");
                        ee.health -= 12.0;
                     }
                }

                // check against walls
                var collisions = Collision.shapeWithShapes( bulletShape, walls);
                if (collisions.length>0) {

                    var coll = collisions[0];
                    var sep = coll.separation;
                    
                    b.mesh.pos.set_xyz(coll.shape1.position.x + sep.x*0.5, b.mesh.pos.y,
                                              coll.shape1.position.y + sep.y*0.5 );
                    expiredBullets.push( b );

                    if (coll.shape2 == grinderShape) {
                        b.mesh.destroy();
                    }
                }
            } 
        }

        for ( b in expiredBullets) {
            bullets.remove( b );

            // heheh.. looks cool to leave these
            splatterMeshes.push( b.mesh );            
        }

        // Check for any dead enemies
        var deceasedEnemies = new Array<Enemy>();
        for (ee in enemies) {
            if (ee.health <= 0.0) {
                deceasedEnemies.push(ee);
            }
        }
        if (deceasedEnemies.length > 0) {
            Luxe.camera.shake( 0.8 );
        }
        for (ee in deceasedEnemies) {            
            enemies.remove(ee);
            Actuate.tween( ee.mesh.scale, 0.3, { x : 0.0, y : 15.0, z : 0.0 } 
                ).onComplete( function() {
                        ee.mesh.destroy();
                    });
            
        }


        // Update camera pos        
        cameraTarget.set_xyz( meshPlayer.pos.x + (strafeDir.x * 3.0), 
                              meshPlayer.pos.y + 18.0,
                              meshPlayer.pos.z + (strafeDir.z * 3.0) + 7.5 );

        Luxe.camera.pos.set_xyz( Luxe.camera.pos.x + (cameraTarget.x-Luxe.camera.pos.x) * dt,
            Luxe.camera.pos.y + (cameraTarget.y-Luxe.camera.pos.y) * dt,
            Luxe.camera.pos.z + (cameraTarget.z-Luxe.camera.pos.z) * dt );

        
        // update hud
        desc.text = 'HP: ${health}';

    } //update

    function updateSpawns( dt : Float )
    {
        nextWaveTimeout -= dt;
        if (nextWaveTimeout <= 0.0) {
            nextWaveTimeout = 15.0;

            spawnCount = Maths.random_int( 8, 14 );
            nextSpawnTimeout = 0.0;            
            spawnLoc = randomSpawnPos(); // everyone in a wave starts from the same pos
        }

        if (spawnCount > 0 ) {
            nextSpawnTimeout -= dt;
            if (nextSpawnTimeout <= 0.0) {
                nextSpawnTimeout = 0.6;
                spawnCount--;

                spawnEnemy();
            }
        }
    }

    function updateEnemyTarget( ee : Enemy )
    {
    
        // is our goal reachable directly?
        var meshPos2d = new Vector( ee.mesh.pos.x, ee.mesh.pos.z );
        var goalPos2d = new Vector( ee.goalPos.x, ee.goalPos.z );

        // trace('UpdateEnemyTarget, goalPos is ${goalPos2d}\n');

        var goalRay = new Ray( meshPos2d, goalPos2d, false );

        var colls = Collision.rayWithShapes( goalRay, enemyWalls );
        if (colls.length==0) {
            // Goal reachable..
            // trace("Goal is directly reachable..\n");
            ee.targetPos.copy_from( ee.goalPos );
        } else {
            // trace("Goal is not reachable...\n");
            var bestDist = 999.0;
            for (np in navPoints) {
                var npDist = Vector.Subtract(  meshPos2d, np ).length;
                // Don't consider navPoints we're alreayd standing on
                if (npDist > 0.2) {
                    // trace('checking pos ${ meshPos2d} np ${np}\n');
                    var npRay = new Ray( meshPos2d, np, false );
                    var colls = Collision.rayWithShapes( npRay, enemyWalls );
                    if (colls.length==0)
                    {                        
                        var npGoalDist = Vector.Subtract( goalPos2d, np ).length;
                        // trace('is reachable, goalDist ${npGoalDist}\n');

                        if (npGoalDist < bestDist) {
                            bestDist = npGoalDist;
                            ee.targetPos.set_xyz( np.x, 0.0, np.y );
                        }
                    } 
                    // else {
                    //     var c = colls[0];                        
                    //     // trace('NOT RECAHABLE: ${c} \n');
                    //     trace('NOT RECAHABLE: \n');
                    // }
                }
            }
        }
    }

    function randomSpawnPos() : Vector {
        var i = Maths.random_int( 0, 3 );
        if (i==0) {
            return new Vector( 0.0, 0.0, -15.0 ); // top
        } else if (i==1) {
            return new Vector( 0.0, 0.0, 15.0 ); // bottom
        } else if (i==2) {
            return new Vector( -15.0, 0.0, 0.0 ); // left
        } else {
            return new Vector( 15.0, 0.0, 0.0 ); // right
        }
    }    

    override function onrender() {

        // Is everything initted?
        if (gameReadySemaphore > 0) {
            return;
        }

        // for (shape in enemyWalls) {
        //     drawer.drawShape(shape);
        // }

        // drawer.drawShape( playerShape );

    }

    function resetGame() {

        // general init
        health = 5;
        spawnCount = 0;
        nextWaveTimeout = 5.0; // timeout for first wave

        if (gameReadySemaphore > 0) {
            return;
        }

        // below here, init that depends on stuff loaded

        while (bullets.length > 0) {
            var b = bullets.pop();
            b.mesh.destroy();
        }

        while (splatterMeshes.length > 0) {
            var m = splatterMeshes.pop();
            m.destroy();
        }

        while (enemies.length > 0) {
            var ee = enemies.pop();
            ee.mesh.destroy();
        }

        meshPlayer.pos.set_xyz( 0.0, 0.0, 5.0 );
        playerShape.position.set_xy( meshPlayer.pos.x, meshPlayer.pos.z );
    }

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
