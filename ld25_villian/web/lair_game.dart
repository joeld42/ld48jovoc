
part of ld25_villian;

class LairGame {
  
  // FPS Counter
  num renderTime;
  double fpsAverage;
  
  CanvasElement gameMapCanvas;
  ImageElement backgroundImg;
  MiniMap minimap;
  
  void drawFrame()
  {
      CanvasRenderingContext2D context = gameMapCanvas.context2d;
      
      // Draw the background
      context.drawImage(backgroundImg, 0, minimap.yval, 640, 400, 0, 0, 640, 400 );
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
  
}
