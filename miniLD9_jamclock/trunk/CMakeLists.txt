# Include this directory in the CPPPATH
include_directories( . )
include_directories( ${CMAKE_SOURCE_DIR}/extern )

# Source files
SET( jamwheel_SRCS 
     clock_main.cpp
     )

# Add Header files to Visual Studio project
IF (WIN32)
   SET( jamwheel_SRCS ${jamwheel_SRCS} 
        )
ENDIF( WIN32 )

include_directories(
    ${IL_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${SDL_INCLUDE_DIR}
    ${X11_INCLUDE_DIRS}

    # FIXME: find this from the system
    /work/stuff/fmodapi43205linux64/api/inc

    )   


set ( LIBS ${LIBS} 
    ${OPENGL_LIBRARIES} 

    ${SDL_LIBRARY}

    # FIXME: find this from the system
    -L/work/stuff/fmodapi43205linux64/api/inc
    -lfmodex64

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
ADD_EXECUTABLE( jamwheel ${jamwheel_SRCS} )
target_link_libraries( jamwheel ${LIBS} )
set_target_properties( jamwheel
                       PROPERTIES
                       RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/miniLD9_jamclock/trunk )
