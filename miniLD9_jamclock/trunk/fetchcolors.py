import os, sys, time
import unicodedata
import urllib2

import xml.dom.minidom
import xml.dom.ext


BASE_URL="http://www.colourlovers.com/api/palettes/top"

BATCH_SIZE=20

def getText( nodelist ):

    rc = ""
    for node in nodelist:
        
        if node.nodeType == node.TEXT_NODE:
            rc = rc+node.data
            

    return rc


colordoc = xml.dom.minidom.Document()

colordb = colordoc.createElement("Palettes")


# pages of results
for x in range(100):

    # Get the page
    response = urllib2.urlopen( 
        BASE_URL+"?numResults=%d&resultOffset=%d" % (BATCH_SIZE, BATCH_SIZE*x))

    cacheName = "cache/results%d.xml" % (BATCH_SIZE*x)

    print "palettes %d - %d\n" % ( x*BATCH_SIZE, (x+1)*BATCH_SIZE ),

    if (os.path.exists( cacheName )):
        print "(cached)"
        pallyXml = open(cacheName).read()
    else:
        print "...http"
        time.sleep( 10 )
        pallyXml = response.read()    
        fp = open( cacheName, "w" )
        fp.write( pallyXml )
        fp.close()

    pallyXml = pallyXml.decode( 'iso-8859-1' )
    pallyXml = unicodedata.normalize('NFKD', pallyXml).encode( 'ascii', 'ignore' )
    pallyDoc = xml.dom.minidom.parseString( pallyXml )
    
    for palElem in pallyDoc.getElementsByTagName('palette'):
        
        # Pull some info out of the pallete
        destPally = colordoc.createElement( "palette" )
        
        # pull info from the palette
        title = palElem.getElementsByTagName("title")[0].childNodes[0].wholeText.encode('ascii', 'ignore')
        userName = palElem.getElementsByTagName("userName")[0].childNodes[0].wholeText.encode('ascii', 'ignore' )
        id = palElem.getElementsByTagName( "id")[0].childNodes[0].wholeText.encode( 'ascii', 'ignore' )

        colors = palElem.getElementsByTagName("colors")[0]
        destPally.appendChild( colors )

        destPally.setAttribute( "title", title )
        destPally.setAttribute( "user", userName )
        destPally.setAttribute( "id", id )

        print "Title:", title
        print "UserName:", userName
        print "-"*30

        colordb.appendChild( destPally ) 

    # write the while colleciton to a file
    fp = open( "colors.xml", "w" )
    xml.dom.ext.PrettyPrint( colordb, fp )
    fp.close()


# write the while colleciton to a file
fp = open( "colors.xml", "w" )
xml.dom.ext.PrettyPrint( colordb, fp )
fp.close()
