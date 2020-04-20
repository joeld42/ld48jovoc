fips_begin_app(ld46_keepalive windowed)
    fips_files(LD46KeepAlive-Main.cpp 
    	      LD46KeepAliveApp.h LD46KeepAliveApp.cpp
              FireGame.h FireGame.cpp

    		  SceneObject.cpp SceneObject.h
	          GameSettings.cpp GameSettings.h
	          Renderizer.h Renderizer.cpp
	          Camera.h Camera.cpp
              Sounds.h Sounds.cpp
	          Ray.h Ray.cpp
	          UIAssets.h UIAssets.cpp	          
	          DebugDraw.cpp DebugDraw.h debug_draw_impl.h              
	          ini.c ini.h     
	          stb_image.h
	          par_easings.h

    		   )
    oryol_shader(shaders.glsl)
    oryol_shader(shadow_shaders.glsl)
    oryol_shader(postproc.glsl)
    oryol_shader(background.glsl)
    oryol_shader(wireframe_shaders.glsl)
    fips_deps(Core IO LocalFS HttpFS Gfx Assets NKUI Dbg Input soloud)
fips_end_app()
