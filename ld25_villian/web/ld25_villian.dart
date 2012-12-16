import 'dart:html';
import 'dart:math';

part 'lair_game.dart';
part 'minimap.dart';
part 'map_tile.dart';
part 'room.dart';
part 'agent.dart';

void main() {
  var game = new LairGame();
  game.startGame();
}
