project ( WANDER )

include_directories( ${WANDER_SRC_DIR}/. )
include_directories( c:/toolkits/allegro-4.2.0/include )

link_directories( c:/toolkits/allegro-4.2.0/lib )
target_link_libraries( alleg )

add_executable( wander game_obj.cpp pixbot.cpp tilemap.cpp wander_main.cpp )