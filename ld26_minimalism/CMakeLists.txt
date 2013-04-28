include_directories( . )

SET ( minimalism_SRCS
    actor.cpp
    actor.h
    minimalism_game.cpp
    minimalism_game.h
    minimalism_main.cpp
    nesfont.cpp
    scene_obj.cpp
    scene_obj.h
    world.cpp
    world.h
)
SOURCE_GROUP( src FILES ${minimalism_SRCS} )

# Include the gamedata files in their own group.
# Note you still need to manually add the "Copy Resources to Bundle"
# and add them there.
SET ( gamedata_FILES
    gamedata/block_1x1.obj
    gamedata/block_sm_tri.obj
    gamedata/block_tall.obj
    gamedata/block_tall_tri.obj
    gamedata/ground_tile.obj
    gamedata/letter_f.obj
    gamedata/minimalism.glsl
    gamedata/nesfont.png
    gamedata/person.obj
    gamedata/person_test.png
    gamedata/simpletex.png
    gamedata/test_post.obj
    gamedata/testlevel.scene.xml
    gamedata/testlevel.story.xml
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
      tinyxml
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
