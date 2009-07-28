=======================
       BAD  FOOD
-----------------------
A 48 hour contest game
that took longer than
48 hours.
=======================

I wasn't able to finish my entry on time during the contest,
but I didn't give up, and finished it the week afterward. I've
probably spent less then 48 hours on it in total, but at
convienant times and with plenty of rest, so it's not really
comparable to the 48 hour entrants.

Anyways, this is the game, instructions are provided in the game.
It was made using allegro, graphics were made with ProMotion and
Lightwave for the title screen, sound effects with Oranginator. 

CONTROLS:

Use WASD or Arrow Keys to move the player around. In discard mode,
use left and right to choose a card to discard. If you have three
cards, you can press ENTER at any time to CALL.

HINTS AND TIPS:

- When discarding, just let the timer run out instead of hitting 
  enter. This lets you breathe for a second and there is less chance
  of accidently CALLing when you don't want to.

- Don't raise if you don't have a good hand. It's fun to collect the money
  icons but this isn't nibbles, and that's not always a good idea.

- If you're going for a combo that uses a lot of the shared cards
  (like a flush with 3 cards from the shared cards) remember the dealer
  probably can make the flush as well.

- Try to pick up just cards first, the just raises when you are ready
  to call. But be flexible with this.

- The cards are really drawn from a deck (actually a deque), so if you 
  miss a card you want, it will not be coming around again. When the deck
  runs out, you automatically CALL or FOLD (if you have < 3 cards)

LEVEL EDITOR:

There is a very crappy level editor included, press F11 to activate
it. It's really hard to use, and you have to cut and paste the level 
you made back into the source code and recompile, but if you are curious
here are the controls:

Keypad  +/- : change floor tile at cursor
Regular +/- : change wall tile at cursor
Keypad .    : put down same as last modified floor tile
  0 (zero)  : put down same as last modified wall tile
  O (letter): mark cursor for box later
  P         : draw box with last floor tile from mark to cursor
  *         : save level to level.txt (have to paste into src to use)
  C         : clear all walls

KNOWN BUGS:

--Sometimes if you have negative money it lets you
play one last round. Make it count..

--Tiebreaks are handled incompletely, only the highest 
extra card is looked at. This is pretty rare, and happens 
in the player's favor when it comes up so it's not a huge
deal, but it's annoying.