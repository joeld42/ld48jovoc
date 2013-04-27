include_directories( . )

SET ( minimalism_SRCS
      nesfont.cpp
      minimalism_game.cpp
      minimalism_game.h
      minimalism_main.cpp
)
SOURCE_GROUP( src FILES ${minimalism_SRCS} )

# Include the gamedata files in their own group.
# Note you still need to manually add the "Copy Resources to Bundle"
# and add them there.
SET ( gamedata_FILES
    gamedata/nesfont.png
    gamedata/simpletex.png
    gamedata/minimalism.glsl
)
SOURCE_GROUP( gamedata FILES ${gamedata_FILES} )

include_directories( 
         ${CMAKE_SOURCE_DIR}/minibasecode/
		     ${CMAKE_SOURCE_DIR}/extern/
		     ${CMAKE_SOURCE_DIR}/extern/glsw/

		     ${OPENGL_INCLUDE_DIR}
		     ${SDL_INCLUDE_DIR}
		     ${PNG_PNG_INCLUDE_DIR}
		     )

set ( LIBS ${LIBS}
      ${OPENGL_LIBRARIES}
      ${SDL_LIBRARY}
      ${PNG_LIBRARY}
      minibasecode
      glsw
      )

ADD_DEFINITIONS( ${PNG_DEFINITIONS} )

#IF(APPLE)
#   SET( MACOSX_BUNDLE_ICON_FILE minimalism.icns 
#ENDIF(APPLE)



ADD_EXECUTABLE( minimalism MACOSX_BUNDLE ${minimalism_SRCS} ${gamedata_FILES} )
target_link_libraries( minimalism ${LIBS} )
set_target_properties( minimalism PROPERTIES
		       		 RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/ld26_minimalism/ )


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
