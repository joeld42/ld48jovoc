include_directories( . )

SET ( takethis_SRCS
      GLee.c
      MapRoom.cpp
      PNGLoader.cpp
      TakeThisGame.cpp
      VoxChunk.cpp
      VoxSprite.cpp
      font.cpp
      nesfont.cpp
      random.cpp
      takethis_main.cpp 

      GLee.h
      MapRoom.h
      PNGLoader.h
      TakeThisGame.h
      VoxChunk.h
      VoxSprite.h
      font.h
      quadbuff.h
      random.h
)

include_directories( 
		     ${CMAKE_SOURCE_DIR}/extern/
		     ${OPENGL_INCLUDE_DIR}
		     ${SDL_INCLUDE_DIR}
		     ${PNG_PNG_INCLUDE_DIR}
		     )

set ( LIBS ${LIBS}
      ${OPENGL_LIBRARIES}
      ${SDL_LIBRARY}
      ${PNG_LIBRARY}
      )

ADD_DEFINITIONS( ${PNG_DEFINITIONS} )

#IF(APPLE)
#   SET( MACOSX_BUNDLE_ICON_FILE TakeThis.icns 
#ENDIF(APPLE)



ADD_EXECUTABLE( takethis MACOSX_BUNDLE ${takethis_SRCS} )
target_link_libraries( takethis ${LIBS} )
set_target_properties( takethis PROPERTIES
		       		 RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/ld20_TakeThis/ )


#INSTALL( TARGETS takethis
#   BUNDLE_DESTINATION . COMPONENT Runtime
#  RUNTIME DESTINATION bin COMPONENT Runtime
#    )

#set(APPS ...)  # paths to executables
#set(DIRS ...)   # directories to search for prerequisites

#message ( "APPS \${CMAKE_INSTALL_PREFIX}/${_dir}/${PROJECT_NAME}${EXE_DEBUG_EXTENSION}${_extension}" )

#INSTALL(CODE "
 #  include(BundleUtilities)
  # fixup_bundle(\"${APPS}\"   \"\"   \"${DIRS}\")
  # " COMPONENT Runtime)
