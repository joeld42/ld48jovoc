# Include this directory in the CPPPATH
include_directories( . )
include_directories( ${CMAKE_SOURCE_DIR}/extern )

# Source files
SET( crossroads_SRCS 
     Mesh.cpp
     GameViewGL.cpp
     GameState.cpp
     Crossroads.cpp
     )

# Add Header files to Visual Studio project
IF (WIN32)
   SET( crossroads_SRCS ${crossroads_SRCS} 
        AVar.h
        GameState.h
        GameViewGL.h
        Mesh.h
        )
ENDIF( WIN32 )

include_directories(
    ${IL_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${SDL_INCLUDE_DIR}
    ${X11_INCLUDE_DIRS}
    )   

set ( LIBS ${LIBS} 
    ${OPENGL_LIBRARIES} 

    ${IL_LIBRARY}
    ${ILU_LIBRARY}
    ${ILUT_LIBRARY}

    # Stupid ILUT is build with allegro support 
    ${ALLEGRO_LIBRARIES}


    ${SDL_LIBRARY}
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

ADD_DEFINITIONS( -DILUT_USE_SDL -DILUT_USE_OPENGL )

# The executable
ADD_EXECUTABLE( crossroads ${crossroads_SRCS} )
target_link_libraries( crossroads ${LIBS} )
set_target_properties( crossroads
                       PROPERTIES
                       RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/ld13_Crossroads/trunk )
