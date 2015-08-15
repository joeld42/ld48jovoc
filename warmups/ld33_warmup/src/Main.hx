

import luxe.Input;
import luxe.Mesh;
import luxe.Color;
import luxe.Vector;
import luxe.Parcel;
import luxe.ParcelProgress;

import luxe.components.cameras.FlyCamera;

import phoenix.geometry.Geometry;
import phoenix.geometry.Vertex;
import phoenix.geometry.TextureCoord;
import phoenix.Batcher;
import phoenix.Texture;
import phoenix.Camera;

import snow.api.buffers.Uint8Array;

import SceneBuilder;

class Main extends luxe.Game {

	var meshGround_ : Mesh;
	var meshTree_ : Mesh;	
	var meshPlayer_ : Mesh;
	var meshOak_ : Mesh;

	var builder_ = new SceneBuilder();

	override function config( config : luxe.AppConfig )
	{
		config.render.antialiasing = 8;
		config.render.depth_bits = 24;
		config.render.depth = true;

		config.preload.textures.push({ id :  "assets/ground_swirl.png", 
									clamp_s : ClampType.repeat, 
									clamp_t : ClampType.repeat  });
		config.preload.textures.push({ id :  "assets/img_tree.png", 
									clamp_s : ClampType.repeat, 
									clamp_t : ClampType.repeat  });
		config.preload.textures.push({ id :  "assets/player.png" });
		config.preload.textures.push({ id :  "assets/pine.png",  
									clamp_s : ClampType.repeat, 
									clamp_t : ClampType.repeat });
		config.preload.textures.push({ id :  "assets/ash_uvgrid02.png", 
									clamp_s : ClampType.repeat, 
									clamp_t : ClampType.repeat  });

		config.preload.texts.push({ id :  "assets/grid10x10.obj" });
		config.preload.texts.push({ id :  "assets/tree_fir.obj" });
		config.preload.texts.push({ id :  "assets/player.obj" });

		config.preload.bytes.push({ id : "assets/mesh/MESH_TreeOakMesh.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_Cube.dat" });

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

    	var cam = new FlyCamera({
    		name:'flycam',
    		projection: ProjectionType.perspective,
    		fov:90,
    		near:0.1,
    		far:1000,
    		cull_backfaces : false,
    		aspect:Luxe.screen.w/Luxe.screen.h
		});

		Luxe.renderer.batcher.view = cam.view;

		//move up and back a bit
    	cam.pos.set_xyz(0,20,15);
    	cam.rotation.setFromEuler( new Vector( -50.0, 0, 0).radians() );

    	var groundTex = Luxe.resources.texture( 'assets/ground_swirl.png');
    	groundTex.generate_mipmaps();
    	// groundTex.clamp_s = groundTex.clamp_t = ClampType.repeat;
    	// groundTex.filter = mip_linear_linear;
    	meshGround_ = new Mesh({ file : 'assets/grid10x10.obj', texture : groundTex } );    	    	
    	//meshGround_.pos.set_xyz( 0.0, -2.0, 0.0 );
    	meshGround_.geometry.locked = true;    	

    	// var treeTex = Luxe.resources.texture( 'assets/img_tree.png');    	
    	// meshTree_ = new Mesh({ file : 'assets/tree_fir.obj', texture : treeTex } );    	
    	// meshTree_.pos.set_xyz( 3.0, 0.0, 0.0 );

    	//var texPlayer = Luxe.resources.texture( 'assets/player.png');    	
    	//meshPlayer_ = new Mesh({ file : 'assets/player.obj', texture : texPlayer } );    	
    	
    	//var texTreeOak = Luxe.resources.texture( 'assets/ash_uvgrid02.png' );
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

    var rot = 0.0;
    override function update(dt:Float) {
    	rot += 90*dt;
    	//meshOak_.rotation.setFromEuler(new Vector(0,rot,0).radians());
    } //update


} //Main
