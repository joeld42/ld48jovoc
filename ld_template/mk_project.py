import os, sys
import shutil
import readline

TEMPLATE_FILES = [
	'CMakeLists.txt',
	'gamedata/nesfont.png',
	'gamedata/simpletex.png',
	'gamedata/template.glsl',
	'nesfont.cpp',
	'template_game.cpp',
	'template_game.h',
	'template_main.cpp'
]


class LDProject (object):
	def __init__( self ):
		self.ludumNumber = 99
		self.projectId = 'ludtest'
		self.projectName = 'Test Project'
		self.classPrefix = 'TestProj'

	def ldProject( self ):
		return 'ld{0}_{1}'.format( self.ludumNumber, self.projectId ) 

	def basePath( self ):
		return os.path.abspath( os.path.join( '..', self.ldProject() ) )

	def buildProject( self ):

		projPath = self.basePath()

		for f in TEMPLATE_FILES:
			srcFile = f			

			destFile = os.path.join( projPath, f.replace( 'template', self.projectId ) )

			self.buildFile( srcFile, destFile )

	def buildFile( self, srcFile, destFile ):

		if (not os.path.exists(srcFile) ):
			print "WARN: can't file template file", srcFile

		destPath, destFileName = os.path.split( destFile )

		# ensure the target directory exists
		if not os.path.exists( destPath ):
			os.makedirs( destPath )

		# Process the file
		print destFile
		destExt = os.path.splitext( destFile )[-1] 
		if destExt in ['.png','.jpg' ]:
			# Copy binary files unmodified
			shutil.copyfile( srcFile, destFile )
		else:
			# Process dest file
			fpDest = open( destFile, "w")
			fpSrc = open( srcFile, "r")
			for l in fpSrc:

				# Replace strings. Could be more elegant with re stuff but
				# this should work OK for now
				l = l.replace( 'ld_template', self.ldProject() )

				# didn't really think this through for c++, instead, use template_
				# to if it's c++ code
				if destExt in ['.h', '.cpp']:
					l = l.replace( 'template_', self.projectId+"_" )
					l = l.replace( 'template.', self.projectId+"." )
				else:
					l = l.replace( 'template', self.projectId )

				l = l.replace( '{{number}}', str(self.ludumNumber) )
				l = l.replace( '{{Project}}', self.projectName )
				l = l.replace( 'Template', self.classPrefix )

				fpDest.write( l )


			fpDest.close()
			fpSrc.close()



if __name__=='__main__':

	proj = LDProject();

	# get ludum dare ludumNumber
	ludnum = raw_input( "Ludum Dare Number [{0}]: ".format( proj.ludumNumber) )
	if ludnum:
		proj.ludumNumber = int(ludnum)

	# get project identifier
	projId = raw_input( "Project Identifier [{0}]: ".format( proj.projectId ) )
	if projId:
		proj.projectId = projId

	# Get Project Name
	displayName = raw_input( "Display Name [{0}]: ".format( proj.projectName ) )
	if displayName:
		proj.projectName = displayName	

	# Get Class name (e.g. TestProjGame)
	classPrefix = raw_input( "Class Prefix [{0}]: ".format( proj.classPrefix ) )
	if classPrefix:
		proj.classPrefix = classPrefix	


	# Print out final project info
	print "="*40
	print "Project Name : ", proj.projectName
	print "Base Path    : ", proj.basePath()
	print "Game Class   : ", proj.classPrefix + "Game"
	print "="*40

	yn = raw_input( "Continue [y/n]? " )
	if (yn != 'y'):
		print "Aborting..."
		sys.exit(1)

	# Make project 
	proj.buildProject()

	# Done!
	print "="*40
	print "Done! Remember you still need to do by hand: "
	print " - Add {0} to the toplevel CMakeLists.txt ".format( proj.ldProject() )
	print " - Add 'Copy Bundle Resources' step to xcode project and add gamedata"
	print " - Replace nesfont with something else."
	print "Good Luck!"



