import 'dart:async';
import 'dart:html' as html;
import 'dart:js';
//import 'package:js/js.dart' as js;
import 'package:stagexl/stagexl.dart';
import 'dart:math';
import 'dart:convert';


class GridCell {
  int x;
  int y;
  int g1;
  int g2;
  int count;
}

var gamesList = html.querySelector('#gameslist');

void onDataLoaded(String responseText) {
  var jsonString = responseText;
  print(jsonString);
  Map parsedMap = JSON.decode(responseText);

  String resultString = "";

  var games = parsedMap['games'];

  var gamesPerRow = 4; // Note: Also update gameDiv column class when changing this
  var numRows = games.length / gamesPerRow;
  for (int row = 0; row < numRows; row++)
  {
    var rowDiv = new html.DivElement();
    rowDiv.classes.add('row');

    for (int ndx=0; ndx < gamesPerRow; ndx++) {

      var gameIndex =row*gamesPerRow + ndx;
      if (gameIndex>=games.length) {
        break;
      }
      var item = games[gameIndex];

      var gameOuterDiv = new html.DivElement();
      gameOuterDiv.classes.add('three');
      gameOuterDiv.classes.add('columns');


      var gameDiv = new html.DivElement();
      gameDiv.classes.add('gameitem');
      gameOuterDiv.children.add(gameDiv);

      var thumbSrc = item['coverArt'];
      if (thumbSrc==null) {
        thumbSrc = "https://static.jam.vg/content/internal/tvfail.png.480x384.fit.jpg";
      }
      var gameThumbnail = new html.ImageElement();
      gameThumbnail.classes.add('thumb-image');
      gameThumbnail.src = thumbSrc;
      gameDiv.children.add(gameThumbnail);

      var gameLink = new html.AnchorElement();
      gameLink.href= "https://ldjam.com" + item['url'];
      gameLink.target = "_blank";
      gameDiv.children.add(gameLink);

      var gameTitle = new html.DivElement();
      gameTitle.classes.add('thumb-overlay');
      gameLink.children.add(gameTitle);

      var gameText = new html.DivElement();
      gameText.classes.add('thumb-text');
      gameText.text = item['title'];
      gameTitle.children.add(gameText);

      rowDiv.children.add( gameOuterDiv );
    }
    gamesList.children.add( rowDiv );
  }

}


Future<Null> main() async {
  
  //context.callMethod('testFunc', 0 );
  var genres = context['genres'];
  var genreCounts = context['genreCounts'];
  var genreKeys = context['genreKeys'];
  var cellColors = [
    0xFF202C11,0xFF1D3920,0xFF154634,0xFF0D524C,0xFF1A5D65,
    0xFF38657C,0xFF5F6B8E,0xFF886F97,0xFFB17098,0xFFD4738F,0xFFEF7B80
  ];
  
  StageOptions options = new StageOptions()
    ..backgroundColor = Color.Black
    ..renderEngine = RenderEngine.WebGL
    ..antialias = true;

  var canvas = html.querySelector('#stage');
  var genreTitle = html.querySelector('#genre-title');

  int WIDTH = canvas.clientWidth;
  int HEIGHT = canvas.clientHeight;
  int SZ = min( WIDTH, HEIGHT ) - 250;
  //var stage = new Stage(canvas, width: 1280, height: 800, options: options);
  var stage = new Stage(canvas, width: WIDTH, height: HEIGHT, options: options);

  var renderLoop = new RenderLoop();
  renderLoop.addStage(stage);

  //var resourceManager = new ResourceManager();
  //resourceManager.addBitmapData("dart", "images/dart@1x.png");

  //await resourceManager.load();

/*
  var logoData = resourceManager.getBitmapData("dart");
  var logo = new Sprite();
  logo.addChild(new Bitmap(logoData));

  logo.pivotX = logoData.width / 2;
  logo.pivotY = logoData.height / 2;

  // Place it at top center.
  logo.x = WIDTH / 2;
  logo.y = 0;

  stage.addChild(logo);

  // And let it fall.
  var tween = stage.juggler.addTween(logo, 3, Transition.easeOutBounce);
  tween.animate.y.to( HEIGHT / 2);

  // Add some interaction on mouse click.
  Tween rotation;
  logo.onMouseClick.listen((MouseEvent e) {
    // Don't run more rotations at the same time.
    if (rotation != null) return;
    rotation = stage.juggler.addTween(logo, 0.5, Transition.easeInOutCubic);
    rotation.animate.rotation.by(2 * PI);
    rotation.onComplete = () => rotation = null;
  });
  logo.mouseCursor = MouseCursor.POINTER;
*/
  int num = genres.length;
  int rowSz = (SZ / num).round();

  int lineColor = 0xFFAE2850;



  int gridX = (( WIDTH - (num*rowSz)) / 2 ).round();
  int gridY = 120;
  /*
  // Draw grid
  var grid = new Shape();
  //grid.graphics.circle( 100, 100, 60 );

  //grid.graphics.strokeColor( Color = Color.AliceBlue)
  for (int i = 0; i <= num; i++) {
    grid.graphics.strokeColor( lineColor, 1.0 );
    grid.graphics.moveTo(i * rowSz, 0);
    grid.graphics.lineTo(i * rowSz, num*rowSz);
  }

  for (int i = 0; i <= num+1; i++) {
    grid.graphics.strokeColor( lineColor, 1.0 );
    grid.graphics.moveTo(0, i * rowSz);
    grid.graphics.lineTo(num*rowSz, i * rowSz );
  }
  grid.x = gridX;
  grid.y = gridY;

  stage.addChild( grid );
  */

  var grid = new Sprite();
//  grid.graphics.rect(0, 0, rowSz*num, rowSz*num);
//  grid.graphics.strokeColor( Color.Orange );
  stage.addChild(grid);
//  grid.x = gridX;
//  grid.y = gridY;

  int currX = gridX;
  int currY = gridY;

  var labelColor = 0xFFE99526;
  var textFormat = new TextFormat('Cairo', rowSz*0.6,  labelColor );
  genres.forEach((item) {

    // Row Label
    var rowLabel = new TextField();
    rowLabel.defaultTextFormat = textFormat;
    rowLabel.autoSize = TextFieldAutoSize.RIGHT;
    rowLabel.text = item;
    rowLabel.x = gridX - 105;
    rowLabel.y = currY;
    rowLabel.width = 100;
    rowLabel.height = 50;
    grid.addChild(rowLabel);
    currY += rowSz;

    // Col Label
    var colLabel = new TextField();
    colLabel.defaultTextFormat = textFormat;
    colLabel.autoSize = TextFieldAutoSize.LEFT;
    colLabel.text = item;
    colLabel.x = currX;
    colLabel.y = gridY - 8;
    colLabel.width = 100;
    colLabel.height = 50;
    colLabel.rotation = -70;
    grid.addChild(colLabel);
    currX += rowSz;
  });

  // Make the row/column highlight
  var bigTextFormat = new TextFormat('Cairo', rowSz,  0xAA000000 );
  int highlightColor = 0x44FFFFFF;
  var rowHighlight = new Sprite();
  rowHighlight.mouseEnabled = false;
  rowHighlight.graphics.rect( 0, 0, rowSz * num, rowSz);
  rowHighlight.graphics.fillColor( highlightColor );
  rowHighlight.pivotY = rowSz / 2.0;

  var rowBigLabel = new Sprite();
  rowBigLabel.graphics.rectRound( rowSz*num + 5, -10, 200, 40,   10, 10 );
  rowBigLabel.graphics.fillColor( labelColor );

  var rowBigLabelText = new TextField();
  rowBigLabelText.defaultTextFormat = bigTextFormat;
  rowBigLabelText.text = "Genre";
  rowBigLabelText.x = rowSz*num + 10;
  rowBigLabelText.y = -4;
  rowBigLabelText.width = 190;
  rowBigLabelText.height= 30;
  rowBigLabelText.autoSize = TextFieldAutoSize.LEFT;
  rowBigLabel.addChild( rowBigLabelText );

  rowHighlight.addChild(rowBigLabel);

  grid.addChild(rowHighlight);

  var colHighlight = new Sprite();
  colHighlight.mouseEnabled = false;
  colHighlight.graphics.rect( 0, 0, rowSz, rowSz * num);
  colHighlight.graphics.fillColor( highlightColor );
  colHighlight.pivotX = rowSz / 2.0;

  var colBigLabel = new Sprite();
  colBigLabel.graphics.rectRound( -100, rowSz*num + 5, 200, 40,   10, 10 );
  colBigLabel.graphics.fillColor( labelColor );


  var colBigLabelText = new TextField();
  colBigLabelText.defaultTextFormat = bigTextFormat;
  colBigLabelText.text = "Genre";
  colBigLabelText.x = -95;
  colBigLabelText.y = rowSz*num + 10;
  colBigLabelText.width = 190;
  colBigLabelText.height= 30;
  colBigLabelText.autoSize = TextFieldAutoSize.CENTER;
  colBigLabel.addChild( colBigLabelText );

  colHighlight.addChild(colBigLabel);

  grid.addChild(colHighlight);

  var labelNum = new Sprite();
  labelNum.mouseEnabled = false;
  labelNum.graphics.circle(0, 0, rowSz * 0.8);
  labelNum.graphics.fillColor( labelColor );
  labelNum.graphics.strokeColor( 0xFFFFFFFF );
  //labelNum.pivotX = rowSz*0.75;
  labelNum.pivotY = rowSz*1.8;

  var labelNumText = new TextField();
  labelNumText.defaultTextFormat = new TextFormat('Cairo', rowSz, 0xAA000000 );
  labelNumText.text = "0";
  labelNumText.width = 50;
//  labelNumText.height = 30;
  labelNumText.pivotX = 25;
  labelNumText.pivotY = 10;
  labelNumText.autoSize = TextFieldAutoSize.CENTER;
  labelNum.addChild( labelNumText );

  rowHighlight.visible = false;
  colHighlight.visible = false;
  labelNum.visible = false;
  grid.addChild( labelNum );


  // Set up the grid cells
  List<GridCell> cells = new List(num*num);
  for (int j=0; j < num; j++) {
    for (int i = 0; i < num; i++) {
      var cell = new GridCell();
      cell.x = gridX + i * rowSz;
      cell.y = gridY + j * rowSz;
      cell.g1 = i;
      cell.g2 = j;

      cells[j*num+i] = cell;

      var cellShape = new Sprite();
      cellShape.graphics.rect( 0, 0, rowSz-1, rowSz-1 ); // imply the grid
      //cellShape.graphics.fillColor( Color.Red );


      int cellColor;
      int gc = genreCounts[j*num+i];
      cell.count = gc;

      if (gc >= cellColors.length) {
        cellColor = cellColors[cellColors.length-1];
      } else {
        cellColor = cellColors[gc];
      }
      cellShape.graphics.fillColor( cellColor );
      grid.addChildAt( cellShape, 0 );
      cellShape.pivotX = cellShape.width / 2;
      cellShape.pivotY = cellShape.height / 2;
      cellShape.x = cell.x + cellShape.pivotX;
      cellShape.y = cell.y + cellShape.pivotY;

      cellShape.userData = cell;

      cellShape.onMouseOver.listen((MouseEvent e) {
        cellShape.scaleX = 1.5;
        cellShape.scaleY = 1.5;

        rowHighlight.visible = true;
        colHighlight.visible = true;
        labelNum.visible = true;

        rowHighlight.x = gridX;
        rowHighlight.y = cellShape.y;
        colHighlight.x = cellShape.x;
        colHighlight.y = gridY;

        labelNum.x = cellShape.x;
        labelNum.y = cellShape.y;

        GridCell cell = cellShape.userData;
        labelNumText.text = cell.count.toString();
        rowBigLabelText.text = genres[cell.g2];
        colBigLabelText.text = genres[cell.g1];

        //if (rotation != null) return;
//        rotation = stage.juggler.addTween(logo, 0.5, Transition.easeInOutCubic);
//        rotation.animate.rotation.by(2 * PI);
//        rotation.onComplete = () => rotation = null;
        grid.setChildIndex( rowHighlight, grid.numChildren-1);
        grid.setChildIndex( colHighlight, grid.numChildren-1);

        grid.setChildIndex( cellShape, grid.numChildren-1);
        grid.setChildIndex( labelNum, grid.numChildren-1);
      });

      cellShape.onMouseOut.listen((MouseEvent e) {
        cellShape.scaleX = 1.0;
        cellShape.scaleY = 1.0;
      });

      cellShape.onMouseClick.listen((MouseEvent e) {
        GridCell cell = cellShape.userData;
        genreTitle.innerHtml = genres[cell.g1] + " + " + genres[cell.g2];

        // TODO Spinner
        gamesList.children = [];

        var url = "http://localhost:8080/combo/${genreKeys[cell.g1]}/${genreKeys[cell.g2]}";
        var request = html.HttpRequest.getString(url).then(onDataLoaded);

      });

    }

    grid.onMouseRollOut.listen((MouseEvent e) {
      rowHighlight.visible = false;
      colHighlight.visible = false;
      labelNum.visible = false;
    });

  }






  // See more examples:
  // https://github.com/bp74/StageXL_Samples
}
