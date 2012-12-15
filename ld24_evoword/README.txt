
EvoWord
--------

A 48 hour game by Joel Davis (joeld42@gmail.com) for the LudumDare 24 contest.

Source code is available at:
http://code.google.com/p/ld48jovoc/
This project is in the ld24_evoword subdirectory:
http://code.google.com/p/ld48jovoc/source/browse/#svn%2Fld24_evoword

Post-compo changes:
- Fixed possible crash bugs
- Fixed bug with bubbles crashing (but left bubbles disabled since
  they weren't in the compo release)
- Added missing dylibs for mac
- Changed mainloop to use a timer to work with screen capture
  software.

Instructions
-------------
Explore the creature's genome by altering one letter at a time. Drag
letters with the mouse from the evolutionary soup to the word. You
can add letters between two letters or at the front or back of the
word. Or you can replace letters in the word. 

Press RETURN to save the creature in the next jar. When all jars are
full, your game is complete and you will see a diagram of the
evolutionary path you took. When you jar a creature, you can
resume from any of the previously jarred creatures.

You may not use words previously used.

Scoring
--------
Letters are worth the following base point values:

1 Point : A E I N O R S T U 
2 Points: C D G H K L P
3 Points: B F M W Y
4 Points: J Q X Z

When you make a new word, you get the sum of the point values of the 
letters SQUARED! For example:

       W   O   R   D
WORD = 3 + 1 + 1 + 2 = 7 squared = 49 Points!

If you try to make a word and the word is wrong or you've already
used the word, you lose the base point value (not squared), 
e.g. 7 points in the example above.
