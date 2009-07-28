======================
  Headline Recycler
======================
by Joel Davis (joeld42@yahoo.com)

Written for TINS 2007. All graphics (except for fonts), 
data and game code done by me entirely in one weekend.

---- Installation -----

The TINS contest package may contain only source and data files. If you want
to play Headline Recycler without trying to build it (and I would strongly
advise this), just get the complete binary from:

http://www.vickijoel.org/tins2007/headline.zip

If you are on linux... stay tuned.

---- How to Play -----

Recycle old headlines to make new ones! Drag old clippings from the 
Recycle bin into your folder. Assemble a headline on the pasteboard 
that matches one of the wire stories, and press the PUBLISH button.
If it's a match, it will go onto the front page!

If your workspace is getting cluttered, drag unused clippings to your
furnace to burn them up. And if you can't find the right word, use the
xacto knife and the scissors to cut up words on the pasteboard to assemble
your headline.

Don't worry about capitals or punctation, your editor takes care of
all of that for you. But you better finish before the deadline!!!!

---- Known Bugs -----

* Sometimes headlines are displayed in the wire news
with the last word repeated like this this this.

---- How to Build -----

Good luck. It should work fine on windows or linux, but the only 
project files I'm including are the MSVC 2005 Solution files. Hopefully
I'll have linux and MinGW makefiles after the contest.

It requires the following libraries:
* Allegro (of course)
* Alfont (for text rendering)
* libCurl (http://curl.haxx.se/)  See note. For retreiving URLS.
* boost (http://www.boost.org/) For some string processing and stuff.
* TinyXML  (http://www.grinninglizard.com/tinyxml/) source code is included in this archive

I had some with libCurl on windows. The prebuilts didn't work for me, and it required a
tiny change to build. Email me if you're having trouble building this and I can dig up 
what I did to get it to work.






