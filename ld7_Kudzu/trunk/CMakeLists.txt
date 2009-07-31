# Include this directory in the CPPPATH
include_directories( . )
include_directories( ${CMAKE_SOURCE_DIR}/extern )

# Source files
SET( kudzu_SRCS 
     klevel.cpp
     kudzu.cpp
     )

# Add Header files to Visual Studio project
IF (WIN32)
   SET( kudzu_SRCS ${kudzu_SRCS} 
        klevel.h
        kmesh.h
        kudzu.h
        )
ENDIF( WIN32 )

# Use GLUT/OpenGL
include_directories( 
    ${OPENGL_INCLUDE_DIR} 
    ${SDL_INCLUDE_DIR}
    )

#include_directories( ${ALLEGRO_INCLUDE_DIRS} ${X11_INCLUDE_DIRS} )
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
ADD_EXECUTABLE( kudzu ${kudzu_SRCS} )
target_link_libraries( kudzu ${LIBS} )
set_target_properties( kudzu
                       PROPERTIES
                       RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/ld7_Kudzu/trunk )
