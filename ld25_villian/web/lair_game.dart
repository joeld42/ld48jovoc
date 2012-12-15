
part of ld25_villian;

class LairGame {
  
  // FPS Counter
  num renderTime;
  double fpsAverage;
  
  CanvasElement gameMapCanvas;
  ImageElement backgroundImg;
  MiniMap minimap;
  
  List<MapTile> worldMap;
  
  void drawFrame()
  {
      CanvasRenderingContext2D context = gameMapCanvas.context2d;
      context.globalAlpha = 1.0;
      
      // Draw the background
      context.drawImage(backgroundImg, 0, minimap.yval, 640, 400, 0, 0, 640, 400 );
      
      // draw the map tiles
      if ((worldMap!=null) && (worldMap.length>0))
      {
          context.fillStyle = "rgba(128, 0, 0, 0.1)";
          for (int j=0; j < 125; j++) {
            for (int i=0; i < 40; i++) {
              if (worldMap[mapIndex(i,j)].terrainType==TERRAIN_LAVA)
              {
                context.rect((i*16)+1, (j*16)+1, 15, 15);
                context.fill();
              }
                        
            }
          }
      }
        
  }
   
  
  void showFps(num fps) {
    if (fpsAverage == null) {
      fpsAverage = fps;
    }
  
    fpsAverage = fps * 0.05 + fpsAverage * 0.95;
  
    query("#fps_counter").text = "${fpsAverage.round().toInt()} fps";
  }

  void gameloop(num _) {
    num time = new Date.now().millisecondsSinceEpoch;
  
    if (renderTime != null) {
      showFps((1000 / (time - renderTime)).round());
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

    // Load map
    buildMap();
    
    // Load resources
    backgroundImg = new ImageElement();
    backgroundImg.src = "gamedata/volcano_bg.jpg";
    Completer c = new Completer();
    backgroundImg.on.load.add((_) {
      
            
      print('Loaded ${backgroundImg.src}');
      c.complete(backgroundImg.src);
      
      // Start the game loop
      window.requestAnimationFrame(gameloop);
    });
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
      print("Map set up.");
      
      }); // onLoad
    
   }
    
   int mapIndex( int x, int y ) {
     return (y*40)+x;
   }
  
}
