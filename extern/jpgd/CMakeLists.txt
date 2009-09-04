include_directories( inc )

SET ( jpgd_SOURCES
    jpgdlib/H2v2.cpp
    jpgdlib/idct.cpp
    jpgdlib/jidctfst.cpp
    jpgdlib/jpegdecoder.cpp )

ADD_LIBRARY( jpgd STATIC ${jpgd_SOURCES} )
