########################
# test map stuff
import game, random, math

def rnd( mx ):
    return int(random.uniform(0,mx) )

def makeTestMap():
    map = game.newMap( 200, "test" )
    for j in range(200):
        for i in range(20):
            game.mapSet( map, i,j, 0 )

    # roads
    for roads in range(10):
        x = rnd(20)
        y0 =rnd(200)
        y1 =rnd(200)

        for y in range( y0, y1 ):
            game.mapSet( map, x, y, 2 )


    for roads in range(10):
        y = rnd(200)
        x0 =rnd(20)
        x1 =rnd(20)

        for x in range( x0, x1 ):
            game.mapSet( map, x, y, 2 )

    # lakes
    for lakes in range(15):
        lakepos = ( int(random.uniform(0,20)),
                    int(random.uniform(0,200)) )
        lakesz = int(random.uniform(0, 10 ))
        for j in range(200):
            for i in range(20):
                if (math.sqrt( (i-lakepos[0])**2 +
                               (j-lakepos[1])**2 ) < lakesz ):
                    game.mapSet( map, i,j, 1 )

    game.setActiveMap( map )
