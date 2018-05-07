# -*- coding: utf-8 -*-

# Debugger support
#import pdb; pdb.set_trace();
import sys
import json
import time
import random
import logging
import itertools
import datetime

from google.appengine.ext import ndb

from flask import Flask, Response, render_template, request, redirect, url_for, jsonify


# Stuff TODO
#  - Adjust counts after each assigned genre
#  - Update_Summary with timer
#  - Progress bar and help wanted text



DEFAULT_GENRES = {
    'unassigned' : 'UNASSIGNED',

    'fishing' : 'Fishing',
    'card' : 'Card, Dice and Board Game',
    'fps' :'First Person Shooter',
    'rts' :'Real-Time Strategy',
    'cooking' :'Cooking, Crafting',
    'rpg' :'Role Playing Game',
    'dating' :'Dating Simulator',
    'racing' :'Racing, Driving, Vehicle',
    'shooter' :'Arcade Shooter, Bullet Hell',
    'dungeon' :'Dungeon Crawler',

    'horror' :'Horror, Zombie',
    'text' : 'Narrative, Text Adventure',
    'platformer' : 'Platformer',
    'roguelike' : 'Roguelike',
    'physics' : 'Physics Toy or Puzzle',
    'breakout' : 'Breakout',
    'battle' : 'Battle Royale',
    'sports' : 'Sports, Fitness',
    'music' : 'Rhythm, Music, Dance',
    'farming' : 'Farming, Gardening',
    'runner' : 'Infinite Runner',
    'typing' : 'Typing',

    'sim' : 'Simulation,City Builder',
    'retro' : 'Retro Arcade',
    'strategy' : 'Turn-Based Strategy',
    'towerdefense' : 'Tower Defense',
    'puzzle' : 'Puzzle, Match3',
    'fighting' : 'Fighting',
    'programming' : 'Programming',
    'pet' : 'Virtual Pet',
    'idle' : 'Clicker, Idle',
    'rampage' : 'Rampage, Kaiju, Destruction',
    'trivia' : 'Education, Trivia, Quiz',
    'idk' : "Dont Know",

    'other' : 'Other',
}


class CompoInfo(ndb.Model):
    compoNum = ndb.IntegerProperty()
    numGridGenres = ndb.IntegerProperty( indexed=False )
    gridCounts = ndb.BlobProperty( indexed=False )
    realAssignedCount = ndb.IntegerProperty( indexed=False )
    assignedCount = ndb.IntegerProperty( indexed=False )
    entryCount = ndb.IntegerProperty( indexed=False )
    needsRebuild = ndb.IntegerProperty( indexed=False )
    lastRebuildTime = ndb.DateTimeProperty( indexed=False )
    adminCode = ndb.StringProperty( indexed=False )


class Genre(ndb.Model):
    genreIdent = ndb.StringProperty()
    genreName = ndb.StringProperty()

#class GenreCombo(ndb.Model):

class Entry(ndb.Model):
    ldjamId = ndb.IntegerProperty()
    url = ndb.StringProperty(indexed=False)
    coverArt = ndb.StringProperty(indexed=False)
    gameTitle = ndb.StringProperty(indexed=False)
    genres = ndb.StructuredProperty( Genre, repeated=True)

# ===============================================
# ===============================================
app = Flask(__name__)
app.debug = True

@app.after_request
def add_cors(resp):
    """ Ensure all responses have the CORS headers. This ensures any failures are also accessible
        by the client. """
    resp.headers['Access-Control-Allow-Origin'] = request.headers.get('Origin','*')
    resp.headers['Access-Control-Allow-Credentials'] = 'true'
    resp.headers['Access-Control-Allow-Methods'] = 'POST, OPTIONS, GET'
    resp.headers['Access-Control-Allow-Headers'] = request.headers.get(
        'Access-Control-Request-Headers', 'Authorization' )
    # set low for debugging
    if app.debug:
        resp.headers['Access-Control-Max-Age'] = '1'
    return resp

def getAssignedPercent( compo ):
    if compo.assignedCount and compo.entryCount:
        return int((float(compo.assignedCount) / compo.entryCount)*100.0)
    else:
        return 0

# Real count is how many have been assigned at last update_summary
def getRealAssignedPercent( compo ):
    if compo.realAssignedCount and compo.entryCount:
        return int((float(compo.realAssignedCount) / compo.entryCount)*100.0)
    else:
        return 0

@app.route('/')
def index():
    genres = getOrderedGenres( False )
    compo = getCompoInfo()

    # expand the genre counts
    ng = len(genres)
    genreCounts = [0] * (ng*ng)

    packedCount = bytearray(compo.gridCounts)

    for j in range(ng):
        for i in range(ng):
            ndx = j*ng+i
            genreCounts[ndx] = packedCount[ndx]
    return render_template( 'index.html', genres=genres, grid=genreCounts,
                            assignedCount = compo.realAssignedCount,
                            entryCount = compo.entryCount,
                            assignPct = getRealAssignedPercent(compo) )

@app.route('/about')
def about():
    return render_template( 'about.html')

@app.route('/assign_howto')
def assignHowto():
    return render_template( 'assign_howto.html')

@app.route('/assign')
def assignPick():

    compo = getCompoInfo()

    undefGenres = Genre.query(Genre.genreIdent=='unassigned').fetch()
    if len(undefGenres)==0:
        return "Need to init genres"
    undefGenre = undefGenres[0]

    offs = 0
    gamesLeft = compo.entryCount - compo.assignedCount
    if gamesLeft > 20:
        offs = random.randint(0,gamesLeft-10)

    gamesNeedAssign = Entry.query( Entry.genres==undefGenre ).fetch(limit=10,offset=offs)

    random.shuffle( gamesNeedAssign )

    return render_template( 'assign.html',
                            numAssigns=len(gamesNeedAssign),
                            games = gamesNeedAssign[:10],
                            assignedCount = compo.assignedCount,
                            entryCount = compo.entryCount,
                            assignPct = getAssignedPercent(compo) )


def getOrderedGenres( forAssign ):
    genres = Genre.query().order(Genre.genreName).fetch()

    # remove 'UNASSIGNED' and put "other" at the end
    filteredGenres = []
    genreOther = None
    genreIDK = None
    for g in genres:
        if g.genreIdent == 'other':
            genreOther = g
        elif g.genreIdent == 'idk':
            genreIDK = g
        elif g.genreName=='UNASSIGNED':
            pass
        else:
            filteredGenres.append( g )

    if genreOther:
        filteredGenres.append( genreOther )
    if genreIDK and forAssign:
        # Only include "I don't know" in assign
        filteredGenres.append( genreIDK )


    return filteredGenres

@app.route('/combo/<int:gk1>/<int:gk2>')
def genreCombo( gk1, gk2 ):

    if not gk1 == gk2:
        genre1 = Genre.get_by_id( gk1 )
        genre2 = Genre.get_by_id( gk2 )

        games = Entry.query( ndb.AND(Entry.genres== genre1, Entry.genres== genre2) ).fetch()
    else:
        genre1 = Genre.get_by_id( gk1 )
        games = Entry.query(Entry.genres==genre1 ).fetch()

        # Filter games that have ONLY one genre
        games = filter( lambda g: len(g.genres)==1 and (g.genres[0].genreIdent == genre1.genreIdent), games )



    gamesJson = []
    #   ldjamId = ndb.IntegerProperty()
    # url = ndb.StringProperty(indexed=False)
    # coverArt = ndb.StringProperty(indexed=False)
    # gameTitle = ndb.StringProperty(indexed=False)
    # genres = ndb.StructuredProperty( Genre, repeated=True)
    for g in games:
        #print g.gameTitle.encode('utf-8')

        #gg = map (lambda x: x.key.id(), filter( lambda y: y.key != None, g.genres))
        #print list(g.genres), gg

        gameJson = {
            "ldjamId" : g.ldjamId,
            "coverArt" : g.coverArt,
            "title" : g.gameTitle,

            "url" :"https://ldjam.com" + g.url

            # DBG: Use assign URL -- todo make this switchable by user
            #"url" : url_for('assignGame',ldjamId=g.ldjamId)
        }
        gamesJson.append( gameJson )

    return jsonify( { "games" : gamesJson });

@app.route('/assign/<int:ldjamId>', methods=['POST','GET'])
def assignGame(ldjamId):

    games = Entry.query( Entry.ldjamId == ldjamId ).fetch()
    if len(games) == 0:
        return "Didn't find game id " + str(ldjamId)
    game = games[0]
    genres = getOrderedGenres( True )


    if request.method == 'POST':

        # Just go back without doing anything
        if request.form.has_key('back'):
            return redirect( url_for('assignPick'))

        compo = getCompoInfo()

        wasAssigned = False
        print game.genres
        if len(game.genres) != 1 or game.genres[0].genreIdent != 'unassigned':
            wasAssigned = True
        print "wasAssigned ", wasAssigned

        # Update the genres for this game
        assignedGenres = []
        for g in genres:
            genreKey = "genre_"+g.genreIdent
            if request.form.has_key(genreKey) and request.form[genreKey]=='on':
                assignedGenres.append(g)
                print "Assigned ", g.genreName

        if len(assignedGenres) > 0:
            game.genres = assignedGenres
            game.put()

            if not wasAssigned:
                compo.assignedCount = compo.assignedCount + 1

            compo.needsRebuild = 1
            compo.put()


        # Redirect back to pick screen or another game if requested
        if request.form.has_key('do_pick'):
            return redirect( url_for('assignPick'))
        if request.form.has_key('do_random'):
            undefGenres = Genre.query(Genre.genreName=='UNASSIGNED').fetch()
            undefGenre = undefGenres[0]

            offs = 0
            gamesLeft = compo.entryCount - compo.assignedCount
            if gamesLeft > 20:
                offs = random.randint(0,gamesLeft-10)

            print "Offset", offs
            gamesNeedAssign = Entry.query( Entry.genres==undefGenre ).fetch( offset=offs )
            if (len(gamesNeedAssign)==0):
                return redirect( url_for('assignPick'))
            else:
                nextGame = random.choice(gamesNeedAssign)
                return redirect( url_for('assignGame',ldjamId=nextGame.ldjamId))


    print "Current assigned"
    currentAssigned = []
    for g in game.genres:
        currentAssigned.append( g.genreIdent )

        print g.genreName, g.genreIdent

    #print "Game ", game.gameTitle, game.ldjamId, len(genres)
    # Split genres into 4 for formatting
    splitNum = (len(genres)+3) / 4
    splitGenres = []
    while len(genres):
        splitGenres.append( genres[0:splitNum])
        genres = genres[splitNum:]

    return render_template( 'assign_game.html', game=game,
                            currGenres=currentAssigned,
                            genres=splitGenres )


@app.route('/get_assigned')
def get_assigned():

    gamesWithGenres = filter( lambda x: len(x.genres) and x.genres[0].genreIdent != 'unassigned',  Entry.query( ).fetch() )
    print len(gamesWithGenres), "games assigned..."

    results = []
    for game in gamesWithGenres:
        genres = map( lambda x: x.genreIdent, game.genres )

        results.append( { 'ldjamId' : game.ldjamId, 'title' : game.gameTitle, 'genres' : genres })

    return jsonify( { 'games' : results })

@app.route('/bulk_assign', methods=['POST', 'GET'])
def bulk_assign():
    if request.method == 'POST':

        compo = getCompoInfo()

        genreList = getOrderedGenres( False )
        genres = {}
        for g in genreList:
            genres[g.genreIdent] = g

        file = request.files['file']
        data = json.loads( file.stream.read() )


        count = 0
        total = len(data['games'])
        print "Bulk Assign ", total
        for gameJson in data['games']:
            gg = Entry.query( Entry.ldjamId == gameJson['ldjamId'] ).fetch()
            if len(gg) == 0:
                logging.warning( "didn't find game for id " + str(gameJson['ldjamId']) + "  " +  gameJson['title'] )
                continue
            game = gg[0]

            genresForThisGame = []
            for gid in gameJson['genres']:
                if genres.has_key( gid ):
                    genresForThisGame.append( genres[gid] )

            if len(genresForThisGame):
                game.genres = genresForThisGame
                game.put()
                logging.info( "Assign " + game.gameTitle + " genres " + str(gameJson['genres']) )


        compo.needsRebuild = 1
        compo.put()

        return redirect( url_for('assignPick') )


    return '''
    <!doctype html>
    <title>Bulk Assign</title>
    <h1>Upload new Bulk Assign JSON</h1>
    <form method=post enctype=multipart/form-data>
      <p><input type=file name=file>
         <input type=submit value=Upload>
    </form>
    '''



@app.route('/add_genre', methods=['POST','GET'])
def add_genre():
    addedMsg = None

    genres = getOrderedGenres( False )
    for g in genres:
        print g, g.genreName

    if request.method == 'POST':
        genre_name = request.form['genre_name']
        genre_ident = request.form['genre_ident']
        genre = Genre( genreName = genre_name, genreIdent = genre_ident )
        genre.put()
        addedMsg = "Added genre "+genre_name

    return render_template( 'add_genre.html',
                            message=addedMsg,
                            genres = genres
                            )

    return result

def getCompoInfo():
    # Don't really need this because this only applies to ld41 but
    # need something to query the CompoInfo by
    compoNum = 41

    genres = getOrderedGenres( False )
    compos = CompoInfo.query( CompoInfo.compoNum==compoNum ).fetch()

    if len(compos)<1:
        print "Making new summary object..."
        compo = CompoInfo( compoNum = 41, numGridGenres = len(genres) )
        compo.put()
    else:
        compo = compos[0]

    return compo

def make_gkey( gi1, gi2 ):
    if (gi1 == gi2):
        return (gi1,)
    elif (gi1 < gi2):
        return (gi1, gi2 )
    else:
        return (gi2, gi1 )

def findsubsets(S,m):
    return set(itertools.combinations(S, m))

@app.route('/check_update')
def check_update():

    compo = getCompoInfo()

    if compo.lastRebuildTime:
        nextRebuild = compo.lastRebuildTime + datetime.timedelta( minutes=10)
    else:
        # just pack an arbitrary rebuild time if not set
        compo.lastRebuildTime = datetime.datetime.now() + datetime.timedelta( minutes=60 )
        compo.put()

    now = datetime.datetime.now()
    if nextRebuild > now:
        print "seconds until next ", (nextRebuild - datetime.datetime.now()).seconds
        timeUntilNext = int((nextRebuild - datetime.datetime.now()).seconds / 60.0)
    else:
        print "rebuild time passed"
        timeUntilNext = 0


    return render_template( 'update.html',
                            assignedCount = compo.realAssignedCount,
                            entryCount = compo.entryCount,
                            assignPct = getRealAssignedPercent(compo),
                            needsUpdate=compo.needsRebuild,
                            lastUpdate=compo.lastRebuildTime,
                            nextUpdate=nextRebuild,
                            timeUntilNext = timeUntilNext )


@app.route('/do_update', methods=['POST','GET'])
def do_update():

    start_time = time.time()

    compo = getCompoInfo()

    # Check if this was explictly requested
    shouldUpdate = False
    if request.method == 'POST':
        # if this is a requested update, check for the proper admin code
        if not request.form.has_key('update_now'):
            return redirect( url_for('check_update') )

        admincode = request.form.get( "admincode", "")
        shouldUpdate = True

    else:
        admincode = request.args.get('admincode')
        if not compo.needsRebuild:
            compo.lastRebuildTime = datetime.datetime.now()
            compo.put()
            return "Already up to date"

    # pseudosecurity..
    if admincode!=compo.adminCode:
        return "Wrong or missing Admin code..."


    genres = getOrderedGenres( False )

    # Fetch ALL the entries
    allEntries = Entry.query().fetch()

    # Count all genres
    genreCounts = {}

    for game in allEntries:
        entGenres = map( lambda x: x.genreIdent, game.genres)
        if len(entGenres)==1:
            gkey = make_gkey(entGenres[0], entGenres[0])
            genreCounts[gkey] = genreCounts.get( gkey, 0 ) + 1
        else:
            for g1, g2 in findsubsets( entGenres, 2 ):
                gkey = make_gkey(g1, g2 )
                genreCounts[gkey] = genreCounts.get( gkey, 0 ) + 1


    # Update the compo grid
    ng = len(genres)
    gridCount = [[0 for x in range(ng)] for x in range(ng)]
    for i in range(ng):
        print i, genres[i].genreName
        for j in range(ng):
            if i>j:
                 gridCount[i][j] = gridCount[j][i]
            else:
                genre1 = genres[i].genreIdent
                genre2 = genres[j].genreIdent
                gkey = make_gkey(genre1, genre2 )
                gridCount[i][j] = genreCounts.get( gkey, 0 )

    #print gridCount

    # Count unassigned
    assignedCount = 0
    for ent in allEntries:

        isUnassigned = False
        for g in ent.genres:
            if g.genreIdent=='unassigned':
                isUnassigned = True

        if not isUnassigned:
            assignedCount += 1

    # Update counts
    compo.realAssignedCount = assignedCount
    compo.assignedCount = assignedCount
    compo.entryCount = len(allEntries)
    compo.needsRebuild = 0
    compo.lastRebuildTime = datetime.datetime.now()
    compo.put()

    # Convert to a blob
    packedCounts = bytearray(ng*ng)
    for j in range(ng):
        for i in range(ng):
            packedCounts[j*ng+i] = gridCount[i][j]

    compo.gridCounts = str(packedCounts)
    compo.put()

    end_time = time.time()
    time_taken = end_time - start_time
    return "Summary updated (%d entries, time taken %3.2fs)..." % (len(allEntries), time_taken)





@app.route('/add_default_genres')
def add_default_genres():

    genres = list(Genre.query())

    genreIdents = map (lambda x: x.genreIdent, genres )

    defaultGenres = DEFAULT_GENRES.keys()
    defaultGenres.sort()

    for g in defaultGenres:
        if not g in genreIdents:
            gname = DEFAULT_GENRES[g]
            genre = Genre( genreIdent = g, genreName = gname )
            genre.put()

    return redirect( url_for('assignPick') )


@app.route('/load_entries', methods=['POST','GET'])
def upload():

    undefGenres = Genre.query(Genre.genreIdent=='unassigned').fetch()
    if len(undefGenres)==0:
        return "Need to init genres"

    undefGenre = undefGenres[0]

    allGenres = getOrderedGenres( True )

    if request.method == 'POST':
        file = request.files['file']
        data = json.loads( file.stream.read() )

        count = 0
        total = len(data['entries'])
        for entryJson in data['entries']:

            coverArt = None
            if entryJson.has_key('meta') and entryJson['meta'].has_key('cover'):
                coverArt = entryJson['meta']['cover']
                if coverArt.startswith('///'):
                    coverArt = 'https://static.jam.vg' + coverArt[2:]


            # DBG: Assign some genres at random
            if False:
                foo = random.randint(1,5)
                if foo==1:
                    genres = [undefGenre]
                else:
                    gg1 = random.choice( allGenres )
                    gg2 = random.choice( allGenres )

                    genres = [ gg1 ]
                    if (gg2.genreName != gg1.genreName):
                        genres.append( gg2 )
            else:
                # Start everything with undefined
                genres = [ undefGenre ]

            entry = Entry( ldjamId=entryJson['id'],
                           gameTitle=entryJson['name'],
                           url = entryJson['path'],
                           coverArt = coverArt,
                           genres = genres
                           )
            entry.put()

            count += 1
            if (count % 100==0):
                print "Added %d/%d entries (%3.2f%%)..." % ( count, total, 100.0*float(count)/total )

        return "%d entries..." % (count)


        # extension = secure_filename(file.filename).rsplit('.', 1)[1]
        # options = {}
        # options['retry_params'] = gcs.RetryParams(backoff_factor=1.1)
        # options['content_type'] = 'image/' + extension
        # bucket_name = "ld41-matrix"
        # path = '/' + bucket_name + '/' + str(secure_filename(file.filename))
        # if file:
        #     try:
        #         with gcs.open(path, 'w', **options) as f:
        #             f.write(file.stream.read())# instead of f.write(str(file))
        #             print jsonify({"success": True})
        #         return jsonify({"success": True})
        #     except Exception as e:
        #         logging.exception(e)
        #         return jsonify({"success": False})

    return '''
    <!doctype html>
    <title>Upload new File</title>
    <h1>Upload new File</h1>
    <form method=post enctype=multipart/form-data>
      <p><input type=file name=file>
         <input type=submit value=Upload>
    </form>
    '''
