fips_begin_app(darkstar windowed)
    fips_files(DarkStar-Main.cpp 
    		   SceneObject.cpp SceneObject.h
    		   Camera.cpp Camera.h
    		   )
    oryol_shader(shaders.glsl)
    oryol_shader(wireframe_shaders.glsl)
    fips_deps(Core IO LocalFS HttpFS Gfx Assets Input)
fips_end_app()
