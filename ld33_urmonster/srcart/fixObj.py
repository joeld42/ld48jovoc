import os, sys



vts = [ (0.0, 1.0), (0.125, 1.0), (0.125, 1.0-0.125), (0.0, 1.0-0.125) ]

faceCount = 0
vtCount = 0;
firstFace = True
for line in open('hugzilla_land.obj'):
	line = line.strip()
	
	if line.startswith("f "):	
		
		if firstFace:
			firstFace = False
			for vtCount in range(196):
				for vndx in range(4):
					vt = vts[ vndx ]
					print "vt %f %f" % (vt[0], vt[1])
				print

		line2 = "f "
		c = (faceCount * 4) + 1
		for elem in line.split(" ")[1:]:			
			v = elem.split("/")
			v2 = (v[0], str(c), v[2])
			line2 += "/".join(v2) + " "
			c += 1

		print line2		
		faceCount += 1

	elif line.startswith("vt "):
		pass
	else:
		print line

print  "# Face count: ", faceCount
