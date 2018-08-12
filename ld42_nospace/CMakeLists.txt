cmake_minimum_required (VERSION 2.8.11)
project (LD42NOSPACE)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

include_directories(
	/Users/joeld/Toolkits/raylib2/raylib/src/
	/Users/joeld/Toolkits/raylib2/raygui/src/
	/usr/local/include
)

link_directories(
	/Users/joeld/Toolkits/raylib2/raylib/build/release
	/usr/local/Cellar/glfw3/3.2.1/lib 	
)

add_executable (ld42_nospace 
				ld42_nospace.c
				)

set ( LIBS ${LIBS}
		"-framework OpenGL"
		"-framework OpenAl"
		"-framework Cocoa"
		"-framework IOKit"
		"-framework CoreVideo"
		"-framework AudioToolbox"
		raylib
		glfw3
      )

target_compile_options( ld42_nospace PUBLIC -fno-rtti)
target_link_libraries( ld42_nospace ${LIBS} )
set_target_properties( ld42_nospace PROPERTIES
		       		 RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/ )