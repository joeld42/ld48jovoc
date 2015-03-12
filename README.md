Introduction
----

This is a collection of all my games made for the 
[LudumDare 48 hour game programming contest](http://www.ludumdare.com), 
and similar contests like Speedhack and TINS. These games are mostly pretty rough but some are already 
quite fun.


<img src="http://www.ludumdare.com/compo/wp-content/uploads/2010/08/ld_montage.jpg">


List of Games
-----
(bolded entries are the ones in this package)
  
  * LD31: *One Screen* -- [Leviathan](https://www.youtube.com/watch?v=iGt-Aj_CzvI)
  * LD30: *Connected Worlds -- Untitled World Game*
  * LD26: *Minimalism -- Minimum RPG*
  * LD25: *You Are the Villian* -- [Evil Lair](http://www.vickijoel.org/ld25/ld25_villian.html)
  * LD24: *Evolution* -- [EvoWord](http://www.youtube.com/watch?v=Wr5etU6131M)
  * LD22: Alone -- [Aloneasaurus Rex](http://www.youtube.com/watch?v=_za96P1ckI8)
  * LD21: Escape -- No Entry
  * LD20: *It's Dangerous to go alone* -- [Take This](http://www.youtube.com/watch?v=IZgl6_976U4)
  * LD19: *Discovery* -- Planet Thing
  * LD18: *Enemies as Weapons -- Iron Reflex*
  * LD17: *Islands -- Crater Galapagos*
  * LD16: Exploration -- No Entry
  * LD15: *Caverns -- Spaceship in a Cave*
  * LD14: *Advancing Wall of Doom -- Glacier*
  * LD13: *Roads -- Crossroads*
  * LD12: *The Tower -- Moonbase Defense*
  * LD11: *Minimalist -- Cowbell Hero*
  * LD10.5: Weird/Unexpected/Surprise -- No Entry
  * LD10: *Chain Reaction -- Jumping Chain Guy* (barely started)
  * LD9: Build the level you play -- No Entry (i think)
  * LD8.5: Moon, Anti-Text -- No Entry
  * LD8: *Swarms -- Bugzapper*
  * LD7: *Growth -- Kudzu Commander*
  * LD6: Light and Darkness -- No Entry
  * LD5: *Random -- Buttonlands*
  * LD4: *Infection -- Bad Food* (2nd try)
  * LD4: *Infection -- !BioWar* (abandoned)
  * LD3: *Preparation -- Hannibal's Cannibals* (lost the code)
  * LD2: *Construction/Destruction (Minor Theme: Sheep) -- Trans-Icelandic Express*
  * LD1: *Guardian -- Zillagame* (the game that started it all for me)
  * LD0: Indirect Interaction (I didn't know about LD yet)
  * MiniLD16: *Constraints -- Moonsweeper* (not included here)
  * MiniLD11: *Sandbox -- The Wanderer*
  * MiniLD9: *Audio/Sound -- !JamClock*  
  * Speedhack 2003 - *Outlands* (my first allegro game)
  * !LudumDare Halloween Hack 2003 - *The Halloween Machine*
  * TINS2006 - *Railroad Merchant aka Merchant of Venus*
  * TINS2007 - *Headline Recycler*

Technology
-----

Mostly these are built with C++ and a bit of python. They use either [allegro](http://www.allegro.cc) or SDL/OpenGL
(with one oddball Ogre experiment)

Newer ones are experiments with haxe/snowkit.

Some of the games includes a branch with the original version as it was after the 48 hours, and the 'trunk' which may
contain future post-contest updates. Newer ones might just be tagged with the ludumdare number.

Part of the reason for creating this repository is to share the games with the world, adding to the pool of open
source games. But I'm also planning to add future games to this repo, so during future contests you can watch me code
in real time (or as often as I check in). 

Building the Games
-----

The build system is pretty craptastic right now. There's a CMake project that will build some of the games on linux
but it will certainly need some tweaking to work for you. For windows, there are a few msdev project files but they
will probably need so much work you'd be better off starting from scratch.

I've updated it a bit, some of them build and run on MacOS now, if you have SDL installed.

The Minibasecode
----

I've collected a few bits and pieces of the code into a "minibasecode" subdirectory, and there's a script in the
"ld_template" directory that will do a lot of the work of setting up a new entry for me, with a basic game loop and
drawing something on the screen. It's pretty buggy so I wouldn't really recommend it using it unless you are me and
know how to workaround all the cruft, but that's what I start my ludumdare games with these days.

I'm also working on a "real" game engine called luddite (which lives elsewhere), unfortunately it is not yet in a
useful state. The "luddite" directory in here is a precursor to that, it's all dead code and isn't very useful
anyways.

There's also an experimental snowkit basecode, but I wouldn't suggest using it.
