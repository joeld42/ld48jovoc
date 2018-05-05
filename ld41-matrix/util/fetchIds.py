import os, sys
import requests


if __name__=='__main__':
	
	outfile = open("ld41_ids.txt", "wt")

	batchSize = 50
	offset = 0
	while 1:
		r = requests.get('https://api.ldjam.com/vx/node/feed/73256/parent/item/game/compo+jam?limit=%d&offset=%d' % (batchSize, offset) )

		print "batch ", offset
		entries = r.json()['feed']
		if (len(entries)==0):
			break

		for item in entries:
			entryId = item['id']
			print entryId
			outfile.write("%d\n" % entryId)

		offset += batchSize


	outfile.close()