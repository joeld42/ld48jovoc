
FIND_PACKAGE( ProtocolBuffers )

# Include this directory in the CPPPATH
include_directories( . )

# Also include the build dir (to get protobuf files)
include_directories( ${CMAKE_BINARY_DIR}/ld15_beneath/trunk/)

# include extern dirs
include_directories( ${CMAKE_SOURCE_DIR}/extern )
include_directories( ${CMAKE_SOURCE_DIR}/extern/tinyxml )

# Source files
SET( spacecave_SRCS 
     Cavern.cpp  
     Common.cpp  
     Editor.cpp  
     GameClient.cpp  
     Shape.cpp  
     Server.cpp  
     GameObj.cpp
     PlayerShip.cpp

     gamefontgl.c  
     jimgui.cpp  
     spacecave_main.cpp  
     tweakval.cpp
     )

# Add Header files to Visual Studio project
IF (WIN32)
   SET( spacecave_SRCS ${spacecave_SRCS} 
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
    ${PROTOBUF_INCLUDE_DIR}
    ${ENET_INCLUDE_DIR}
    )   

set ( LIBS ${LIBS} 
    ${OPENGL_LIBRARIES} 

    ${IL_LIBRARY}
    ${ILU_LIBRARY}
    ${ILUT_LIBRARY}

    # Stupid ILUT is build with allegro support 
    ${ALLEGRO_LIBRARIES}

    ${PROTOBUF_LIBRARY}
    
    ${SDL_LIBRARY}
    ${ENET_LIBRARIES}
     )


# Call protoc to wrap stuff
# FIXME: doing this by hand for now
WRAP_PROTO( PROTO_SRC SpaceCave.proto )

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
ADD_EXECUTABLE( spacecave ${spacecave_SRCS} ${PROTO_SRC} )
target_link_libraries( spacecave ${LIBS} tinyxml )
set_target_properties( spacecave
                       PROPERTIES
                       RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/ld15_beneath/trunk )
