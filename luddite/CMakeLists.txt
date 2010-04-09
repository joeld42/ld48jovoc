# Include this directory in the CPPPATH
include_directories( . )
include_directories( ./luddite )
# include_directories( ${CMAKE_SOURCE_DIR}/luddite )

# Source files
SET( luddite_SRCS 

     luddite/atom.cpp
     luddite/debug.cpp
     # luddite/debugtext.cpp
     # luddite/gamefontgl.cpp
     # luddite/gui.cpp
     # luddite/load_obj.cpp
     luddite/luddite.cpp
     # luddite/Md2Model.cpp
     luddite/resource.cpp
     luddite/texture.cpp
     # luddite/texture_dds.cpp
     # luddite/texture_png.cpp
     # luddite/tweakval.cpp

     example/luddite_demo.cpp
     )

# Add Header files to Visual Studio project
IF (WIN32)
   SET( luddite_SRCS ${luddite_SRCS} 

   # luddite/Anorms.h
   luddite/atom.h
   luddite/avar.h
   luddite/debug.h
   # luddite/gamefontgl.h
   # luddite/gui.h
   # luddite/load_obj.h
   luddite/luddite.h
   # luddite/Md2Model.h
   luddite/resource.h
   # luddite/texture_dds.h
   luddite/texture.h
   # luddite/texture_png.h
   # luddite/tweakval.h

        )
ENDIF( WIN32 )

# Use GLUT/OpenGL
include_directories( 
    ${OPENGL_INCLUDE_DIR} 
    ${SDL_INCLUDE_DIR}
    )

#include_directories( ${X11_INCLUDE_DIRS} )
set ( LIBS ${LIBS} 
    ${OPENGL_LIBRARIES} 
    ${SDL_LIBRARY}
     )


# === Additional dependancies to build on unix
if (UNIX)
    include_directories( ${X11_INCLUDE_DIRS} )
    set ( LIBS ${LIBS} 
        ${X11_LIBRARIES} 
#        ${X11_Xcursor_LIB}
#        ${X11_Xpm_LIB}
#        pthread
    )

endif (UNIX)

ADD_DEFINITIONS( ${PNG_DEFINITIONS} )

# The executable
ADD_EXECUTABLE( luddite_demo ${luddite_SRCS} )
target_link_libraries( luddite_demo ${LIBS} )
set_target_properties( luddite_demo
                       PROPERTIES
                       RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/luddite/example )
