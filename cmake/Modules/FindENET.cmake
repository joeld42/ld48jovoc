# - Find ENET library
# Find the native ENET includes and library
# This module defines
#   ENET_INCLUDE_DIR, where to find enet/enet.h
#   ENET_LIBRARIES libraries to link against to use ENET
#   ENET_FOUND If false, do not try to use ENET
# $ENETDIR is an environment variable that would correspond
# to the prefix used to configure ENET.
#
# Created by Olivier Delannoy. This was influenced by
# FindOpenAL.cmake module. MACOSX is not yet handled correctly

# message ("env  " $ENV{ENETDIR} )

FIND_PATH(ENET_INCLUDE_DIR enet/enet.h
        $ENV{ENETDIR}/include
       ~/Library/Frameworks/GLFW.framework/Headers
	    /Library/Frameworks/GLFW.framework.Headers # Tiger
	    /usr/local/include
	    /usr/local/X11R6/include
	    /usr/X11R6/include
	    /usr/X11/include
	    /usr/include/X11
	    /usr/include
	    /opt/X11/include
	    /opt/include
        )

FIND_LIBRARY(ENET_LIBRARIES NAMES enet PATHS
	    $ENV{ENETDIR}
	    $ENV{ENETDIR}/lib
	    /usr/local/lib
	    /usr/local/X11R6/lib
	    /usr/X11R6/lib
	    /usr/X11/lib
	    /usr/lib/X11
	    /usr/lib
	    /opt/X11/lib
	    /opt/lib
        )

message( "enet libraries: " ${ENET_LIBRARIES} )	

SET(ENET_FOUND "NO")
IF (ENET_LIBRARIES)
      SET(ENET_FOUND "YES")
ENDIF(ENET_LIBRARIES)
MARK_AS_ADVANCED(ENET_INCLUDE_DIR ENET_LIBRARIES)

