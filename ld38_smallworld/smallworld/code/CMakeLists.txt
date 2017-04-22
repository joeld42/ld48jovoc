fips_begin_app(smallworld windowed)
    fips_files(SmallWorld-Main.cpp 
    		   SceneObject.cpp SceneObject.h
    		   Camera.cpp Camera.h
    		   Planet.cpp Planet.h
    		   IsosurfaceBuilder.h IsosurfaceBuilder.cpp IsosurfaceTables.cpp
    		   )
    oryol_shader(shaders.shd)
    fips_deps(Core IO LocalFS Gfx Assets Input NKUI)
fips_end_app()
