fips_begin_app(ld47loop windowed)
    fips_files(LD47Loop-Main.cpp 
    	       LD47LoopApp.h LD47LoopApp.cpp
    		  SceneObject.cpp SceneObject.h
	          GameSettings.cpp GameSettings.h
	          Renderizer.h Renderizer.cpp
	          Camera.h Camera.cpp
	          Ray.h Ray.cpp
	          UIAssets.h UIAssets.cpp	          
	          DebugDraw.cpp DebugDraw.h debug_draw_impl.h
			  DebugTools.cpp DebugTools.h
			  arcball_camera.cpp arcball_camera.h
	          ini.c ini.h     
	          stb_image.h
	          par_easings.h

    		   )
    oryol_shader(shaders.glsl)
    oryol_shader(shadow_shaders.glsl)
    oryol_shader(postproc.glsl)
    oryol_shader(background.glsl)
    oryol_shader(wireframe_shaders.glsl)
    fips_deps(Core IO LocalFS HttpFS Gfx Assets IMUI NKUI Dbg Input soloud)
fips_end_app()
