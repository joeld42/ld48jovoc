# -*- coding: utf-8 -*- 
import os, sys
import string
import requests
import json
import codecs
import time

batchSize = 50
ids = list()
with open('ld41_ids.txt', 'r') as fp:
    for line in fp.readlines():
    	ids.append( string.strip(line))

gameInfos = []

#print "len ids", len(ids)
total = len(ids)
while len(ids) > 0:
	batchIds = ids[:batchSize]
	ids = ids[batchSize:]

	fetchURL = "https://api.ldjam.com/vx/node/get/" + string.join( batchIds, "+" )
	r = requests.get( fetchURL )
	#print batchIds, len(ids)

	rjson = r.json()['node']	

	gameInfos = gameInfos + rjson

	print len(gameInfos), "fetched,", len(ids), "remaining. (%3.2f%%)" % ( 100.0*float(len(gameInfos)) / total )
	time.sleep(0.5)


gameInfoJson = { "entries" : gameInfos }
with codecs.open('ld41_gamedata.json', 'w', encoding="utf-8") as outfile:

    json.dump( gameInfoJson, outfile, sort_keys = True, indent = 4, ensure_ascii = False)
