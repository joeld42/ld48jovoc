
part of ld25_villian;

// global RNG
Random rand;

class Point {
  num x, y;
  Point(this.x, this.y);
}

num clamp( num orig, num low, num high) {
  if (orig < low) {
    return low;
  } else if (orig > high) {
    return high;
  } else {
    return orig;
  } 
}

int mapIndex( int x, int y ) {
  return (y*40)+x;
}

class LairGame {
  
  // FPS Counter
  num renderTime;
  double fpsAverage;
  
  CanvasElement gameMapCanvas;
  ImageElement backgroundImg;
  ImageElement agentImg;
  ImageElement conduitImg;
  
  MiniMap minimap;
  
  List<MapTile> worldMap;
  List<Point> roomLocs;
  num nextAgentSpawn;
  
  // Mouse pos
  Point mousePos;
  Point mousePosWorld; // mouse pos in world coords (matches bgimage)
  Point mousePosMap; // mouse pos in map tile index
  
  List<Room> rooms;
  List<Agent> agents;
  
  List <Future> _preloads;
  
  void drawFrame()
  {
      CanvasRenderingContext2D context = gameMapCanvas.context2d;
      
      //context.clearRect( 0, 0, 640, 400 );
      
//      context.fillStyle = "#f0f";
//      context.rect(0,0,640,400);
//      context.fillRect();
//      
      // Draw the background
      context.drawImage(backgroundImg, 0, minimap.yval, 630, 400, 0, 0, 630, 400 );
      
      // draw map cursor
      //context.fillStyle = "rgba(255, 255, 0, 0.5)";
      context.fillStyle = "#fff";
      context.fillRect((mousePosMap.x*16)+1, 
                       ((mousePosMap.y*16)-minimap.yval)+1, 15, 15);
      
      // draw the map tiles
      /*
      int lastTerrainType = -1;
      if ((worldMap!=null) && (worldMap.length>0))
      {
          for (int j=0; j < 125; j++) {
            
            num screenYpos = (j*16) - minimap.yval;
            if ( (screenYpos < -15) ||
                 (screenYpos > 400) ) {
              continue;
            }
            
            for (int i=0; i < 40; i++) {
              
              int terr = worldMap[mapIndex(i,j)].terrainType;
              if (terr!=lastTerrainType)
              {
                if (terr==TERRAIN_DIRT) {
                  context.fillStyle = "rgba(128, 80, 0, 0.5)";
                } else if (terr==TERRAIN_LAVA) {
                  context.fillStyle = "rgba(255, 0, 0, 0.5)";  
                } else if (terr==TERRAIN_SOLID) {
                  context.fillStyle = "rgba(255, 0, 255, 0.5)";  
                }
                lastTerrainType = terr;
              }
              
              if (terr!=TERRAIN_SKY)
              {
                context.fillRect((i*16)+1, screenYpos+1, 15, 15);
              }
                        
            }
          }
      }
   */
      
      // Draw the rooms
      for (Room room in rooms) {
          room.draw( context, minimap.yval );
      }
      
      // Draw agents
      for (Agent agent in agents) {
        agent.draw( context, minimap.yval );
      }
  }
   
  
  void showFps(num fps) {
    if (fpsAverage == null) {
      fpsAverage = fps;
    }
  
    fpsAverage = fps * 0.05 + fpsAverage * 0.95;
  
    query("#fps_counter").text = "${fpsAverage.round().toInt()} fps";
  }
  
  void spawnAgent() {
    print( "spawn agent...");
    nextAgentSpawn = 2 + (rand.nextDouble() * 3.0);
    
    var agent = new Agent( agentImg, rand.nextDouble()*600, 20.0 );
    agents.add( agent );      
  }
  
  void update( num dt)
  {      
      // Update all agents
      final int numSteps = 5;
      for (int i=0; i < numSteps; i++)
      {
        for (Agent agent in agents) {
          agent.update(dt/numSteps, worldMap );
        }
      }
  }

  void gameloop(num _) {
    num time = new Date.now().millisecondsSinceEpoch;
    if (renderTime != null) {
      num dt = time - renderTime;      
      showFps((1000 / dt).round());
      
      update(dt/1000.0);
    }  
    
    renderTime = time;
      
    drawFrame();
    
    window.requestAnimationFrame(gameloop);
  }

  
void startGame()
{
    // Get our game elements from the document
    gameMapCanvas = query("#gamemap");
    
    // Make minimap
    minimap = new MiniMap();
    
    mousePos = new Point(0,0);
    mousePosWorld = new Point(0,0);
    mousePosMap = new Point(0,0);
    
    // update mouse pos
    gameMapCanvas.on.mouseMove.add( (MouseEvent ev) {
      
      ev.preventDefault();
      
      num clickYpos = ev.clientY - gameMapCanvas.offsetTop;
      num clickXpos = ev.clientX - gameMapCanvas.offsetLeft;
      
      mousePos.x = clickXpos; 
      mousePos.y = clickYpos;
      
      mousePosWorld.x = mousePos.x;
      mousePosWorld.y = mousePos.y + minimap.yval;
      
      mousePosMap.x = (mousePosWorld.x / 16).floor();
      mousePosMap.y = (mousePosWorld.y / 16).floor();
      
      mousePosMap.x = clamp( mousePosMap.x, 0, 40 );
      mousePosMap.y = clamp( mousePosMap.y, 0, 125 );
        
      //print("yval ${minimap.yval} mousePosWorld ${mousePosWorld.x} ${mousePosWorld.y} Map ${mousePosMap.x}, ${mousePosMap.y}");
      
    } );
    
    // init random 
    rand = new Random();
    
    // Init agents
     agents = new List<Agent>();
    
    // Load map
    buildMap();
      
    // Load resources
    _preloads = new List<Future>();
    
    backgroundImg = new ImageElement( src:"gamedata/volcano_bg.jpg" );
    preloadImg( backgroundImg );
    
    agentImg = new ImageElement( src:"gamedata/agent.png" );
    preloadImg( agentImg );       
    
    conduitImg = new ImageElement( src:"gamedata/conduit.png");
    preloadImg( conduitImg );
    
    // Wait for all resources
    Futures.wait(_preloads).then((List values) {
      print ("all imgs loaded...");
      
       // Hook up buttons
      query("#btn_expand").on.click.add((_){
        print("TODO: Expand base...");
      });
   
      query("#btn_go").on.click.add((_){
         spawnAgent();
      });
        
        // Start the game loop
        window.requestAnimationFrame(gameloop);      
    });
    
 }

Future preloadImg( ImageElement img ) {
  Completer c = new Completer();
  Future fut = c.future;
  img.on.load.add((_) {                 
    print('Loaded ${img.src}');
    c.complete(img.src);
  });  
  
  _preloads.add(fut);
  
  return fut;
}

 // ===========================================
 // Map stuff
 // ===========================================
  void buildMap()
  {
    var mapAttrImg = new ImageElement();
    mapAttrImg.src = "gamedata/map_attrs.png";
    Completer c = new Completer();
    mapAttrImg.on.load.add((_) {
      print('Loaded ${mapAttrImg.src}');
      c.complete(mapAttrImg.src);
      
      // Get the image data from the attrs image
      CanvasElement attrCanv = new CanvasElement(width:40, height:125);      
      CanvasRenderingContext2D context = attrCanv.context2d;
      
      context.drawImage( mapAttrImg, 0, 0, 40, 125, 0, 0, 40, 125 );
      
      ImageData imgData= context.getImageData( 0, 0, 40, 125 );
      print('attr image data ${imgData.width}, ${imgData.height}');
      
      roomLocs = new List<Point>();
      
      //var rand = new Random();
      worldMap = new List<MapTile>( 40*125 );
      for (int y=0; y < 125; y++) {      
        for (int x=0; x < 40; x++) {        
          var tile = new MapTile();
          
          int ndx = ((y*40)+x)*4;
          int r = imgData.data[ndx+0];
          int g = imgData.data[ndx+1];
          int b = imgData.data[ndx+2];
          
         if ((r==0)&&(g==255)&&(b==255)) {
            // background/sky
           tile.terrainType = TERRAIN_SKY;
         } else if ((r==0)&&(g==0)&&(b==255)) {
           // special, also add to the list of room locs
           tile.terrainType = TERRAIN_SKY;
           roomLocs.add( new Point( x, y) );
         } else if ((r==255)&&(g==0)&&(b==0)) {
            tile.terrainType = TERRAIN_LAVA;
         } else if ((r==0)&&(g==255)&&(b==0)) {
            tile.terrainType = TERRAIN_SOLID;
         } else {
           //print ('${r}, ${g}, ${b}');
           
           tile.terrainType = TERRAIN_DIRT;
         }
        //  tile.terrainType = rand.nextInt(6);
//          tile.terrainType = TERRAIN_LAVA;
          
          worldMap[ mapIndex( x, y) ] = tile;
        }        
      }
    
      // load rooms
      resetRooms();
      
      print("Map set up.");
      
      }); // onLoad
    
   }
  
  void resetRooms()
  {
    rooms = new List<Room>();
    var room = new Room( 3, 2, "gamedata/tinylair.png");
    room.x = roomLocs[0].x.toInt();
    room.y = roomLocs[0].y.toInt();
    rooms.add( room );    
  }
    
   
   Point screenToMap( int x, int y) {
      return new Point( x/16, (y+yval)/16 );
   }
  
}
