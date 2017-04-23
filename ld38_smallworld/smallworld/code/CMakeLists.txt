fips_begin_app(smallworld windowed)
    fips_files(SmallWorld-Main.cpp 
    		   SceneObject.cpp SceneObject.h
    		   Camera.h Camera.cpp
    		   GameObjects.h GameObjects.cpp
    		   Planet.cpp Planet.h
    		   IsosurfaceBuilder.h IsosurfaceBuilder.cpp IsosurfaceTables.cpp
                par_easings.h
    		   )
    oryol_shader(shaders.shd)
    fips_deps(Core IO LocalFS Gfx Assets Input NKUI)
fips_end_app()
