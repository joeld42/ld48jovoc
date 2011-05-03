include_directories( . )

message( "In glsw cmake" )

SET  ( glsw_SOURCES
     bstrlib.c
     glsw.c

   bstrlib.h
   glsw.h
    )

ADD_LIBRARY( glsw STATIC ${glsw_SOURCES} )
