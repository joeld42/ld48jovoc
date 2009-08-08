# Include this directory in the CPPPATH
include_directories( . )

# Source files
SET( wander_SRCS 
     game_obj.cpp
     pixbot.cpp
     tilemap.cpp
     mkterrain.cpp
     wander_main.cpp 
     )

# Add Header files to Visual Studio project
IF (WIN32)
   SET( wander_SRCS ${wander_SRCS} 
        game_obj.h
        tilemap.h 
        mkterrain.h
        )
ENDIF( WIN32 )

# Use allegro
include_directories( ${ALLEGRO_INCLUDE_DIRS} ${X11_INCLUDE_DIRS} )
set ( LIBS ${LIBS} 
    ${ALLEGRO_LIBRARIES} 
     )

# === Additional dependancies to build on unix
if (UNIX)
    set ( LIBS ${LIBS} 
        ${X11_LIBRARIES} 
        ${X11_Xcursor_LIB}
        ${X11_Xpm_LIB}
        pthread
    )

endif (UNIX)


# The executable
ADD_EXECUTABLE( wander ${wander_SRCS} )
target_link_libraries( wander ${LIBS} )
set_target_properties( wander
                       PROPERTIES
                       RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/miniLD11_wander/trunk )
