part of ld25_villian;

class Agent {
  ImageElement frames;
  num x;
  num y;
  
  int mapx;
  int mapy;
  
  num walkDir;
  num fighting; // timer
  var doneFight; // callback
  
  Element thumb;
  
  String name;
  int hp;
  
  Agent( ImageElement this.frames, num this.x, num this.y ) {
    walkDir = 1.0;
    if (rand.nextDouble() < 0.5) {
      walkDir = -1.0;
    }
    fighting = 0.0;
  }
  
  void update(num dt, List<MapTile> map) {
    
    // find what tile we're standing on
    mapx = (x/16).floor().toInt();
    mapy = ((y+1)/16).floor().toInt();
  
    // are we fighting?
    if (fighting > 0.0) {
      fighting -= dt;
      if (fighting < 0.0) {
        doneFight();
      }
      return;
    }
      
    MapTile tile = map[mapIndex(mapx, mapy)];
    if (tile.terrainType == TERRAIN_DIRT)
    {
      num oldX = x;
      x = x + (walkDir*100*dt);
      
//      print("walking x is ${x} walkdir ${walkDir}");
      
      int mapx2 = (x/16).floor().toInt();
      int mapy2 = ((y-8)/16).floor().toInt();
      MapTile nuTile = map[mapIndex(mapx2, mapy2)];
      if (nuTile.terrainType != TERRAIN_SKY) {
        // ran into a wall, turn around
//        print("turn around, walkdir is ${walkDir}");
        
        x = oldX;
        walkDir = -walkDir;
      }
    } else if (tile.terrainType==TERRAIN_SKY)
    {
      // falling
      y += 200.0*dt;
    }
    
  }
  
  void draw(CanvasRenderingContext2D context, num yval) {
    num jitterx=0.0, jittery=0.0;
    
    if (fighting > 0.0)
    {
      context.fillStyle = "#f00";
      context.fillRect( x - 6,(y-15)-yval,12,15 );
      
      jitterx = ((rand.nextDouble() * 2.0) - 1.0) * 5.0;
      jittery = ((rand.nextDouble() * 2.0) - 1.0) * 5.0;
    } 
     
    context.drawImage( frames, 0, 0, 16, 16, (x-8)+jitterx, ((y-15)-yval)+jittery, 16, 16);
    
    context.fillStyle = "#fff";
    context.fillText( "${name} (${hp})", x-15, (y-15)-yval );
    
  }
}
