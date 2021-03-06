
part of ld25_villian;

class Room {
  int w; 
  int h;
  int x;
  int y;
  ImageElement roomImg;
  String roomName;
  int hp;
  bool ready;
  bool isLair;
  
  Room( int w, int h, String imgName )
  {
      this.w = w;
      this.h = h;
      this.ready = false;
      this.roomName = "Room Name";
      hp = rand.nextInt(15)+1;
      if (hp > 10) hp = 10;
      x=0;
      y=0;
      isLair = false;
      
      roomImg = new ImageElement();
      roomImg.src = imgName;
      roomImg.on.load.add((_) {
        this.ready = true;
      });
           
  }
  
  void draw( CanvasRenderingContext2D context, num yval)
  {
    if (!ready) {
      // img not ready, draw as a grey box
      context.fillStyle = "rgba(20, 20, 20, 0.5)";
      context.fillRect( (x*16),(y*16)-yval,w*16,h*16);
    } else {
//      context.drawImage(backgroundImg, 0, minimap.yval, 640, 400, 0, 0, 640, 400 );
      context.drawImage( roomImg, 0,0, w*16, h*16, x*16, y*16-yval, w*16, h*16 ); 
    }
    
    context.fillStyle="#fff";
    context.fillText( roomName, x*16 + 4, (y*16-yval) + 24 );

    context.fillStyle="#000";
    context.fillText( hp.toString(), x*16+2, (y*16-yval) + 12 );

  }
  
  
  
  
}
