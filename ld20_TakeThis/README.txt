==============================
 LudumDare #20
 Take This!

Joel Davis (joeld42@yahoo.com)
==============================

This game was developed during the Ludum Dare 48 hour game contest.

Everything created by Joel Davis aka jovoc during the 48 hours.

==============================
LINUX NOTES:
==============================

I did include a linux binary, but that might not work if you don't
have the right libs installed. However, on linux (unlike other
platforms) it's very easy to build it for yourself.

Here's some build instructions:

Make sure the following things are installed:

SDL (incl. dev stuff)
libpng, zlib 

These are very often preinstalled for you, but if not it's pretty
easy but it depends on your distro. For example:

Ubuntu:
$ apt-get install libsdl-dev

Fedora:
$ yum install libsdl-dev

You also need to install CMake to build if it's not preinstalled.

To build (using Cmake):
$ cmake .
$ make

Then just run it:
$ ./takethis



==============================

INSTRUCTIONS:

Collect all three Triforces to win!!!
Kill monsters to get health and Rubees.
Chat with strange old men who live in caves.

The "voxel sprites" were created in Sproxel:
http://code.google.com/p/sproxel/

Source code is available at:
http://code.google.com/p/ld48jovoc/
Under the /ld20_TakeThis directory. This may be more recent than 
the contest version, if you want the exact compo source, check out
revision r146 (or so, I might make a few changes in the next few minutes).

CONTROLS

WASD or Arrows -- move around
Z -- Attack (once you have the sword)

press 'f' to toggle First Person mode, it is neat but still pretty buggy so it's not
enabled by default. If you get stuck, just toggle out of it and back in.

other keys:
i - toggle wireframe
b - toggle world bbox
o - fancy camera orbit (looks cool but not very playable)
