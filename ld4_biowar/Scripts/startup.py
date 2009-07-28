# Built in-s
import game, sys

# some of the python standard lib is copied into here
sys.path.append( './scripts/lib' )

# and game modules are here
sys.path.append( './scripts' )

####################
# redirect output to console

game.conout( "Executing startup script..." )

class ConsoleOut:
    def __init__( self, color=None ):
        self.color = color
        
    def write( self, msg ):
        if self.color:
            game.conout( msg, self.color )
        else:
            game.conout( msg )

sys.stdout = ConsoleOut()
sys.stderr = ConsoleOut( (255,0,0) )

# test map stuff
#import maptest
#maptest.makeTestMap()

# hand off control to gameplay
import gameplay
gameplay.start()



