# Source Files
SET ( minibasecode_SRCS
	GLee.c
	GLee.h
	png_loader.cpp
	png_loader.h
	gamedata.cpp
	gamedata.h
	random.cpp
	random.h
	font.h
	font.cpp
)

include_directories( 
		     ${CMAKE_SOURCE_DIR}/extern/
		     ${CMAKE_SOURCE_DIR}/extern/glsw/

		     ${OPENGL_INCLUDE_DIR}
		     ${SDL_INCLUDE_DIR}
		     ${PNG_PNG_INCLUDE_DIR}
		     )


# minibasecode library
add_library( minibasecode ${minibasecode_SRCS} )

