fips_begin_app(ld41_cardfish windowed)
    fips_files(CardFish-Main.cpp 
    	       CardFishApp.h CardFishApp.cpp
    	       CardFishGame.h CardFishGame.cpp
    		  SceneObject.cpp SceneObject.h
	          #SceneCoordinator.cpp SceneCoordinator.h
	          #SceneController.cpp SceneController.h 
	          #GameScene.cpp GameScene.h
	          #TitleScene.cpp TitleScene.h
	          Camera.h Camera.cpp
	          Ray.h Ray.cpp
	          UIAssets.h UIAssets.cpp	          
	          DebugDraw.cpp DebugDraw.h debug_draw_impl.h	          
	          stb_image.h
	          par_easings.h

    		   )
    oryol_shader(shaders.glsl)
    oryol_shader(wireframe_shaders.glsl)
    fips_deps(Core IO LocalFS HttpFS Gfx Assets NKUI Dbg Input soloud)
fips_end_app()
