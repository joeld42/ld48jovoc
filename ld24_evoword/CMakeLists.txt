include_directories( . )

SET ( evoword_SRCS
      nesfont.cpp
      evoword_game.cpp
      evoword_game.h
      evoword_main.cpp
      grobold.cpp
      creature.h
      creature.cpp
      fragment.h
      fragment.cpp
      history_tree.h
      history_tree.cpp
)
SOURCE_GROUP( src FILES ${evoword_SRCS} )

# Include the gamedata files in their own group.
# Note you still need to manually add the "Copy Resources to Bundle"
# and add them there.
SET ( gamedata_FILES
    gamedata/nesfont.png
    gamedata/simpletex.png
    gamedata/evoword.glsl
    gamedata/background.png
    gamedata/grobold.png
    gamedata/palettes.txt
    gamedata/mouth1.png
    gamedata/mouthcard.obj
    gamedata/mouthparts.png
    gamedata/eyeball.obj
    gamedata/eyeball.png
    gamedata/eyelid.obj
    gamedata/critter_map.png
    gamedata/urchin.obj
    gamedata/2of12inf.txt
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
#   SET( MACOSX_BUNDLE_ICON_FILE evoword.icns 
#ENDIF(APPLE)



ADD_EXECUTABLE( evoword MACOSX_BUNDLE ${evoword_SRCS} ${gamedata_FILES} )
target_link_libraries( evoword ${LIBS} )
set_target_properties( evoword PROPERTIES
		       		 RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/ld24_evoword/ )


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
