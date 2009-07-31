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

message( "Allegro  : " ${ALLEGRO_FOUND} )
message( "OpenGL   : " ${OPENGL_FOUND} )
message( "GLUT     : " ${GLUT_FOUND} )
message( "SDL      : " ${SDL_FOUND} )
message( "OpenIL   : " ${IL_FOUND} )

# ==================================================
# Find X11 on unix
# ==================================================
IF (UNIX)
  FIND_PACKAGE( X11 )
  FIND_PACKAGE( Threads )  
ENDIF (UNIX)

# ==================================================
# LD1 - Zillagame
#     Depdencies: Glut, OpenGL, Plib
# 
# ==================================================
#if (OPENGL_FOUND AND GLUT_FOUND)
#  ADD_SUBDIRECTORY( ld1_zilla/trunk )
#endif (OPENGL_FOUND AND GLUT_FOUND)

# ==================================================
# LD4 - Bad Food
#     Dependancies: Allegro, Alfont
# ==================================================
#if (ALLEGRO_FOUND)
#   ADD_SUBDIRECTORY( ld4_BadFood/trunk )
#endif (ALLEGRO_FOUND)

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
if (SDL_FOUND AND OPENGL_FOUND)
   ADD_SUBDIRECTORY( ld13_Crossroads/trunk )
endif (SDL_FOUND AND OPENGL_FOUND)

# ==================================================
# miniLD11 - wander
#     Dependancies: Allegro
# ==================================================
if (ALLEGRO_FOUND)
   ADD_SUBDIRECTORY( miniLD11_wander/trunk )
endif (ALLEGRO_FOUND)



