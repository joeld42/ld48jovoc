import os, sys, string
import ID3

# todo: use Tkinter to make a browser
if len(sys.argv) == 1:
    print "To run this, give the path to the base directory which"
    print "contains mp3 music files."
    print
    print "for example:"
    print "musicdb.py \"C:\Documents and Settings\jdavis\My Documents\My Music\iTunes\""
    print

    # pause here in cause user started it from explorer
    os.system( "pause" )
    sys.exit(1)

basepath = sys.argv[1]
basepath = string.replace( basepath, "\\", "/" )

print "Scanning ", basepath
print "-------------------------------------"

musiclist = []
for root, dirs, files in os.walk( basepath ):
    
    print ">>", root[ len(basepath)+1 : ]

    # skip directories with non-music stuff
    rootlower = root.lower()
    skip_it = False
    for skipword in ['podcast', 'audiobook' ]:
        if (string.find( rootlower, skipword ) != -1 ):
            skip_it = True

    if (skip_it):
        continue
    
    for f in files:        
        
        if (os.path.splitext(f)[-1] == '.mp3'):
            print "     ", f
            id3info = ID3.ID3( root + "\\" + f );
            try:
                title = id3info['TITLE']
            except KeyError:
                print "Guessing title..."
                title = f[:-3]

            try:
                artist = id3info['ARTIST']
            except KeyError:
                artist = 'Unknown'

            fullpath = root + "\\" + f 
                
            #print "Title: ", title
            #print "Artist: ", artist
            #print "Path: ", fullpath
            #print

            musiclist.append( ( artist, title, fullpath) )


if (len(musiclist)==0):
    print "No mp3 files found. Music list not created."
else:
    musiclist.sort()
    fp = open( "musicdb.txt", "wt")
    for m in musiclist:
        fp.write( m[0] + "\t" + m[1] + "\t" + m[2] + "\n" )

    fp.close

    print "musicdb.txt created with " + str(len(musiclist)) + " entries\n"
