CMAKE_MINIMUM_REQUIRED( VERSION 2.6.4)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules)

project( ld48jovoc )

message ("CMAKE_SOURCE_DIR is ${CMAKE_SOURCE_DIR}" )
message ("Module path is  ${CMAKE_MODULE_PATH} " )

# ==================================================
# Set up dependancies
# ==================================================
FIND_PACKAGE( Allegro )
FIND_PACKAGE( OpenGL )
FIND_PACKAGE( GLUT )
FIND_PACKAGE( SDL )
FIND_PACKAGE( SDL_image )
FIND_PACKAGE( SDL_ttf )
FIND_PACKAGE( DevIL )
FIND_PACKAGE( PNG )
FIND_PACKAGE( ENET )
FIND_PACKAGE( ProtocolBuffers )


message( "Allegro  : " ${ALLEGRO_FOUND} )
message( "OpenGL   : " ${OPENGL_FOUND} )
message( "GLUT     : " ${GLUT_FOUND} )
message( "SDL      : " ${SDL_FOUND} )
message( "OpenIL   : " ${IL_FOUND} )
message( "PNG      : " ${PNG_FOUND} )
message( "ENET     : " ${ENET_FOUND} )
message( "protobuf : " ${PROTOBUF_FOUND} )

# ==================================================
# Find X11 on unix
# ==================================================
IF (UNIX)
  FIND_PACKAGE( X11 )
  FIND_PACKAGE( Threads )  
ENDIF (UNIX)

# ==================================================
#  External dependencies
# ==================================================
#no worky
#ADD_SUBDIRECTORY( extern/jpgd )
ADD_SUBDIRECTORY( extern/tinyxml )

# ==================================================
# LD1 - Zillagame
#     Depdencies: Glut, OpenGL, PNG
# 
# ==================================================
if (OPENGL_FOUND AND GLUT_FOUND)
  ADD_SUBDIRECTORY( ld1_zilla/trunk )
endif (OPENGL_FOUND AND GLUT_FOUND)

# ==================================================
# LD4 - Bad Food
#     Dependancies: Allegro, Alfont, libjpgd
# ==================================================
#if (ALLEGRO_FOUND)
#   ADD_SUBDIRECTORY( ld4_BadFood/trunk )
#endif (ALLEGRO_FOUND)

# ==================================================
# LD7 - Kudzu
#     Depdencies: SDL, OpenGL
# 
# ==================================================
if (OPENGL_FOUND AND SDL_FOUND)
  ADD_SUBDIRECTORY( ld7_Kudzu/trunk )
endif (OPENGL_FOUND AND SDL_FOUND)

# ==================================================
# LD10 - ChainGuy
#     Dependancies: Allegro
# ==================================================
if (ALLEGRO_FOUND)
   ADD_SUBDIRECTORY( ld10_chains/trunk )
endif (ALLEGRO_FOUND)

# ==================================================
# LD13 - Crossroads
#     Dependancies: SDL, OpenGL, OpenIL
# ==================================================
if (SDL_FOUND AND OPENGL_FOUND AND ALLEGRO_FOUND)
   ADD_SUBDIRECTORY( ld13_Crossroads/trunk )
endif (SDL_FOUND AND OPENGL_FOUND AND ALLEGRO_FOUND)

# ==================================================
# LD13 - Spaceship in a Cave
#     Dependancies: SDL, OpenGL, OpenIL, enet
# ==================================================
if (SDL_FOUND AND OPENGL_FOUND AND ALLEGRO_FOUND)
  ADD_SUBDIRECTORY( ld15_beneath/trunk/ )
endif (SDL_FOUND AND OPENGL_FOUND AND ALLEGRO_FOUND)

# ==================================================
# miniLD11 - wander
#     Dependancies: Allegro
# ==================================================
if (ALLEGRO_FOUND)
   ADD_SUBDIRECTORY( miniLD11_wander/trunk )
endif (ALLEGRO_FOUND)

# ==================================================
# ld18 Iron Reflex
#  Dependancies: SDL/OpenGL
# ==================================================
if (SDL_FOUND AND OPENGL_FOUND)
  ADD_SUBDIRECTORY( ld18_IronAndAlchemy )
endif (SDL_FOUND AND OPENGL_FOUND)

# ==================================================
# luddite (toolkit/basecode)
#  Dependancies: SDL (example only)/OpenGL
# ==================================================
if (SDL_FOUND AND OPENGL_FOUND)
  ADD_SUBDIRECTORY( luddite )
endif (SDL_FOUND AND OPENGL_FOUND)



