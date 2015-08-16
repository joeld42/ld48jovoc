

import luxe.Input;
import luxe.Mesh;
import luxe.Color;
import luxe.Vector;
import luxe.Parcel;
import luxe.ParcelProgress;
import luxe.Sprite;
import luxe.Camera;
import luxe.Rectangle;

import luxe.components.cameras.FlyCamera;

import phoenix.geometry.Geometry;
import phoenix.geometry.Vertex;
import phoenix.geometry.TextureCoord;
import phoenix.Batcher;
import phoenix.Texture;
// import phoenix.Camera;

import phoenix.Shader;
import phoenix.RenderTexture;

import snow.api.buffers.Uint8Array;
import snow.system.assets.Assets;

import SceneBuilder;

// notes: small things that would be useful
//  -- create a resource on the fly
//  -- build a shader from source

class Main extends luxe.Game {

	var meshGround_ : Mesh;
	var meshTree_ : Mesh;	
	var meshPlayer_ : Mesh;
	var meshOak_ : Mesh;

	var worldShader_ : Shader;

	var builder_ = new SceneBuilder();

	var texTarget_ : RenderTexture;
	var display_sprite : Sprite;

	var hudBatcher_ :Batcher;

	var flyCamera_ : FlyCamera;
	var topBatcher_ : Batcher;
	var topCamera_ : Camera;

	override function config( config : luxe.AppConfig )
	{
		config.render.antialiasing = 8;
		config.render.depth_bits = 24;
		config.render.depth = true;

		var texNames = [ "ground_swirl.png", "img_tree.png", "player.png", "pine.png", 
						 "missing.png", "rock.png", "stone.png"];
		for (texName in texNames)
		{
			config.preload.textures.push({ id :  "assets/" + texName,
									clamp_s : ClampType.repeat, 
									clamp_t : ClampType.repeat  });
		}

		config.preload.texts.push({ id :  "assets/grid10x10.obj" });
		config.preload.texts.push({ id :  "assets/tree_fir.obj" });
		config.preload.texts.push({ id :  "assets/player.obj" });

		config.preload.texts.push({ id :  "assets/warmup.glsl" });
		// config.preload.texts.push({ id :  "assets/world.vert.glsl" });
		// config.preload.texts.push({ id :  "assets/world.frag.glsl" });
		config.preload.shaders.push({ id:'world', frag_id:'assets/world.frag.glsl', vert_id:'assets/world.vert.glsl' });


		// config.preload.bytes.push({ id : "assets/mesh/MESH_TreeOakMesh.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_Cube.dat" });

		config.preload.bytes.push({ id : "assets/mesh/MESH_Brown_Cliff_Mesh.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_Brown_Cliff_Top_Mesh.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_Cube.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_Ground.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_Rock1Mesh.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_TreeOakMesh.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_TreePineMesh.dat" });

		config.preload.jsons.push( { id : "assets/forest_small_test.json" });

		return config;
	}

    override function ready() {

    	// Luxe.camera.view.set_perspective({
    	// 	far:1000,
     //        near:0.1,
     //        fov: 90,
     //        cull_backfaces : false,
     //        aspect : Luxe.screen.w/Luxe.screen.h
    	// });

    	flyCamera_ = new FlyCamera({
    		name:'flycam',
    		projection: ProjectionType.perspective,
    		fov:90,
    		near:0.1,
    		far:1000,
    		cull_backfaces : true,
    		aspect:Luxe.screen.w/Luxe.screen.h
		});

 		worldShader_ = Luxe.resources.shader('world');
    	builder_.testShader_ = worldShader_;
    	trace( 'Shader is ${worldShader_}' );

		Luxe.renderer.batcher.view = flyCamera_.view;

		topCamera_ = new Camera({
				name : 'topcam',
				near: 0.1,
				far: 1000,
				aspect: 1.0,
				projection: ProjectionType.ortho,				
			});

		topCamera_.viewport = new Rectangle( 0, 0, 512, 512 );

		topBatcher_ = Luxe.renderer.create_batcher({ name : 'topbatcher', camera:topCamera_.view });

		//move up and back a bit
    	flyCamera_.pos.set_xyz(0,20,15);
    	flyCamera_.rotation.setFromEuler( new Vector( -50.0, 0, 0).radians() );

    	var groundTex = Luxe.resources.texture( 'assets/ground_swirl.png');
    	groundTex.generate_mipmaps();
    	groundTex.clamp_s = groundTex.clamp_t = ClampType.repeat;
    	 // groundTex.filter = mip_linear_linear;
    	meshGround_ = new Mesh({ file : 'assets/grid10x10.obj', texture : groundTex, batcher : topBatcher_ } );    	    	
    	meshGround_.pos.set_xyz( 0.0, -2.0, 0.0 );
    	meshGround_.geometry.locked = true;    	

    	builder_.loadScene( "assets/forest_small_test.json");

    	 texTarget_ = new RenderTexture({ id:'rtt', width:512, height:512 });

     	 hudBatcher_ = Luxe.renderer.create_batcher({ name:'hud_batcher', layer:4 });

    	 display_sprite = new Sprite({
    	 	batcher : hudBatcher_,
            texture : texTarget_,
            size : new Vector(100, 100),
            //pos : Luxe.screen.mid,
            pos : new Vector( 60, 60 )
        });


    }

	// load effect for shaders
    function load_effect( path:String ) {
        trace( '>>> load_effect: ${path}');
        return Luxe.resources.text( path ).asset.text;
    }

    function makeTreeTestGrid() 
    {
    	for (i in 0...100)
    	{
			var meshTree = builder_.makeInstance( "assets/mesh/MESH_TreeOakMesh.dat", "assets/pine.png" );
			var ii : Float = (Std.int(i/10) * 4) - 15.0;
			var jj : Float = (Std.int(i%10) * 4) - 15.0; 
			trace('row: ${i} ${Std.int(i/10)%2}');
			if (Std.int(i/10)%2 > 0 ) {
				jj += 2.0;
			}
			meshTree.pos.set_xyz( ii, 0, jj ); 
			meshTree.geometry.locked = true;
		}    	
    }

    override function onkeyup( e:KeyEvent ) {

        if(e.keycode == Key.escape) {
            Luxe.shutdown();
        }

    } //onkeyup

     override function onprerender() {

            //wait for onloaded
        if(display_sprite == null) return;

            //Set the current rendering target
        Luxe.renderer.target = texTarget_;

                //clear the texture!
            Luxe.renderer.clear(new Color().rgb(0xff4b03));
                
                //draw the geometry inside our batcher
            //Luxe.renderer.batcher.draw();
            topBatcher_.draw();

            //reset the target back to no target (i.e the screen)
        Luxe.renderer.target = null;

    } //onprerender

    var rot = 0.0;
    override function update(dt:Float) {
    	rot += 90*dt;
    	//meshOak_.rotation.setFromEuler(new Vector(0,rot,0).radians());
    } //update


} //Main
