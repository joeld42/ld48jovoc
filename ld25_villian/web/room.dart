
part of ld25_villian;

class Room {
  int w; 
  int h;
  int x;
  int y;
  ImageElement roomImg;
  bool ready;
  
  Room( int w, int h, String imgName )
  {
      this.w = w;
      this.h = h;
      this.ready = false;
      
      x=0;
      y=0;
      
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
  }
  
  
  
  
}
