# -*- coding: utf-8 -*-

# Debugger support
#import pdb; pdb.set_trace();
import sys
import json
import random
from google.appengine.ext import ndb

from flask import Flask, Response, render_template, request, redirect, url_for, jsonify


#import cloudstorage as gcs
#from werkzeug import secure_filename

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
    'sports' : 'Sports, Fitness',
    'music' : 'Rhythm, Music, Dance',
    'farming' : 'Farming, Gardening',
    'runner' : 'Infinite Runner',
    'typing' : 'Typing',

    'hunting' : 'Hunting',
    'sim' : 'Simulation',
    'retro' : 'Retro Arcade',
    'strategy' : 'Turn-Based Strategy',
    'towerdefense' : 'Tower Defense',
    'puzzle' : 'Puzzle',
    'fighting' : 'Fighting',
    'match3' : 'Match-3',
    'programming' : 'Programming',
    'pet' : 'Virtual Pet',
    'idle' : 'Clicker, Idle',
    'rampage' : 'Rampage, Kaiju, Destruction',
    'trivia' : 'Education, Trivia, Quiz',
    'idk' : "Don't Know",

    'other' : 'Other',
}


class CompoInfo(ndb.Model):
    compoNum = ndb.IntegerProperty()
    numGridGenres = ndb.IntegerProperty( indexed=False )
    gridCounts = ndb.BlobProperty( indexed=False )


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


@app.route('/')
def index():
    genres = getOrderedGenres()
    compo = getCompoInfo()

    # expand the genre counts
    ng = len(genres)
    genreCounts = [0] * (ng*ng)

    packedCount = bytearray(compo.gridCounts)
    print "Len packedCount", len(packedCount), len(genreCounts)
    for j in range(ng):
        for i in range(ng):
            ndx = j*ng+i
            genreCounts[ndx] = packedCount[ndx]

    return render_template( 'index.html', genres=genres, grid=genreCounts )

@app.route('/assign')
def assignPick():

    undefGenres = Genre.query(Genre.genreIdent=='unassigned').fetch()
    if len(undefGenres)==0:
        return "Need to init genres"
    undefGenre = undefGenres[0]

    gamesNeedAssign = Entry.query( Entry.genres==undefGenre ).fetch(limit=10)

    random.shuffle( gamesNeedAssign )

    return render_template( 'assign.html',
                            numAssigns=len(gamesNeedAssign),
                            games = gamesNeedAssign[:10]
                            )


def getOrderedGenres():
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
    if genreIDK:
        filteredGenres.append( genreIDK )


    return filteredGenres

@app.route('/combo/<int:gk1>/<int:gk2>')
def genreCombo( gk1, gk2 ):

    genre1 = Genre.get_by_id( gk1 )
    genre2 = Genre.get_by_id( gk2 )

    games = Entry.query( ndb.AND(Entry.genres== genre1),
                         ndb.AND(Entry.genres== genre2) ).fetch()

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

        print g
        gameJson = {
            "ldjamId" : g.ldjamId,
            "coverArt" : g.coverArt,
            "title" : g.gameTitle,
            "url" : g.url
        }
        gamesJson.append( gameJson )

    return jsonify( { "games" : gamesJson });
    # resp = Response(response=jsonify( { "games" : gamesJson }),
    #             status=200,
    #             mimetype="application/json")

    return resp

@app.route('/assign/<int:ldjamId>', methods=['POST','GET'])
def assignGame(ldjamId):

    games = Entry.query( Entry.ldjamId == ldjamId ).fetch()
    if len(games) == 0:
        return "Didn't find game id " + str(ldjamId)
    game = games[0]
    genres = getOrderedGenres()

    if request.method == 'POST':

        # Update the genres for this game
        assignedGenres = []
        for g in genres:
            genreKey = "genre_"+str(g.key.id())
            if request.form.has_key(genreKey) and request.form[genreKey]=='on':
                assignedGenres.append(g)
                print "Assigned ", g.genreName

            game.genres = assignedGenres
            game.put()


        # Redirect back to pick screen or another game if requested
        if request.form.has_key('do_pick'):
            return redirect( url_for('assignPick'))
        if request.form.has_key('do_random'):
            undefGenres = Genre.query(Genre.genreName=='UNASSIGNED').fetch()
            undefGenre = undefGenres[0]

            gamesNeedAssign = Entry.query( Entry.genres==undefGenre ).fetch()
            if (len(gamesNeedAssign)==0):
                return redirect( url_for('assignPick'))
            else:
                nextGame = random.choice(gamesNeedAssign)
                return redirect( url_for('assignGame',ldjamId=nextGame.ldjamId))


    currentAssigned = []
    for g in game.genres:
        print g.genreName
        if g.key:
            currentAssigned.append( g.key.id() )

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
        # TODO : bulk assign
        pass
    return render_template('bulk_assign.html')



@app.route('/add_genre', methods=['POST','GET'])
def add_genre():
    addedMsg = None

    genres = getOrderedGenres()
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

    genres = getOrderedGenres()
    compos = CompoInfo.query( CompoInfo.compoNum==compoNum ).fetch()

    if len(compos)<1:
        print "Making new summary object..."
        compo = CompoInfo( compoNum = 41, numGridGenres = len(genres) )
        compo.put()
    else:
        compo = compos[0]

    return compo

@app.route('/update_summary')
def update_summary():

    compo = getCompoInfo()

    genres = getOrderedGenres()

    # Fetch ALL the entries
    allEntries = Entry.query().fetch()


    # Update the compo grid
    ng = len(genres)
    gridCount = [[0 for x in range(ng)] for x in range(ng)]
    for i in range(ng):
        print i, genres[i].genreName
        for j in range(ng):
            if i>j:
                 gridCount[i][j] = gridCount[j][i]
            else:
                count = 0

                if (i==j):
                    for ent in allEntries:
                        entGenres = map( lambda x: x.genreName, ent.genres)
                        if len(entGenres)==1 and (entGenres[0]==genres[i].genreName):
                            count += 1
                else:
                    genre1 = genres[i].genreName
                    genre2 = genres[j].genreName
                    for ent in allEntries:
                        entGenres = map( lambda x: x.genreName, ent.genres)
                        if (genre1 in entGenres) and (genre2 in entGenres):
                            count += 1

                gridCount[i][j] = count

    print gridCount

    # Convert to a blob
    packedCounts = bytearray(ng*ng)
    for j in range(ng):
        for i in range(ng):
            packedCounts[j*ng+i] = gridCount[i][j]

    compo.gridCounts = str(packedCounts)
    compo.put()

    return "Updated..."





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

    allGenres = getOrderedGenres()

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

@app.route('/form')
def form():
    return render_template('form.html')

@app.route('/submitted', methods=['POST'])
def submitted_form():

    #print request.form
    name = request.form['name']
    email = request.form['email']
    site = request.form['site_url']
    comments = request.form['comments']

    return render_template( 'submitted_form.html',
                            name=name, email=email, site=site,
                            comments=comments)
