import os, string, json
import codecs

def chunks(l, n):
    """Yield successive n-sized chunks from l."""
    for i in range(0, len(l), n):
        yield l[i:i + n]

with open('ld41_gamedata.json') as f:
    data = json.load(f)

    entries = data['entries']
    index = 0
    for entChunk in chunks( entries, 500 ):
        splitInfo = { 'entries' : entChunk }
        index += 1
        print "Chunk ", index, len(entChunk)
        with codecs.open('ld41_gamedata_%d.json' % index, 'w', encoding="utf-8") as outfile:
            json.dump( splitInfo, outfile, sort_keys = True, indent = 4, ensure_ascii = False)
