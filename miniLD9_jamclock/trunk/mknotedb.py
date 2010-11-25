import os, sys, string
import xml.dom.ext
import xml.dom.minidom

# rather than figure out the logic in the main app, precompute all the scales and
# chords in this script and just read them
#
# The Scale shows up in the reference fretboard
#    -- Use to constrain the Chord Picker
#    -- 

CHROMATIC = [ 'A', 'A#', 'B',  'C',  'C#', 'D',  'D#',  'E',  'F',   'F#', 'G', 'G#' ]
STEPS     = [ 'R', 'b2', '2',  'b3', '3',  '4',  'b5',  '5',  '#5',  '6',  'b7', '7',
              '8', 'b9', '9',  '#9', '10', '11', '#11', '12', 'b13', '13'  ]

SCALES = [
    ( "Blues" ,           "R  b3  4   b5  5   b7" ),
    
    ( "Ionian" ,          "R  2   3   4   5   6   7" ),
    ( "Dorian" ,          "R  2   b3  4   5   6   b7"  ),
    ( "Phrygian" ,        "R  b2  b3  4   5   #5  b7" ),
    ( "Lydian" ,          "R  2   3   b5  5   6   7" ),
    ( "Mixolydian" ,      "R  2   3   4   5   6   b7" ),
    ( "Aeolian" ,         "R  2   b3  4   5   #5  b7" ),
    ( "Locrian" ,         "R  b2  b3  4   b5  #5  b7" ),
    
    ( "Diminished (H-W)", "R  b2  b3  3   b5  5   6   b7" ),
    ( "Diminished (W-H)", "R  2   b3  4   b5  #5  6   7" ),
    ( "Whole Tone" ,      "R  2   3   b5  #5  b7" ),
    ( "Melodic Minor" ,   "R  2   b3  4   5   6   7" ),
    ( "Dorian b2" ,       "R  b2  b3  4   5   6   b7" ),
    ( "Lydian Aug" ,      "R  2   3   b5  #5  6   7" ),
    ( "Lydian b7" ,       "R  2   3   b5  5   6   b7" ),
    ( "Mixolydian b6" ,   "R  2   3   4   5   #5  b7" ),
    ( "Locrian 2" ,       "R  2   b3  4   b5  #5  b7" ),
    ( "Altered" ,         "R  b2  b3  3   b5  #5  b7" ),
    ( "Harmonic Minor",   "R  2   b3  4   5   #5  7" ),
    ("Locrian 6",         "R  b2  b3  4   b5  6   b7" ),
    ("Ionian Aug",        "R  2   3   4   #5  6   7" ),
    ("Dorian #4",         "R  2   b3  b5  5   6   b7" ),
    ("Phrygian Major",    "R  b2  3   4   5   #5  b7" ),
    ("Lydian #9",         "R  b3  3   b5  5   6   7" ),
    ("Altered bb7",       "R  b2  b3  3   b5  #5  6" ),
    ("Pentatonic Major",  "R  2   3   5   6" ),
    ("Pentatonic Minor",  "R  b3  4   5   b7" ),
    ("Augmented",         "R  R   3   5   #5  7" ),
    ("Arabian",           "R   2   3   4   b5  #5  b7" ),
    ("Balinese",          "R   b2  b3  5   #5" ),
    ("Byzantine",         "R   b2  3   4   5   #5  7" ),
    ("Chinese",           "R   3   b5  5   7" ),
    ("Chinese Mongolian", "R   2   3   5   6" ),
    ("Double Harmonic",   "R   b2  3   4   5   #5  7" ),
    ("Egyptian",          "R   2   4   5   b7" ),
    ("Eight Tone Spanish","R   b2  b2  3   4   b5  #5  b7" ),
    ("Enigmatic",         "R   b2  3   b5  #5  b7  7" ),
    ("Hindu",             "R   2   3   4   5   #5  b7" ),
    ("Hirajoshi",         "R   2   b3  5   #5" ),
    ("Hungarian Major",   "R   b3  3   b5  5   6   b7" ),
    ("Hungarian Minor",   "R   2   b3  b5  5   #5  7" ),
    ("Hungarian Gypsy",   "R   2   b3  b5  5   #5  7" ),
    ("Ichikosucho",       "R   2   3   4   b5  5   6  7" ),
    ("Kumoi",             "R   2   b3  5   6" ),
    ("Leading Whole Tone","R   2   3   b5  #5  b7  7" ),
    ("Lydian Diminished", "R   2   b3  b5  5   6   7" ),
    ("Lydian Minor",      "R   2   3   b5  5   #5  b7" ),
    ("Mohammedan",        "R   2   b3  4   5   #5  7" ),
    ("Neopolitan",        "R   b2  b3  4   5   #5  7" ),
    ("Neopolitan Major",  "R   b2  b3  4   5   6   7" ),
    ("Neopolitan Minor",  "R   b2  b3  4   5   #5  b7" ),
    ("Overtone",          "R   2   3   b5  5   6   b7" ),
    ("Pelog",             "R   b2  b3  5   #5" ),
    ("Persian",           "R   b2  3   4   b5  #5  7" ),
    ("Prometheus",        "R   2   3   b5  6   b7" ),
    ("Prometheus Neopolitan", "R   b2  3   b5  6   b7" ),
    ("Purvi Theta",       "R   b2  3   b5  5   #5  7" ),
    ("Six Tone Symmetrical", "R   b2  3   4   #5  6" ),
    ("Todi Theta",        "R   b2  b3  b5  5   #5  7" )
    ]


CHORDS = [
    ("Major",        "",         "R   3   5"),
    ("Minor",        "m",        "R   b3  5"),
    ("Fifth",        "5",        "R  5"),
    ("sus4",         "sus4",     "R 4   5"),
    ("b5",           "b5",       "R   3   b5"),
    ("Diminished",   "dim",      "R   b3  b5"),
    ("Augmented",    "aug",      "R    3   #5"),
    ("Major6",       "maj6",     "R   3   5   6"),
    ("Minor6",       "m6",       "R   b3  5   6"),
    ("7",            "7",        "R    3   5   b7"),
    ("7sus4",        "7sus4",    "R    4   5   b7"),
    ("Minor7",       "m7",       "R   b3  5   b7"),
    ("Minor7(9)",    "m7(9)",    "(R)  b3  (5) b7  9"),
    ("Minor7(9,11)", "m7(9,11)", "(R)   b3  (5) b7  9   11"),
    ("Major7",       "maj7",     "R   3   5   7"),
    ("Major7(9)",    "maj7(9)",  "(R)  3   (5) 7   9"),
    ("Major7(#11)",  "maj7(#11)","(R)    3   (5) 7   9   #11"),
    ("Major7(13)",   "maj7(13)", "(R) 3   (5) 7   13"),
    ("Major7(9,13)", "maj7(9,13)", "(R)   3   (5) 7   9   13"),
    ("Major7#5",     "maj7#5",     "R 3   #5  7"),
    ("Major7#5(9)",  "maj7#5(9)", "(R)    3   #5  7   9"),
    ("MinMaj7",      "m/maj7",   "R  b3  5   7"),
    ("MinMaj7(9)",   "m/maj7(9)",   "(R) b3  (5) 7   9"),
    ("7b5",          "7b5",       "R  3   b5  b7"),
    ("Minor7b5",     "min7b5",    "R b3  b5  b7"),
    ("Aug7",         "aug7",      "R 3   #5  b7"),
    ("Dim7",         "dim7",      "R b3  b5  6"),
    ("Add9",         "add9",      "R 3   5   9"),
    ("MinorAdd9",    "m9",        "R    b3  5   9"),
    ("Major6(9)",    "6/9",       "(R)  3   (5) 6   9"),
    ("Minor6(9)",    "m6/9",      "(R)  b3  (5) 6   9"),
    ("7(9)",         "9",         "(R)   3   (5) b7  9"),
    ("7(b9)",        "7b9",       "(R)  3   (5) b7  b9"),
    ("7(#9)",        "7#9",       "(R)  3   (5) b7  #9"),
    ("7(13)",        "13",        "(R)  3   (5) b7  13"),
    ("7(b13)",       None,     "(R) 3   (5) b7  b13"),
    ("7(9,13)",      None,   "(R)    3   (5) b7  9   13"),
    ("7(b9,13)",     None,   "(R)   3   (5) b7  b9  13"),
    ("7(#9,13)",     None, "(R)   3   (5) b7  #9  13"),
    ("7(b9,b13)",    None, "(R)  3   (5) b7  b9  b13"),
    ("7(#9,b13)",    None, "(R)  3   (5) b7  #9  b13"),
    ("7(9,#11)",     None, "(R)   3   (5) b7  9   #11"),
    ("7(9,#11,13)",  None, "(R)    3   (5) b7  9   #11 13" )
    ]

# For each Key...
#   -- A fretboard map of the scale
      
print "Total scales", len(SCALES) * len(CHROMATIC)

doc = xml.dom.minidom.Document()

musicdb = doc.createElement( "MusicData" )
doc.appendChild( musicdb )

stepIndex = {}

for index in range(len(STEPS)):
    stepIndex[ STEPS[index] ] = index

for sinfo in SCALES:
    sname, snotes = sinfo

    for rndx in range(len(CHROMATIC)):

        r = CHROMATIC[rndx]
        
        scale = doc.createElement( "Scale" )
        scale.setAttribute( "root", r );
        scale.setAttribute( "name", sname );

        # Add Scale tones
        scaleNotesRel = string.split( string.strip( snotes ) )
        scaleNotes = []
        for n in scaleNotesRel:
            noteNdx = (rndx + stepIndex[n]) % 12
            #print n, CHROMATIC[noteNdx]
            note = doc.createElement( "Note" )            
            note.setAttribute( "val", CHROMATIC[noteNdx] )

            scaleNotes.append( CHROMATIC[noteNdx] )

            scale.appendChild( note )

        # Add Chords for this scale
        #print scaleNotes
        for cname, cAbbr, cnotes in CHORDS:

            if cAbbr is None:
                cAbbr = cname

            for chordRootIndex in range( len(CHROMATIC)):
                
                chordRoot = CHROMATIC[chordRootIndex]
                chordTones = []
                
                # Are all the chord notes in this scale?
                isChordInScale = True
                for n in string.split( string.strip( cnotes) ):
                    n = n.replace( "(", "" )
                    n = n.replace( ")", "" )

                    if not stepIndex.has_key(n):
                        print "unknown interval ", n
                        isChordInScale = False
                        break
                    
                    noteNdx = (stepIndex[n] + chordRootIndex) % 12
                    note = CHROMATIC[noteNdx]

                    chordTones.append( note )
                    
                    if not note in scaleNotes:
                        isChordInScale = False
                        break

                if isChordInScale:
                    #print chordRoot," ", cname, chordTones
                    chordElem = doc.createElement( "Chord" )
                    chordElem.setAttribute( "root", chordRoot )
                    chordElem.setAttribute( "notes", string.join( chordTones, " " ))
                    chordElem.setAttribute( "desc", "%s %s" % (chordRoot, cname) )
                    chordElem.setAttribute( "name", "%s%s" % (chordRoot, cAbbr) )
                    scale.appendChild( chordElem )
                
        
            
        #sys.exit(1)
        
        musicdb.appendChild( scale )


fp = open("musicdb.xml", "w")
xml.dom.ext.PrettyPrint( doc, fp )
fp.close()
