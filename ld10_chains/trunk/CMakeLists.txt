# Include this directory in the CPPPATH
include_directories( . )

# Source files
SET( chains_SRCS 
     chain_main.cpp 
     )

# Add Header files to Visual Studio project
#IF (WIN32)
#   SET( chains_SRCS ${wander_SRCS} 
#        )
#ENDIF( WIN32 )

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
ADD_EXECUTABLE( chains ${chains_SRCS} )
target_link_libraries( chains ${LIBS} )
set_target_properties( chains
                       PROPERTIES
                       RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/ld10_chains/trunk )
