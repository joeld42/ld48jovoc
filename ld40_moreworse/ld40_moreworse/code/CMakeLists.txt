fips_begin_app(ld40_moreworse windowed)
    fips_files(LD40MoreWorse-Main.cpp 
    		   SceneObject.cpp SceneObject.h
    		   Camera.cpp Camera.h
    		   DebugDraw.cpp DebugDraw.h debug_draw_impl.h
    		   stb_image.h
    		   )
    oryol_shader(shaders.glsl)
    oryol_shader(wireframe_shaders.glsl)
    fips_deps(Core IO LocalFS HttpFS NKUI Gfx Assets Input)
fips_end_app()
