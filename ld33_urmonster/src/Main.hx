import luxe.Input;
import luxe.Mesh;
import luxe.Color;
import luxe.Vector;
import luxe.Parcel;
import luxe.ParcelProgress;
import luxe.Sprite;
import luxe.Camera;
import luxe.Rectangle;
import luxe.Entity;

import luxe.components.cameras.FlyCamera;

import phoenix.geometry.Geometry;
import phoenix.geometry.Vertex;
import phoenix.geometry.TextureCoord;
import phoenix.Batcher;
import phoenix.Texture;
import phoenix.Shader;

import snow.api.buffers.Uint8Array;
import snow.system.assets.Assets;

import SceneRender;


class Main extends luxe.Game {

	var worldShader_ : Shader;
	var hudBatcher_ :Batcher;

	var flyCamera_ : FlyCamera;

	var scene_ : SceneRender;

	var zilla_ : SceneObj;

	override function config( config : luxe.AppConfig )
	{
		config.render.antialiasing = 8;
		config.render.depth_bits = 24;
		config.render.depth = true;

		preloadResources(config);

		return config;
	}

	// TODO: make this a regular preload parcel and
	// just use the app preload to set up a preloadprogress
	function preloadResources( config : luxe.AppConfig )
	{
		// Configure texture
		var texNames = [ "ground1.jpg", "house.png", "suzilla.png" ];
		for (texName in texNames)
		{
			config.preload.textures.push({ id :  "assets/" + texName,
									clamp_s : ClampType.repeat, 
									clamp_t : ClampType.repeat  });
		}

		config.preload.shaders.push({ id:'world', frag_id:'assets/world.frag.glsl', vert_id:'assets/world.vert.glsl' });
		//config.preload.shaders.push({ id:'dbg_shad', frag_id:'assets/dbg_shad.frag.glsl', vert_id:'default' });
		//config.preload.shaders.push({ id:'shadow', frag_id:'assets/shadow.frag.glsl', vert_id:'assets/shadow.vert.glsl' });

		// TESTLAND
		config.preload.bytes.push({ id : "assets/mesh/MESH_Cube.001.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_GroundMesh.dat" });
		config.preload.bytes.push({ id : "assets/mesh/MESH_SuzillaMesh.dat" });

		config.preload.jsons.push( { id : "assets/testland.json" });

	}

	function initCamera()
	{
		flyCamera_ = new FlyCamera({
    		name:'flycam',
    		projection: ProjectionType.perspective,
    		fov:90,
    		near:0.1,
    		far:1000,
    		cull_backfaces : true,
    		aspect:Luxe.screen.w/Luxe.screen.h
		});
    	
    	scene_.sceneCamera_ = flyCamera_;
    	Luxe.renderer.batcher.view = flyCamera_.view;

    	flyCamera_.pos.set_xyz(0,20,15);
    	flyCamera_.rotation.setFromEuler( new Vector( -50.0, 0, 0).radians() );
	}

    override function ready() 
    {
    	scene_ = new SceneRender();

 		worldShader_ = Luxe.resources.shader('world');
    	scene_.worldShader_ = worldShader_;
    	//scene_.shadowShader_ = Luxe.resources.shader('shadow');
    	
    	// init camera
    	initCamera();

    	// init lighting
    	Luxe.renderer.clear_color.rgb(0x6ca9d3);
    	var lightDir = new Vector( 1, -0.7, 1.0 );
    	lightDir.normalize();
    	lightDir.invert();
    	worldShader_.set_vector3( "lightDir", lightDir);

		hudBatcher_ = Luxe.renderer.create_batcher({ name:'hud_batcher', layer:4,  no_add : true});

    	scene_.loadScene( "assets/testland.json");
    	scene_.initShadows();

    	zilla_ = scene_.findSceneObj("Suzilla");
    	var camPos = new Vector();
    	camPos.copy_from(zilla_.xform_.pos);
    	
    	flyCamera_.pos.set_xyz( camPos.x, camPos.y + 5.0, camPos.z );

    } //ready

    override function onkeyup( e:KeyEvent ) {

        if(e.keycode == Key.escape) {
            Luxe.shutdown();
        }

    } //onkeyup

	override function onpostrender() {

        scene_.drawScene();

        // Draw the hud batcher afterward so the hud scene stays on top        
		hudBatcher_.draw();

    } //onpostrender

    override function update(dt:Float) {

    } //update


} //Main
