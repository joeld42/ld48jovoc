import game, sys


# virus names
ANTARAX  = 0
ZOMBY    = 1
GUTSPEW  = 2
PHAGE    = 3
NUPLAGUE = 4

# antibodies
ANTI_ANTARAX  = 5
ANTI_ZOMBY    = 6
ANTI_GUTSPEW  = 7
ANTI_PHAGE    = 8
ANTI_NUPLAGUE = 9

class TradeRoute:
    def __init__( self, cfrom, cto ):
        self.cfrom = cfrom
        self.cto = cto

        infection_rate = [ 0, 0, 0, 0, 0 ]

class City:
    def __init__( self, name, loc ):
        self.name = name
        self.loc = loc

        # virus and antibody
        self.supply = [0,0,0,0,0,  0,0,0,0,0]


class GameShell:
    def __init__( self ):
        self.foo = 1

    def buildMainMenu(self):
        game.addMainMenuItem( "Start Game", self.startGame )
        game.addMainMenuItem( "High Scores", self.hiscores )
        game.addMainMenuItem( "Quit", self.quit )

    def quit( self ):
        print "In Quit..."
        sys.exit(0)

    def startGame( self ):
        print "In Stargame..."
        game.setGameMode( "city" )

        # init cities
        self.cities = []
        self.cities.append( City( "Zaxxon Base", (35, 30) ) )
        self.cities.append( City( "Helios", (80, 103) ) )
                            

    def hiscores( self ):
        print "In highScores..."

def start():
    gm = GameShell()
    gm.buildMainMenu()
