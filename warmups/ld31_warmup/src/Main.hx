
import luxe.Input;
import luxe.Mesh;
import luxe.Color;

import luxe.Vector;
import luxe.Quaternion;
import luxe.Color;
import luxe.Text;
import luxe.Rectangle;

import phoenix.Batcher;
import phoenix.geometry.Geometry;
import phoenix.geometry.Vertex;
import phoenix.geometry.TextureCoord;

import luxe.Parcel;
import luxe.ParcelProgress;

import Brick;

class Main extends luxe.Game {

    var hud_batcher:Batcher;
    var mouse : Vector;
    var dragging : Bool = false;
	
    var loadCount : Int = 0;
    var didSceneInit : Bool = false;

	var brick : Mesh;
    var valis : Mesh;
    var ball : Mesh;

    // game stuff
    var score: Int = 0;

    // ball movement
    var ballMoving: Bool = false;    
    var ballAxis: Vector;
    var ballDir: Vector;

    var bricks : Array<Brick>;
    var valisSpin : Float = 0.0;

    override function config( config:luxe.AppConfig ) {

        config.render.depth = true;
        config.render.depth_bits = 24;
        config.render.antialiasing=8;
        return config;
    }

    override function ready() {

        bricks = new Array<Brick>();

        ballAxis = new Vector( 0.0, 1.0, 0.0 );

    	Luxe.renderer.clear_color = new Color().rgb( 0x333333 );
    	var preload = new Parcel();

    	preload.add_texture( "assets/brick.png");
        preload.add_texture( "assets/valis.png");
		preload.add_text( "assets/brick1.obj", true);
        preload.add_text( "assets/valis.obj", true);
        preload.add_text( "assets/ball.obj", true);        
		
		new ParcelProgress({
            parcel      : preload,
            background  : new Color(1,1,1,0.85),
            oncomplete  : assets_loaded
        });

        preload.load();
        connect_input();

    } //ready

 	function assets_loaded( _ )
    {
        trace("Assets loaded...");
        Luxe.camera.view.set_perspective({
    		far:1000,
            near:0.1,
            aspect : Luxe.screen.w/Luxe.screen.h,
            // cull_backfaces: true,            
            // depth_test : true
    	});

    		//move up and back a bit
    	Luxe.camera.pos.set_xyz(0,0.5,20);

    	var tex = Luxe.loadTexture('assets/brick.png');    	
    	tex.clamp = repeat;  
    	tex.onload = function(t) 
    	{
            trace("in tex onload" );

            // load the master brick
    		new Mesh({ file:'assets/brick1.obj', 
                      texture: t, 
                      //batcher:batcherScene, 
                      onload : function ( m : Mesh ) {
                            trace("in mesh onload" );
                            //trace("Main, Mesh loaded, loading shader");
                            //cshad = Luxe.loadShader('assets/cust_frag.glsl');
                            //m.geometry.shader = cshad;

                            //testManyMeshes( m, batcherScene );

                            loadCount++;
                            brick = m;

                            // place it offscreen
                            brick.pos.set_xyz( -100, -100, -100 );

                             init_board();
                        }
                      });

            // load the ball
            new Mesh({
                    file:'assets/ball.obj',
                    texture: t,
                    onload : function ( m : Mesh ) {
                        loadCount++;
                        ball = m;
                   } 
                });
		}

        var tex2 = Luxe.loadTexture('assets/valis.png');     
        tex2.clamp = repeat;  
        tex2.onload = function(t) 
        {
            new Mesh({ file:'assets/valis.obj', 
                      texture: t, 
                      onload : function ( m : Mesh ) {
                            
                            valis = m;
                            loadCount++;

                            valis.pos.set_xyz( 0.0, -5.0, 0.0 );
                        }
                      });
        }

        

        create_hud();
	}

    function init_board()
    {
        for (row in 0...3)
        {
            var num=10;
            var offs= 0.0;
            if (row==1)
            {
                num=11;
                offs = -0.5;
            }
            for (i in 0...num)
            {
                var b = new Brick({ name:'brick${20*row+i}'});
                b.mesh = cloneMesh(brick);
                b.mesh.pos.set_xyz( -7.515 + offs + (i*1.53), 4.5 + (row * 1.1), 0.0 );
                bricks.push(b);
            }
        }

        brick.pos.set_xyz( -100, -100, -100 );
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

    function scene_init()
    {
        // Hide the "master brick" mesh
        brick.pos.set_xyz( -100, -100, 0 );
        valis.pos.set_xyz( 0.0, -5.0, 0.0 );

        didSceneInit = true;
    }

    override function onkeyup( e:KeyEvent ) {

        if(e.keycode == Key.escape) {
            Luxe.shutdown();
        }

    } //onkeyup

    function connect_input() 
    {

        //here, we are going to bind A/left and D/right into a single named
        //input event, so that we can keep our movement code the same when changing keys

        Luxe.input.add('left', Key.left);
        Luxe.input.add('left', Key.key_a);

        Luxe.input.add('right', Key.right);
        Luxe.input.add('right', Key.key_d);

        Luxe.input.add('fire', Key.key_w);
        Luxe.input.add('fire', Key.up);
        Luxe.input.add('fire', Key.space);
        Luxe.input.add('fire', MouseButton.left);

    } //connect_input

    
    override function oninputup( _input:String, e:InputEvent ) {
        trace( 'named input up : ${_input}' );
        if (_input=="fire")
        {
            trace("fire...");
            if (ballMoving == false)
            {
                trace("launching ball...");
                ballMoving = true;
                ballDir = new Vector( random_1_to_1(), 1.0, 0.0 );
                ballDir.normalize();

                ballAxis = new Vector( Math.random(), Math.random(), Math.random() );
                ballAxis.normalize();
                trace( 'ball moving! dir ${ballDir}' );
            }            
        }
    } //oninputdown

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

    function checkHitBricks( newPos : Vector ) : Bool
    {
        // check for collide with bricks
        // FIXME: figure out how to do this with Entity stuff, this is a mess
        for (b in bricks)
        {
            if (!b.alive)
            {
                continue;                
            }

            var rectDist = new Vector( Math.max( Math.abs(newPos.x - b.mesh.pos.x) - 1.5, 0.0 ),
                                       Math.max( Math.abs(newPos.y - b.mesh.pos.y) - 1.0, 0.0 ), 0.0 );

            // trace( 'brick pos ${b.pos} newPos ${newPos} rectDist ${rectDist.length}');

            if (rectDist.lengthsq < 0.09) // 0.3^2
            {
                // hit brick
                b.alive = false;
                b.mesh.pos.x = -100;
                score += 1;
                return true;

                // TODO: destroy brick more elegantly
            }            
        }
        return false;
    }



    override function update(dt:Float) {

        if (loadCount >= 2)
        {
            if (!didSceneInit)
            {
                scene_init();
            }

            var move_speed = 30.0;
            var ball_speed = 20.0;
            var moving = false;
            if(Luxe.input.inputdown('left')) 
            {
                valis.pos.x -= move_speed * dt;            
                moving = true;
            } else if(Luxe.input.inputdown('right')) {
                valis.pos.x += move_speed * dt;
                moving = true;
            } //left/right

            if ((!moving) && (dragging))
            {
                valis.pos.x = (mouse.x -  Luxe.screen.w/2) / 60.0;
            }

            // update ball pos
            if (!ballMoving)
            {
                ball.pos.set_xyz( valis.pos.x, valis.pos.y + 1,  valis.pos.z );
            }
            else
            {
                var hitbrick : Bool = false;

                var vel = ballDir.clone();
                var newPos =ball.pos.clone();
                vel.multiplyScalar( ball_speed * dt );
                
                // update vert
                newPos.y += vel.y;

                if (checkHitBricks(newPos))
                {
                    // bounce y
                    ballDir.y = -ballDir.y;   
                }
                // Check for bounce with valis
                else if ((newPos.y < -4.7) && (Math.abs(newPos.x - valis.pos.x) < 2.5))
                {
                    // bounce y
                    ballDir.y = -ballDir.y;   
                }
                // Check for collide with walls
                else if (newPos.y < -8.0)
                {
                    // You lose! reset the ball..
                    // TODO: SFX Lose
                    ballMoving = false;                    
                }
                else if (newPos.y > 8.0)
                {
                    ballDir.y = -ballDir.y;
                    // TODO: SFX Bounce
                }
                else
                {
                    ball.pos.y = newPos.y;
                }

                // update horiz
                newPos.x += vel.x;

                if ((checkHitBricks(newPos)) || (Math.abs(newPos.x) > 12.0))
                {
                    ballDir.x = -ballDir.x;
                    // TODO: SFX Bounce
                }                
                else
                {
                    ball.pos.x = newPos.x;
                }

                valisSpin += 2.0 * dt;
                valis.rotation.setFromEuler( new Vector( valisSpin, 0.0, 0.0 ) );
            }

            // rotato potato
            var rot = new Quaternion();
            rot.setFromAxisAngle( ballAxis, 40*(3.1415/180.0)*dt );

            ballAxis.add( new Vector( random_1_to_1() * 0.1,
                                        random_1_to_1() * 0.1, 
                                        random_1_to_1() * 0.1 ));
            ballAxis.normalize();

            ball.rotation.multiply( rot );



            Luxe.draw.text({
                batcher : hud_batcher,
                immediate : true,
                pos:new Vector(10,10),
                color:new Color().rgb(0xff4b03),
                text :  'Score: ${score}', //'world mouse : ' + world_mouse.x + ',' + world_mouse.y + '\n' + 'view mouse : ' + view_mouse.x + ',' + view_mouse.y
            });
         }

    } //update

    // Usefuls
    function random_1_to_1(){ return Math.random() * 2 - 1; }

} //Main
