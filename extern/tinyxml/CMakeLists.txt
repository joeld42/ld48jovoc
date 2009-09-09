include_directories( . )

SET  ( tinyxml_SOURCES
     tinystr.cpp
     tinyxml.cpp
     tinyxmlerror.cpp
     tinyxmlparser.cpp
    )

ADD_LIBRARY( tinyxml STATIC ${tinyxml_SOURCES} )
