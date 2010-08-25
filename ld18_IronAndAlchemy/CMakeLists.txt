# Include this directory in the CPPPATH
include_directories( . )
include_directories( ./luddite )
# include_directories( ${CMAKE_SOURCE_DIR}/luddite )

# Source files
SET( luddite_SRCS 

luddite/atom.cpp
luddite/debug.cpp
luddite/digistrip.cpp
luddite/font.cpp
luddite/luddite.cpp
luddite/particle.cpp
luddite/random.cpp
luddite/sprite.cpp
luddite/stb_image.cpp
luddite/texture.cpp
luddite/tweakval.cpp

behavior.cpp
enemy.cpp
entity.cpp
game.cpp
iron_main.cpp
physics.cpp
player.cpp
tilemap.cpp
)

# Add Header files to Visual Studio project
IF (WIN32)
   SET( luddite_SRCS ${luddite_SRCS} 
luddite/atom.h
luddite/avar.h
luddite/debug.h
luddite/font.h
luddite/GLee.h
luddite/handle.h
luddite/luddite.h
luddite/particle.h
luddite/quadbuff.h
luddite/random.h
luddite/resource.h
luddite/singleton.h
luddite/sprite.h
luddite/stb_image.h
luddite/texture.h
luddite/tweakval.h
behavior.h
enemy.h
entity.h
game.h
physics.h
player.h
tilemap.h
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
ADD_EXECUTABLE( iron_reflex ${luddite_SRCS} )
target_link_libraries( iron_reflex ${LIBS} )
set_target_properties( iron_reflex
                       PROPERTIES
                       RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/ld18_IronAndAlchemy )
