#!/usr/bin/env python
import os, sys
import traceback
import inspect

try:
    import jinja2
    import curses
    import curses.wrapper
    import curses.textpad
except ImportError:

    exc_type, exc_value, exc_traceback = sys.exc_info()
    traceback.print_exception( exc_type, exc_value, exc_traceback, 
                               limit=2 )

    print """
 ==============================================
 This script requires the 'jinja2' and 'curses' 
 python modules.
 
 jinja2 may be obtained from:
   http://jinja.pocoo.org/2/

 curses is standard on linux and mac, on windows you may
 use the 'wcurses' module; more info here:
   http://adamv.com/dev/python/curses/

 =============================================="""
    sys.exit(1)

FEATURES = [
    ( 'gamedata', 'Game Data', 
      ('Some example game data, including a basic font'+
       'and sprite and stuff') ),

    ( 'vcproj', 'Visual Studio Project',
      'A project to build the game with SDL/Visual Studio' ),

    ( 'iphone', 'iPhone Project',
      'An xcode project to build the game for iPhone' ),

]

class Project( object ):

    def __init__( self ):
        self.name = None
        self.dev  = 'Your Name Here'
        self.baseDir = "."

        self.features = {}
        for f in FEATURES:
            self.features[f[0]] = True

    def create( self ):
        env = jinja2.Environment( 
            loader=jinja2.FileSystemLoader( self.templateDir )
            )

        # Create directories
        projDir = os.path.abspath( os.path.join(self.baseDir, self.name) )
        os.mkdir( projDir )

        vars = {}
        vars['name'] = self.name
        vars['dev' ] = self.dev

        # test file
        template = env.get_template( 'main.cpp' )
        print template.render( vars )
        

class LudditeWizard(object):

    def __init__( self, stdscr, proj ):
        self.stdscr = stdscr;

        self.proj = proj

        # clear screen, draw background
        stdscr.clear()
        self.winsz = stdscr.getmaxyx()
        stdscr.border()

    def updateDirName( self ):
        dirstatus = self.stdscr.derwin( 1, self.winsz[1]-6,
                                        self.winsz[0]-2, 5 )
        dirstatus.clear()

        dirstatus.addstr( 0,0,
                          "Target Dir: " +
                          os.path.abspath( os.path.join( self.proj.baseDir, 
                                                         self.proj.name ) )
                            )
        self.stdscr.refresh()

    def featuresMenu( self, targbox ):        

        curry = 1
        for f in FEATURES:

            attr = curses.color_pair(1)
            if (curry -1)==self.currFeature:
                attr = attr | curses.A_REVERSE

            targbox.addstr( curry, 4, "[X] "+f[1] + " "*(30-len(f[1])), 
                            attr )
            curry += 1

        targbox.addstr( curry, 10, "[ DONE ] " )



    def run( self ):

        # Normal widgets
        curses.init_pair( 1, curses.COLOR_WHITE, 
                          curses.COLOR_BLUE )

        # Draw the form
        infobox = self.stdscr.derwin( 5, 40, 
                                      (self.winsz[0]/2)-10,
                                      (self.winsz[1]/2)-20 )
        infobox.border()
        infobox.addstr( 0, 2, " Project Info ", curses.color_pair(1) )

        infobox.addstr( 1, 4, " Game Name : " )
        infobox.addstr( 2, 4, " Developer : " )
        infobox.addstr( 3, 4, "  Base Dir : " )

        self.currFeature = -1
        targbox = self.stdscr.derwin( 3 + len(FEATURES), 40,
                                      (self.winsz[0]/2)-5,
                                      (self.winsz[1]/2)-20 )
        targbox.border()
        targbox.addstr( 0, 2, "Features" )

        self.featuresMenu ( targbox )
        self.stdscr.refresh()

        # Fill in project name
        nameedit = infobox.derwin( 1, 22, 1, 17 )
        tb = curses.textpad.Textbox( nameedit )
        self.proj.name = tb.edit()

        nameedit = infobox.derwin( 1, 22, 2, 17 )
        tb = curses.textpad.Textbox( nameedit )
        self.proj.dev = tb.edit()

        self.updateDirName()
        while True:
            nameedit = infobox.derwin( 1, 22, 3, 17 )
            nameedit.clear()
            nameedit.addstr( 0, 0, self.proj.baseDir )

            tb = curses.textpad.Textbox( nameedit )
            self.proj.baseDir = tb.edit()
            if self.proj.baseDir == '':
                self.proj.baseDir = '.'

            self.updateDirName()
            if (os.path.exists(self.proj.baseDir)):
                break
        
        # TODO: make checkboxes go
        self.currFeature = 0
        while True:
            self.featuresMenu( targbox )
            
            cg = self.stdscr.getch()



def gui_main( stdscr, proj ):
    
    wizard = LudditeWizard( stdscr, proj )
    wizard.run()

def main():

    proj = Project()

    # Find where the script is so we can find the templates
    srcfile = inspect.currentframe().f_code.co_filename
    srcfile = os.path.abspath(srcfile)
    templateDir = os.path.join( os.path.split( srcfile )[0], "templates" )    

    if not os.path.exists( templateDir):
        print "Can't find template dir, tried: ", templateDir
        os.exit(1)

    proj.templateDir = templateDir

    # play nicely with terminal
    curses.wrapper( gui_main, proj )

    proj.create()


if __name__=='__main__':
    main()
