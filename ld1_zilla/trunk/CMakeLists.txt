# Include this directory in the CPPPATH
include_directories( . )
include_directories( ${CMAKE_SOURCE_DIR}/extern )

# Source files
SET( zilla_SRCS 
     loadPng.cpp
     main.cpp
     )

# Add Header files to Visual Studio project
IF (WIN32)
   SET( zilla_SRCS ${zilla_SRCS} 
        zillaGame.h
        )
ENDIF( WIN32 )

# Use GLUT/OpenGL
include_directories( 
    ${OPENGL_INCLUDE_DIR} 
    ${GLUT_INCLUDE_DIR} 
    ${PNG_INCLUDE_DIR}
    )

#include_directories( ${ALLEGRO_INCLUDE_DIRS} ${X11_INCLUDE_DIRS} )
set ( LIBS ${LIBS} 
    ${OPENGL_LIBRARIES} 
    ${GLUT_LIBRARIES}
    ${PNG_LIBRARIES}
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
ADD_EXECUTABLE( zilla ${zilla_SRCS} )
target_link_libraries( zilla ${LIBS} )
set_target_properties( zilla
                       PROPERTIES
                       RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/ld1_zilla/trunk )
