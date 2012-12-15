import 'dart:html';

part 'lair_game.dart';
part 'minimap.dart';

void main() {
//  query("#text")
//    ..text = "Click me!"
//    ..on.click.add(reverseText);

  var game = new LairGame();
  game.startGame();

}

void reverseText(Event event) {
  var text = query("#text").text;
  var buffer = new StringBuffer();
  for (int i = text.length - 1; i >= 0; i--) {
    buffer.add(text[i]);
  }
  query("#text").text = buffer.toString();
}
