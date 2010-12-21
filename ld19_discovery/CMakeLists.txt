# Include this directory in the CPPPATH
include_directories( . )
include_directories( ${CMAKE_SOURCE_DIR}/extern/glsw/ )
include_directories( ${CMAKE_SOURCE_DIR}/luddite/ )
include_directories( ${CMAKE_SOURCE_DIR}/luddite/luddite/ )


# Source files
SET( discovery_SRCS 

discovery_main.cpp
bonsai.cpp
noise.cpp
pally.cpp

# Skeletal PVRT math
PVRT/PVRTQuaternionX.cpp
PVRT/PVRTQuaternionF.cpp
PVRT/PVRTFixedPoint.cpp
PVRT/PVRTVector.cpp
PVRT/PVRTMatrixX.cpp
PVRT/PVRTMatrixF.cpp

../luddite/luddite/GLee.c
../luddite/luddite/font.cpp
../luddite/luddite/texture.cpp
../luddite/luddite/debug.cpp
../luddite/luddite/digistrip.cpp
../luddite/luddite/random.cpp
../luddite/luddite/debug.cpp
../luddite/luddite/PNGLoader.cpp

../extern/glsw/bstrlib.c
../extern/glsw/glsw.c

)

# Use GLUT/OpenGL
include_directories( 
    ${OPENGL_INCLUDE_DIR} 
    ${SDL_INCLUDE_DIR}
    )

#include_directories( ${X11_INCLUDE_DIRS} )
set ( LIBS ${LIBS} 
    ${OPENGL_LIBRARIES} 
    ${SDL_LIBRARY}
    z
    png
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
ADD_EXECUTABLE( discovery ${discovery_SRCS} )
target_link_libraries( discovery ${LIBS} )
set_target_properties( discovery
                       PROPERTIES
                       RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/ld19_discovery )
