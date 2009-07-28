#!/usr/bin/env python
import random
import os, sys, string
import Tkinter, tkMessageBox

class IceFloe:
    def __init__( self ):
        self.health = 1.0
        self.height = 0.05
        self.type = 'Ice'
        self.breakable = 0
        self.anchored = 1        

        self.pnts = []

    def getCenter( self ):
        c = ( 0, 0 )
        for p in self.pnts:
            c = (c[0]+p[0], c[1]+p[1])

        if len(self.pnts):
            c = (c[0]/len(self.pnts),
                 c[1]/len(self.pnts))
        return c

class Location:
    def __init__( self ):
        self.loc = ( 0, 0 )
        self.type = 'Item'
        self.color = '0x888888'
        self.name = 'Null'

class RoadTile:
    def __init__( self ):
        self.pylon = 0
        self.road  = 0

class TIEedit:
    def __init__( self ):
        self.icefloes = []
        self.currFloe = None
        self.currFloeNdx = -1
        self.mapsize = ( 10, 10 )

        self.locations = []
        self.locTypes = [
            ( 'StartPos', '#00ff00' ),
            ( 'Sheep', '#eeeeee' ),
            ( 'Crate', '#ffaa00' )
            ]

        self.locColor = {}
        for i in self.locTypes:
            self.locColor[i[0]] = i[1]

        self.c_blocksize = (70, 70)

        self.road = {}

        self.mode = 'Select'

    def getRoad( self, i, j ):
        if self.road.has_key( (i,j) ):
            return self.road[(i,j)]
        else:
            road = RoadTile()
            self.road[(i,j)] = road

            return road

    def setRoad( self, i, j, road ):
        self.road[(i,j)] = road

    def changeFloeType( self ):
        print 'floeType is ', self.floeType.get()
        if self.currFloe:
            self.currFloe.type = self.floeType.get()
            self.redrawFloe( self.currFloeNdx )
        
    def changeDrawMode( self ):
        for i in range( len(self.icefloes) ):            
            self.redrawFloe( i )

    def drawRoads( self ):
        self.canv.delete( 'road' )
        if self.doDrawRoads.get():
            xx = float(self.canv['width']) / self.mapsize[0]
            yy = float(self.canv['height']) / self.mapsize[1]

            for i in range( self.mapsize[0] ):
                for j in range( self.mapsize[1] ):
                    road=self.getRoad( i, j )
                    if road.road:                        
                        self.canv.create_rectangle( i*xx, j*yy,
                                                    (i+1)*xx, (j+1)*yy,
                                                    fill='grey30',
                                                    outline='black',
                                                    tag='road')
                    if road.pylon:
                        spc = (self.c_blocksize[0]/4, self.c_blocksize[1]/4)
                        self.canv.create_rectangle( i*xx + spc[0],
                                                    j*yy + spc[1],
                                                    (i+1)*xx - spc[0],
                                                    (j+1)*yy - spc[1],
                                                    fill='gray85',
                                                    outline='white',
                                                    tag='road' )
            

    def saveLevel( self ):
        levelfn = ( './gamedata/levels/%s.txt' %
                    self.lvlName.get() )
        if os.path.exists( levelfn ):
            if not tkMessageBox.askyesno( "Overwrite",
                                            "%s exists, overwrite?" % levelfn):
                return

        fp = open( levelfn, 'w' )
        fp.write('mapsize %d %d\n' % (self.mapsize[0], self.mapsize[1] ) )
        fp.write('blocksize %d %d\n' % (self.c_blocksize[0], self.c_blocksize[1] ) )
        fp.write('level_desc %s\n' % self.lvlDesc.get() )
        fp.write('level_playable %s\n'% self.lvlFinal.get() )
        fp.write('victory %s\n' % self.Victory.get() );

        fp.write('num_floes %d\n' % len(self.icefloes) )
        for floe in self.icefloes:
            fp.write('floe %f %f %d %d %s %d\n' %
                     ( floe.health, floe.height,
                       floe.breakable, floe.anchored,
                       floe.type, len(floe.pnts) ))
            for p in floe.pnts:
                fp.write('%f %f\n' % p )
                
        fp.write('road_tiles %d\n' % (self.mapsize[0]*self.mapsize[1]) )
        for j in range( self.mapsize[1] ):
            for i in range( self.mapsize[0] ):
                road = self.getRoad( i,j )
                fp.write('road %d %d %d %d\n' % (i,j, road.road, road.pylon ))

        for l in self.locations:
            fp.write('location %f %f %s\n' % ( l.pos[0], l.pos[1],
                                               l.type ) )
        fp.close()

    def loadLevel( self ):
        levelfn = ( './gamedata/levels/%s.txt' %
                    self.lvlName.get() )
        if not os.path.exists( levelfn ):
            tkMessageBox.showwarning( "Cant find level",
                                              "No level named "+levelfn )                                          
            return

        # clear existing floes
        for floe_ndx in range( len(self.icefloes) ):
            tag = 'floe%d' % floe_ndx
            self.canv.delete( tag )
        self.icefloes = []
        self.locations = []
        self.currFloe = None

        self.lvlDesc.set("")
        self.lvlFinal.set(0)
        
        fp = open( levelfn, 'r' )
        while 1:
            line = fp.readline()
            if not line:
                break
            
            if line[0]=='#':
                continue
            l = string.split(line)
            if l[0]=='mapsize':
                self.mapsize = (string.atoi(l[1]),
                                string.atoi(l[2]) )
                self.mapX.set( self.mapsize[0] )
                self.mapY.set( self.mapsize[1] )
            elif l[0]=='level_desc':
                self.lvlDesc.set( string.join( l[1:] ) )
            elif l[0]=='level_final':
                self.lvlFinal.set( string.atoi(l[1]) )
            elif l[0]=='victory':
                self.Victory.set( l[1] )
            elif l[0]=='blocksize':                
                self.c_blocksize = (string.atoi(l[1]),
                                    string.atoi(l[2]) )
                self.blockSz.set( self.c_blocksize[0] )
            elif l[0]=='road_tiles':
                num = string.atoi(l[1])
                for i in range(num):
                    line = fp.readline()
                    l = string.split(line)
                    if l[0]!='road':
                        print "Error: expected road, got ", line
                    else:
                        ii = string.atoi( l[1] )
                        jj = string.atoi( l[2] )
                        road = self.getRoad( ii,jj )
                        road.road = string.atoi(l[3])
                        road.pylon = string.atoi(l[4])
                        print "new road ", ii,jj,road.road, road.pylon
            elif l[0]=='location':
                loc = Location()
                loc.pos = (string.atof(l[1]), string.atof(l[2]))
                loc.type = l[3]
                self.locations.append( loc )
            elif l[0]=='num_floes':
                num = string.atoi(l[1])
                for i in range(num):
                    line = fp.readline()
                    l = string.split(line)
                    if l[0]!='floe':
                        print "Error: expected floe, got ", line
                        n=0
                    else:
                        floe = IceFloe()
                        floe.health = string.atof(l[1])
                        floe.height = string.atof(l[2])
                        floe.breakable = string.atoi(l[3])
                        floe.anchored = string.atoi(l[4])
                        floe.type = l[5]
                        n = string.atoi(l[6])

                    for j in range(n):
                        line = fp.readline()
                        l = string.split(line)
                        p = ( string.atof(l[0]), string.atof(l[1]) )
                        floe.pnts.append( p )

                    print "floe %d added" % len(self.icefloes)
                    self.icefloes.append( floe )

        self.changeMapSize()
        self.redrawAll()
        self.selectionChanged()
                    

    def selectMode( self ):
        self.mode='Select'
        self.status.set( 'Click on floes to select them' )

    def roadMode( self ):
        self.mode='Road'
        self.doDrawRoads.set(1)
        self.drawRoads()
        self.status.set( 'Left button toggle roads, right toggle pylons' )            

    def selectionChanged( self ):
        self.canv.delete( "cursor" )
        if not self.currFloe:
            self.lbl_currFloe['text'] = 'Current Floe: None'
        else:
            self.lbl_currFloe['text'] = 'Current Floe: #%d' % (self.currFloeNdx+1)            
            self.floeStrength.set( self.currFloe.health )
            self.floeType.set( self.currFloe.type )
            self.floeBreakable.set( self.currFloe.breakable )
            self.floeAnchored.set( self.currFloe.anchored )
            self.floeHeight.set( self.currFloe.height )
            self.floeStrength.set( self.currFloe.health )

            p = self.currFloe.getCenter()
            c =  ( int(p[0]*self.c_blocksize[0]),
                   int(p[1]*self.c_blocksize[1]) )
            self.canv.create_arc( c[0]-10, c[1]-10,
                                    c[0]+10, c[1]+10, outline = '#ff6666',
                                      width=2, extent=359,
                                     fill='#883333', tag='cursor' )
        
    def newFloe( self ):
        self.mode='NewFloe'
        self.status.set("Click to add points, right click to undo.")
        self.currFloe = IceFloe()

        self.currFloeNdx = len( self.icefloes )
        self.icefloes.append( self.currFloe )

        self.selectionChanged()

    def clearLocs( self ):
        print "In clearlocs"
        self.locations = []
        self.drawLocs()

    def addLocation( self ):
        self.mode = 'AddLoc'
        locNdx = int(self.lb_locs.curselection()[0])
        locType = self.locTypes[locNdx][0]
        self.status.set("Click to add " + locType )

    def delFloe( self ):
        self.mode='Select'
        if self.currFloe:
            self.icefloes = (self.icefloes[:self.currFloeNdx] +
                             self.icefloes[self.currFloeNdx+1:])
            self.currFloe = None
            self.canv.delete( 'floe%d' % self.currFloeNdx )

            self.selectionChanged()

    def changeMapSize( self):
        print 'mapSize is ', self.mapX.get(), self.mapY.get()
        self.mapsize = ( int(self.mapX.get()),
                         int(self.mapY.get()) )
        bsz = int(self.blockSz.get() )
        self.c_blocksize = ( bsz, bsz )
            
        self.canv['width'] = str(self.c_blocksize[0] * self.mapsize[0] )
        self.canv['height'] = str(self.c_blocksize[1] * self.mapsize[1] )
        self.drawGrid()

    def changeBreak( self ):
        if self.currFloe:
            self.currFloe.breakable = int(self.floeBreakable.get())
            self.currFloe.anchored = int(self.floeAnchored.get())            
            self.redrawFloe( self.currFloeNdx )
        
    def changeStrength( self, x ):
        s = float(self.floeStrength.get())
        if (self.currFloe):
            self.currFloe.health = s
            self.redrawFloe( self.currFloeNdx )

    def changeHeight( self, x ):
        s = float(self.floeHeight.get())
        print s
        if (self.currFloe):
            self.currFloe.height = s            
            self.redrawFloe( self.currFloeNdx )

    def drawGrid( self ):
        print "in drawgrid"
        self.canv.delete( 'grid' )
        xx = float(self.canv['width']) / self.mapsize[0]
        yy = float(self.canv['height']) / self.mapsize[1]

        for i in range( self.mapsize[0]+1 ):
            self.canv.create_line( xx*i, 0, xx*i,
                                   self.c_blocksize[1] * self.mapsize[1],
                                   fill='#33aaaa', tag='grid' )

        for i in range( self.mapsize[1]+1 ):
            self.canv.create_line( 0, yy*i,
                                   self.c_blocksize[0] * self.mapsize[0], yy*i,
                                   fill='#33aaaa', tag='grid' )

    def redrawAll( self ):
        for i in range(len(self.icefloes)):
            self.redrawFloe( i )
        self.drawGrid()

        self.drawRoads()
        self.drawLocs()

    def drawLocs( self ):
        self.canv.delete( 'loc' )
        for l in self.locations:
            p = (int(l.pos[0]*self.c_blocksize[0]),
                 int(l.pos[1]*self.c_blocksize[1] ))

            self.canv.create_rectangle( p[0]-5, p[1]-5,
                                        p[0]+5, p[1]+5,
                                        fill = self.locColor[l.type],
                                        tag = 'loc')
                                        

    def redrawFloe( self, floe_ndx ):
        tag = 'floe%d' % floe_ndx
        self.canv.delete( tag )

        floe = self.icefloes[floe_ndx]
        
        canv_pnts = []
        for p in floe.pnts:
            p2 = ( int(p[0]*self.c_blocksize[0]),
                   int(p[1]*self.c_blocksize[1]) )
            canv_pnts.append( p2 )

        print self.drawMode.get()
        if self.drawMode.get()=='Strength':
            ol = 3
            if floe.type=='Ice':
                fg = int(floe.health * 255)
                fg = '#%02x%02x%02x' % ( 255-fg, fg, fg )
            elif floe.type=='Land':
                fg = '#66ff66'
            elif floe.type=='Blocker':
                fg = '#994499'
            else:
                fg = '#bb8833'
        elif self.drawMode.get()=='Height':
            ol = 1
            if (floe.height <= 1.0):
                fg = int(floe.height * 255)
                fg = '#%02x%02x%02x' % ( fg, fg, fg )
            else:
                fg = int((floe.height-1.0) * 255)
                fg = '#%02x%02x%02x' % ( 255-fg, 255-fg, 255 )
        else:
            fg = '#ff00ff'
            ol = 2

        pat=None
        if floe.breakable:
            pat='gray75'

        if floe.anchored:
            olcolor='#ffffff'
        else:
            olcolor='#ff00ff'
                
            
        if (len(canv_pnts) >= 3):
            self.canv.create_polygon( canv_pnts, tag=tag,
                                      fill=fg, outline=olcolor,
                                      stipple=pat,
                                      width=ol )
        else:
            for p in canv_pnts:
                self.canv.create_rectangle( p[0]-2, p[1]-2,
                                            p[0]+2, p[1]+2,
                                            fill=fg, outline='#ffffff',                                            
                                            tag=tag )

        #self.drawGrid()
        self.canv.lift( 'grid' )
            

    def canvasClick( self, event ):
        x = self.canv.canvasx( event.x )
        y = self.canv.canvasy( event.y )

        xx = float(x) / self.c_blocksize[0]
        yy = float(y) / self.c_blocksize[1]
        print "click ", self.mode, xx, yy
            
        if self.mode=='NewFloe':
            self.currFloe.pnts.append( (xx, yy) )
            self.redrawFloe( self.currFloeNdx )
            self.selectionChanged()
        elif self.mode=='AddLoc':
            locNdx = int(self.lb_locs.curselection()[0])
            locType = self.locTypes[locNdx]
            loc = Location()
            loc.type = locType[0]
            loc.pos = ( xx, yy )
            self.locations.append( loc )
            self.drawLocs()
        elif self.mode=='Select':
            bestNdx = -1
            bestD = -1.0
            for floeNdx in range(len(self.icefloes)):
                floe= self.icefloes[floeNdx]
                c = floe.getCenter()

                d = (c[0]-xx)**2 + (c[1]-yy)**2
                if (bestD < 0.0) or (d < bestD):
                    bestNdx = floeNdx
                    bestD = d

            if bestNdx >0:
                self.currFloeNdx = bestNdx
                self.currFloe = self.icefloes[bestNdx]
                self.selectionChanged()
                
                
        elif self.mode=='Road':
            i = int(xx)
            j = int(yy)
            road = self.getRoad( i, j )        
            print "road  ", i, j, road.road, road.pylon
            road.road = 1-road.road

            self.drawRoads()

    def canvasClick2( self, event ):
        x = self.canv.canvasx( event.x )
        y = self.canv.canvasy( event.y )

        xx = float(x) / self.c_blocksize[0]
        yy = float(y) / self.c_blocksize[1]
        
        if self.mode=='NewFloe':
            if len(self.currFloe.pnts):
                self.currFloe.pnts = self.currFloe.pnts[:-1]
            self.redrawFloe( self.currFloeNdx )
            self.selectionChanged()
        elif self.mode=='Select':
            self.currFloe = None
            self.selectionChanged()
        elif self.mode=='Road':
            i = int(xx)
            j = int(yy)
            road = self.getRoad( i, j )        
            print "road  ", i, j, road.road, road.pylon
            road.pylon = 1-road.pylon

            self.drawRoads()
            

    def build( self ):
        self.root = Tkinter.Tk()
        self.root.title('Trans-Icelandic Express Level Editor')

        # control frame
        fmain = Tkinter.Frame( self.root )
        fmain.pack( side='left', expand='true', fill ='y' )

        btn = Tkinter.Button(  fmain, text='Select',
                               bg='#ffaaaa', fg='#ffffff',
                               command=self.selectMode )
        btn.pack( side='top', fill='x')

        btn = Tkinter.Button(  fmain, text='New Ice Floe',
                               bg='#44aaff', fg='#ffffff',
                               command=self.newFloe )
        btn.pack( side='top', fill='x')

        btn = Tkinter.Button(  fmain, text='Delete Floe',
                               bg='#44aaff', fg='#ffffff',
                               command=self.delFloe )
        btn.pack( side='top', fill='x')        

        btn = Tkinter.Button(  fmain, text='Edit Roads',
                               bg='#444444', fg='#ffffff',
                               command=self.roadMode )
        btn.pack( side='top', fill='x')        

        btn = Tkinter.Button(  fmain, text='Save',
                               bg='#996633', fg='#ffffff',
                               command=self.saveLevel )
        btn.pack( side='top', fill='x')
        
        btn = Tkinter.Button(  fmain, text='Load',
                               bg='#996633', fg='#ffffff',
                               command=self.loadLevel )
        btn.pack( side='top', fill='x')

        f = Tkinter.Frame( fmain, borderwidth='3', relief='raised' )
        lbl = Tkinter.Label( f, text='Map Settings',
                             bg='#3333ff', fg='#ffff33' )
        lbl.pack( side='top', expand='true', fill='x' )
        
        ## blocksize        
        f2 = Tkinter.Frame( f )        
        lbl = Tkinter.Label( f2, text='Block Size' )
        lbl.pack( side='left' )
        self.blockSz = Tkinter.StringVar()
        self.blockSz.set(str(self.c_blocksize[0]))    
        ent = Tkinter.Entry( f2, width=8, textvariable=self.blockSz )
        ent.pack( side='left' )
        f2.pack( side='top', anchor='w' )
        
        #mapsize
        lbl = Tkinter.Label( f, text='Map Size' )
        lbl.pack( side='top' )
    
        f2 = Tkinter.Frame( f )
        self.mapX = Tkinter.StringVar()
        self.mapX.set( self.mapsize[0] )
        ent = Tkinter.Entry( f2, width=8, textvariable=self.mapX )
        ent.pack( side='left' )
        self.mapY = Tkinter.StringVar()
        self.mapY.set( self.mapsize[1] )
        ent = Tkinter.Entry( f2, width=8, textvariable=self.mapY )
        ent.pack( side='left' )        
        f2.pack( side='top', anchor='w' )
        
        btn = Tkinter.Button(  f, text='Apply to Map',
                               bg='#aaffaa',
                               command=self.changeMapSize )
        btn.pack( side='top' )

        f.pack( side='top', fill='x' )        

        ##### the floe properties
        f = Tkinter.Frame( fmain, borderwidth='3', relief='raised' )
        lbl = Tkinter.Label( f, text='Floe Properties',
                             bg='#3333ff', fg='#ffff33' )
        lbl.pack( side='top', expand='true', fill='x' )
        f2 = Tkinter.Frame( f )
        self.lbl_currFloe = Tkinter.Label( f, bg='#ffffff',
                                           text='Current Floe: None' )
        self.lbl_currFloe.pack( side='top' )

        # flow type
        self.floeType = Tkinter.StringVar()
        self.floeType.set('Ice')
        for i in ['Ice', 'Land', 'Blocker', 'Custom']:
            rb = Tkinter.Radiobutton( f, text=i, value=i,
                                      var=self.floeType,
                                      command=self.changeFloeType )
            rb.pack( side='top', anchor='w', )

        # floe strength
        f2 = Tkinter.Frame( f, relief='groove', borderwidth=2 )
        lbl = Tkinter.Label( f2, text='Strength' )
        lbl.pack( side='top', anchor='w' )
        self.floeStrength = Tkinter.StringVar()
        self.floeStrength.set( 1.0 )
        ent = Tkinter.Scale( f2, showvalue='true', orient='horiz',
                             from_='0.0', to='1.0', resolution=0.01,
                             var=self.floeStrength,
                             command=self.changeStrength )
        ent.pack( side='top' )
        f2.pack( side='top', anchor='w' )

        # floe height
        f2 = Tkinter.Frame( f, relief='groove', borderwidth=2 )
        lbl = Tkinter.Label( f2, text='Height' )
        lbl.pack( side='top', anchor='w' )
        self.floeHeight = Tkinter.StringVar()
        self.floeHeight.set( 0.05 )
        ent = Tkinter.Scale( f2, showvalue='true', orient='horiz',
                             from_='0.0', to='2.0', resolution=0.01,
                             var=self.floeHeight,
                             command=self.changeHeight )
        ent.pack( side='top' )
        f2.pack( side='top', anchor='w' )
        
        # flow stability
        self.floeBreakable = Tkinter.IntVar()
        self.floeBreakable.set( 0 )
        btn = Tkinter.Checkbutton( f, text='Breakable',
                                   command=self.changeBreak,
                                   variable=self.floeBreakable )
        btn.pack( side='top' )

        self.floeAnchored = Tkinter.IntVar()
        self.floeAnchored.set( 1 )
        btn = Tkinter.Checkbutton( f, text='Anchored',
                                   command=self.changeBreak,
                                   variable=self.floeAnchored )
        btn.pack( side='top' )
        
        f.pack( side='top', fill='x' )


        

        

        ##### canvas
        f = Tkinter.Frame( self.root )
        f.pack( side='left', expand='true', fill='both')

        self.canv = Tkinter.Canvas( f,
                                    width=str(self.c_blocksize[0]*self.mapsize[0]),
                                    height=str(self.c_blocksize[1]*self.mapsize[1]),
                                    bg='#334455' )
        self.canv.pack( expand='true', fill='both')

        self.canv.bind( '<Button-1>', self.canvasClick )
        self.canv.bind( '<Button-3>', self.canvasClick2 )

        self.status = Tkinter.StringVar()
        self.status.set( "Welcome to the Editor..." )
        self.statusbar = Tkinter.Label( f, bg='#ffffff', justify='left',
                                        textvariable=self.status )
        self.statusbar.pack( side='bottom', expand='true', fill='x' )

        # right hand frame
        fmain = Tkinter.Frame( self.root )
        fmain.pack( side='left', expand='true', fill ='y' )

        ##### display properties
        f = Tkinter.Frame( fmain, borderwidth='3', relief='raised' )
        lbl = Tkinter.Label( f, text='Display',
                             bg='#3333ff', fg='#ffff33' )
        lbl.pack( side='top', expand='true', fill='x' )

        # Level name
        f2 = Tkinter.Frame( f, relief='groove', borderwidth=2 )
        lbl = Tkinter.Label( f2, text='Name:' )
        lbl.pack( side='top', anchor='w' )
        self.lvlName = Tkinter.StringVar()
        self.lvlName.set( "Unnamed" )
        ent = Tkinter.Entry( f2, textvariable=self.lvlName )
        ent.pack( side='top' )
        
        lbl = Tkinter.Label( f2, text='Description:' )
        lbl.pack( side='top', anchor='w' )
        self.lvlDesc = Tkinter.StringVar()
        self.lvlDesc.set( "" )
        ent = Tkinter.Entry( f2, textvariable=self.lvlDesc )
        ent.pack( side='top' )

        self.lvlFinal = Tkinter.IntVar()
        self.lvlFinal.set( 1 )
        btn = Tkinter.Checkbutton( f2, text='Playable',
                                   variable=self.lvlFinal )
        btn.pack( side='top' )
        
        f2.pack( side='top', anchor='w' )



        # Display mode
        self.drawMode = Tkinter.StringVar()
        self.drawMode.set('Strength')
        for i in ['Strength', 'Height']:
            rb = Tkinter.Radiobutton( f, text=i, value=i,
                                      var=self.drawMode,
                                      command=self.changeDrawMode )
            rb.pack( side='top', anchor='w', )

        # draw roads
        self.doDrawRoads = Tkinter.IntVar()
        self.doDrawRoads.set( 0 )
        btn = Tkinter.Checkbutton( f, text='Draw Roads',
                                   command=self.drawRoads,
                                   variable=self.doDrawRoads )
        btn.pack( side='top' )

        f.pack( side='top', fill='x' )

        ##### Locations/Items
        f = Tkinter.Frame( fmain, borderwidth='3', relief='raised' )
        lbl = Tkinter.Label( f, text='Locations',
                             bg='#3333ff', fg='#ffff33' )
        lbl.pack( side='top', expand='true', fill='x' )
        
        frame = Tkinter.Frame(f)
        frame.pack( side='top' )
        scrollbar = Tkinter.Scrollbar(frame, orient=Tkinter.VERTICAL)
        self.lb_locs = Tkinter.Listbox(frame, yscrollcommand=scrollbar.set)
        scrollbar.config(command=self.lb_locs.yview)
        scrollbar.pack(side=Tkinter.RIGHT, fill=Tkinter.Y)
        self.lb_locs.pack(side=Tkinter.LEFT, fill=Tkinter.BOTH, expand=1)
        
        for type, color in self.locTypes:    
            self.lb_locs.insert( Tkinter.END, type)

        f2 = Tkinter.Frame( f )
        btn = Tkinter.Button( f2, text='Add',
                                bg='#aaffaa',
                                command=self.addLocation )
        btn.pack( side='left' )
        btn = Tkinter.Button( f2, text='Clear',
                                bg='#ffaaaa',
                                command=self.clearLocs )
        btn.pack( side='left' )
        f2.pack( side='top' )

        f.pack( side='top', fill='x' )

        f = Tkinter.Frame( fmain, borderwidth='3', relief='raised' )
        lbl = Tkinter.Label( f, text='Victory',
                             bg='#3333ff', fg='#ffff33' )
        lbl.pack( side='top', expand='true', fill='x' )

        self.Victory = Tkinter.StringVar()
        self.Victory.set('Connect')
        for i in ['Connect', 'KillSheep', 'GetCrates']:
            rb = Tkinter.Radiobutton( f, text=i, value=i,
                                      var=self.Victory )
            rb.pack( side='top', anchor='w', )


        f.pack( side='top', fill='x' )

        # draw the graphical bits
        self.drawGrid()
    
    def run( self ):
        self.root.mainloop()
        
if __name__=='__main__':
    tie = TIEedit()

    tie.build()
    tie.run()
