fips_begin_app(darkstar windowed)
    fips_files(DarkStar-Main.cpp 
    		   SceneObject.cpp SceneObject.h
    		   Camera.cpp Camera.h
    		   )
    oryol_shader(shaders.shd)
    fips_deps(Core IO LocalFS Gfx Assets Input)
fips_end_app()
