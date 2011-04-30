include_directories( . )

SET ( takethis_SRCS
      takethis_main.cpp )

include_directories( 
		     ${OPENGL_INCLUDE_DIR}
		     ${SDL_INCLUDE_DIR}
		     ${PNG_PNG_INCLUDE_DIR}
		     )

set ( LIBS ${LIBS}
      ${OPENGL_LIBRARIES}
      ${SDL_LIBRARY}
      )

ADD_DEFINITIONS( ${PNG_DEFINITIONS} )

ADD_EXECUTABLE( takethis ${takethis_SRCS} )
target_link_libraries( takethis ${LIBS} )
set_target_properties( takethis PROPERTIES
		       		 RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/ld20_TakeThis/ )

