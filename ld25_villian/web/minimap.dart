
part of ld25_villian;

class MiniMap {
  
  num yval;
  Element minimapElem;
  Element frameElem;
  bool dragging;
  
  MiniMap()
  {
    yval = 0.0;
    frameElem = query( '#minimap-frame');
    minimapElem = query( '#minimap');
   
    minimapElem.on.mouseDown.add( startDrag );
    minimapElem.on.mouseMove.add( handleDrag );
    minimapElem.on.mouseUp.add( endDrag );
    
    frameElem.on.mouseDown.add( (MouseEvent ev) { ev.preventDefault(); } );
    frameElem.on.mouseMove.add( (MouseEvent ev) { ev.preventDefault(); } );
    frameElem.on.mouseUp.add( (MouseEvent ev) { ev.preventDefault(); } );
    
    dragging = false;
  }
  
  void startDrag( MouseEvent ev)
  {
    ev.preventDefault();
    dragging = true;
  }
  
  void endDrag( MouseEvent ev )
  {
    ev.preventDefault();
    dragging = false;    
  }
  
  void handleDrag(MouseEvent ev)
  {
    ev.preventDefault();
    
    if (!dragging) return;
    
    num clickYpos = ev.clientY - minimapElem.offsetTop;
     
    num frameYpos = clickYpos - 40;
    if (frameYpos < 0.0) {
      frameYpos = 0.0;
    }
    if (frameYpos > 320) {
      frameYpos = 320;
    }
    print("frameYpos is ${frameYpos}");
    
    frameYpos += minimapElem.offsetTop;
    
    
    frameElem.style.top = "${frameYpos}px";
    yval = (clickYpos-40) * 5;
    if (yval > 1800) {
      yval = 1800;
    }
    
    if (yval < 0) {
      yval = 0;
    }

  }
}
