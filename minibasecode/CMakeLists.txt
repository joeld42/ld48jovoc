# Source Files
SET ( minibasecode_SRCS
	GLee.c
	GLee.h
	png_loader.cpp
	png_loader.h
	gamedata.cpp
	gamedata.h
	useful.cpp
	useful.h
	font.h
	font.cpp
	load_obj.h
	load_obj.cpp
	pally.h
	pally.cpp
	quadbuff.h
	shapes.h
	shapes.cpp
	shader.h
	shader.cpp
	sprite.h
	sprite.cpp

)

include_directories( 
	         ${CMAKE_SOURCE_DIR}/minibasecode/
		     ${CMAKE_SOURCE_DIR}/extern/
		     ${CMAKE_SOURCE_DIR}/extern/glsw/

		     ${OPENGL_INCLUDE_DIR}
		     ${SDL_INCLUDE_DIR}
		     ${PNG_PNG_INCLUDE_DIR}
		     )


# minibasecode library
add_library( minibasecode ${minibasecode_SRCS} )

