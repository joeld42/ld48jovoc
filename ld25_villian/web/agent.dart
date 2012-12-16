part of ld25_villian;

class Agent {
  num x;
  num y;
  
  num walkDir;
  bool dead;
  
  Agent( num this.x, num this.y ) {
    walkDir = 1.0;
    if (rand.nextDouble() < 0.5) {
      walkDir = -1.0;
    }
    dead = false;
  }
  
  void update(num dt, List<MapTile> map) {
    
    // find what tile we're standing on
    int mapx = (x/16).floor().toInt();
    int mapy = ((y+1)/16).floor().toInt();
    
    MapTile tile = map[mapIndex(mapx, mapy)];
    if (tile.terrainType == TERRAIN_DIRT)
    {
      num oldX = x;
      x = x + walkDir;
      
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
      y += 60.0*dt;
    }
    
  }
  
  void draw(CanvasRenderingContext2D context, num yval) {
    context.fillStyle = "#00f";
    context.fillRect( x - 6,(y-15)-yval,12,15 );
  }
}
