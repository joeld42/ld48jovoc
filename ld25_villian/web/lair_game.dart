
part of ld25_villian;

// global RNG
Random rand;

var roomNames = [ "Tickle Room",
                  "Unregulated Machine Shop",
                  "Petting Zoo",
                  "Discount Sushi Buffet",
                  "Der Toestubbah",
                  "Lava Sauna",
                  "Shark Pool",
                  "Crotch Laser",
                  "Mongo's Arena",
                  "Alligator Pit",
                  "Laser Disco",
                  "Electric Feel",
                  "Rocket Launch Area",
                  "Spikey Room"
                  ];

var agentNames = [ "Agent 007", "Agent Sandbag", "Agent Fred",
                   "Dronebot", "Lt. Hockensocks", "Mister X",
                   "Agent Chip", "Legolas", "Agent Stuffy",
                   "Mayor McCheese", "Dr. Pepperspray", 
                   "0xDEADBEEF", "Nascar Charlie", "Agent Foo" ];

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
//      context.fillStyle = "#fff";
//      context.fillRect((mousePosMap.x*16)+1, 
//                       ((mousePosMap.y*16)-minimap.yval)+1, 15, 15);
      
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
    var agent = new Agent( agentImg, 350, 20.0 );
    
    
    agent.name = agentNames[ rand.nextInt(agentNames.length)];
    agent.hp = 1+rand.nextInt(15);
    if (agent.hp > 10) {
      agent.hp = 10;
    }
    
    // Reveal his HP
    Element agentbar = query("#toolbar");
    var agentThumb = agentbar.children.last; 
    agentThumb.text = "${agent.hp}"; 
    
    agent.thumb = agentThumb;
    
    // add the agent
    agents.add( agent );     
    
    addAlert( '${agent.name} (+${agent.hp}) has entered our airspace!');
    
  }
  
  void update( num dt)
  {      
      // Update all agents
      final int numSteps = 5;
      for (int i=0; i < numSteps; i++)
      {
        for (Agent agent in agents) {
          agent.update(dt/numSteps, worldMap );
          
          // Is this agent in a room?
          for (Room r in rooms)
          {
            if ( (r.hp > 0) && (agent.fighting <= 0.0) && 
                 (agent.mapx >= r.x) && (agent.mapx < r.x + r.w) &&
                 (agent.mapy-1 >= r.y) && (agent.mapy-1 < r.y + r.h) ) {
              agent.fighting = 1.0; // 1 sec.
              agent.doneFight = () {
                  num roomhp = r.hp;
                  
                  r.hp -= agent.hp;
                  agent.hp -= roomhp;
                  
                  if (agent.hp < 0) {
                    agent.hp = 0;
                  }
                  
                  if (r.hp <= 0) {
                    r.hp = 0;
                    
                    addAlert( "The ${r.roomName} has been deactivated." );
                  }
                  
                  // update thumbnail
                  agent.thumb.text = "${agent.hp}";
                  
                  // Is this the last room?
                  if (r.isLair) {
                      print("It's the last room...");
                      
                      if (agent.hp>0) {
                        print("Agent still alive after last room! lose");
                        loseGame( "Shucks! ${agent.name} defeated you and brought down your lair.");
                        addAlert( "${agent.name} has defeated you!");
                      } else {
                        winGame( "Yeah! You taunted ${agent.name} with your sinister plan before getting rid of him.");
                        addAlert( "Before you die a horrible death, ${agent.name}, let me tell you about my plan to...");
                      }
                  }
                  else if (agent.hp <=0) {
                    print("agent is dead");
                    addAlert( "The ${r.roomName} has dealt with ${agent.name}!");
                    
                    showActionButtons( true );
                    
                    num ndx = agents.indexOf( agent );
                    agents.removeAt( ndx );
                    
                    Element agentbar = query("#toolbar");
                    if (agentbar.children.length==1) {
                         print("That was the last agent..");                   
                        loseGame( "Oh no! You killed all the agents before you could reveal your evil plan!");
                    } else {
                      agentbar.children.removeLast();
                    }
                  }
              };
            }
          }
          
        }
      }
  }
  
  void winGame( String message ) {
    endGame( "You Win!!", message );
  }
  
  void loseGame( String message ) {
    endGame( "You Lose!", message );
  }
  
  void endGame( String title, String message ) {
    var overlay = query("#overlay");
    overlay.hidden = false;
    
    showActionButtons( false );
    
    var titleElem = query("#ov_title");
    titleElem.text = title;
    
    var messageElem = query("#ov_message");
    messageElem.text = message;
    
    // make sure there's no pesky agents running around
    agents.clear();
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
        
        if (agents.length>0) {
          return;
        }
        
        print("Expand base...");
        if (expandBase()) {          
          // expanding base trigger a new agent
          spawnAgent();          
          showActionButtons( false );
        }

      });
   
      query("#btn_go").on.click.add((_){
        
        if (agents.length>0) {
          return;
        }

        
        // spawn a new agent without expanding base
         spawnAgent();
         
         showActionButtons( false );
      });
     
      query("#btnplay").on.click.add((_){
        resetGame();
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
bool expandBase()
{
  print("Expand base...");
  if (rooms.length < roomLocs.length) {
    
    var room = new Room( 4, 3, "gamedata/room4x3.png");
    
    room.roomName = roomNames[ rand.nextInt( roomNames.length )];
    
    rooms.add( room );    
    layoutRooms();
    
    return true;
  } else {
    return false;
  }
}


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
      
      print("${roomLocs.length} roomLocs");
      for (int i=0; i < roomLocs.length; i++) {
        var loc = roomLocs[i];
        print("loc ${loc.x} ${loc.y}");
      }
    
      // load rooms
      resetGame();
      
      print("Map set up.");
      
      }); // onLoad
    
   }
  
  void resetGame()
  {
    // reset overlay
    var overlay = query("#overlay");
    overlay.hidden = true;
    
   // reset agents
    agents.clear();
    
    // reset alerts
    resetAlerts();
    addAlert("Welcome to Evil Lair created by Joel Davis for LudumDare 25.");
        
    // reset the rooms
    rooms = new List<Room>();
    var room = new Room( 3, 2, "gamedata/room3x2.png");
    room.roomName = "Lair";
    
    // lair rooms have 3-6 hp
    room.hp = rand.nextInt(3) + 3;
    
    room.isLair = true;
    rooms.add( room );
    
    // add a starting room
    expandBase();
    
    showActionButtons( true );
    
    // Build the agents
    int numAgents = rand.nextInt(3) + 3;
    
    Element agentbar = query("#toolbar");
    agentbar.children.clear();
    
    for (int i=0; i < numAgents; i++) {
      var item = new DivElement();
      item.text = '(?)';
      item.classes.add( 'button' );
      
      agentbar.children.add( item );
    }
    
  }
  
  void layoutRooms()
  {
    
    for (int i=0; i<rooms.length; i++) {
      int locNdx = (rooms.length-1) - i;
      
      Room r = rooms[i];
      r.x = roomLocs[locNdx].x - (r.w/2).round().toInt();
      r.y = roomLocs[locNdx].y - (r.h-1);
      
//      r.x = roomLocs[locNdx].x;
//      r.y = roomLocs[locNdx].y;
      
    }
  }
    
  void showActionButtons( bool doShow )
  {
    print("show action buttons ${doShow}");
    
    if (!doShow) {
      query("#btn_go").classes.add('greyed');
      query("#btn_expand").classes.add('greyed');      
    } else {
      query("#btn_go").classes.remove('greyed');      
      query("#btn_expand").classes.remove('greyed');      
    }
    

  }
  
  void addAlert( String message ) {
    var alerts = query("#alerts");
    
    var item = new ParagraphElement();
    item.text = message;
//    item.classes.add( 'button' );
    
    alerts.children.add( item );
    alerts.scrollTop = alerts.scrollHeight;
  }
  
  void resetAlerts()
  {
    var alerts = query("#alerts");
    alerts.children.clear();
  }
   
   Point screenToMap( int x, int y) {
      return new Point( x/16, (y+yval)/16 );
   }
  
}
