import os, sys

def doCommand( msg, cmd ):

	print "----", msg, "---"
	print cmd
	result = os.system(cmd)
	if result != 0:
		print "ERROR: Exit code ", result
		sys.exit(result)

if __name__=='__main__':

	#doCommand("Sync working code with ld48jovoc",
	#		  "rsync -rtvu c:/oprojects/ld45_nothing/ /c:/ld48jovoc/ld45_nothing")
	doCommand("Copy working code to ld48jovoc",
		"xcopy c:\\oprojects\\ld45_nothing\\ c:\\Projects\\ld48jovoc\\ld45_nothing\\" )

	#if ("-publish" in sys.argv):
	#	doCommand("Upload to site",
	#			"rsync -avz -L -e ssh /Users/joeld/oprojects/fips-deploy/ld45_nothing/emsc-make-release/ vczdj@clockwatching.net:/home/vczdj/tapnik.com/ld45_nothing/")
	#else:
	#	print "Not uploading..."

