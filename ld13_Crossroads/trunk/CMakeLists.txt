# Include this directory in the CPPPATH
include_directories( . )
include_directories( 
             ${CMAKE_SOURCE_DIR}/extern/
             ${CMAKE_SOURCE_DIR}/extern/glsw/
             )

# Source files
SET( crossroads_SRCS 

        AVar.h
        GameState.h
        GameViewGL.h
        Mesh.h

        Mesh.cpp
        GameViewGL.cpp
        GameState.cpp
        Crossroads.cpp

        GLee.c
        PNGLoader.cpp
        ResourceFile.cpp

        GLee.h
        PNGLoader.h
     )

include_directories(
    ${OPENGL_INCLUDE_DIR}
    ${SDL_INCLUDE_DIR}
    ${X11_INCLUDE_DIRS}
    ${PNG_PNG_INCLUDE_DIR}
    )   

set ( LIBS ${LIBS} 
    ${OPENGL_LIBRARIES} 
    ${SDL_LIBRARY}
    ${PNG_LIBRARY}
     )


# === Additional dependancies to build on unix
#if (UNIX)
#    include_directories( ${X11_INCLUDE_DIRS} )
#    set ( LIBS ${LIBS} 
#        ${X11_LIBRARIES} 
#        ${X11_Xcursor_LIB}
#        ${X11_Xpm_LIB}
#        pthread
#    )
#endif (UNIX)

ADD_EXECUTABLE( crossroads MACOSX_BUNDLE ${crossroads_SRCS} )
target_link_libraries( crossroads ${LIBS} )
set_target_properties( crossroads PROPERTIES
                     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/ld13_Crossroads/ )


