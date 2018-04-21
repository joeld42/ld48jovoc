import os, sys

def doCommand( msg, cmd ):

	print "----", msg, "---"
	print cmd
	result = os.system(cmd)
	if result != 0:
		print "ERROR: Exit code ", result
		sys.exit(result)

if __name__=='__main__':

	doCommand("Sync working code with ld48jovoc",
			  "rsync -rtvu /Users/joeld/oprojects/ld41_cardfish/ /Users/joeld/Projects/ld48jovoc/ld41_cardfish")

	if ("-publish" in sys.argv):
		doCommand("Upload to site",
				"rsync -avz -L -e ssh /Users/joeld/oprojects/fips-deploy/ld41_cardfish/emsc-make-release/ vczdj@clockwatching.net:/home/vczdj/tapnik.com/ld41/")
	else:
		print "Not uploading..."

