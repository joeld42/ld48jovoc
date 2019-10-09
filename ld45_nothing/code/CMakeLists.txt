fips_begin_app(ld45nothing windowed)
    fips_files(LD45Nothing-Main.cpp 
    	       LD45NothingApp.h LD45NothingApp.cpp
    		  SceneObject.cpp SceneObject.h
	          #SceneCoordinator.cpp SceneCoordinator.h
	          #SceneController.cpp SceneController.h 
	          #GameScene.cpp GameScene.h
	          #TitleScene.cpp TitleScene.h
	          Sounds.h Sounds.cpp
	          CivGame.h CivGame.cpp
	          Renderizer.h Renderizer.cpp
	          Camera.h Camera.cpp
	          Ray.h Ray.cpp
	          UIAssets.h UIAssets.cpp	          
	          DebugDraw.cpp DebugDraw.h debug_draw_impl.h	          
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
