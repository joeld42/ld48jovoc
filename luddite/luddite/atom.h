#ifndef ATOM_H
#define ATOM_H

/* ============================================================
 * atom.h
 * 
 * 'Atomize' a string, returning a const char * that points to 
 * an equivalent string but is guarenteed to be the same for 
 * any matching src string, and will also stick around for the 
 * lifetime of the app. Such strings can be used for fast indexing
 * and comparisons, etc, but are still simple to debug and work
 * with (unlike GUIDs or handles). 
 * 
 * Code is mostly lifted from "C Interfaces and Implementations"
 * by David Hanson
 * http://sites.google.com/site/cinterfacesimplementations/
 * with modifications to convert to C++ style
 * ============================================================
 */

namespace luddite
{    
int   Atom_length(const char *str);
const char *Atom_new   (const char *str, int len);
const char *Atom_string(const char *str);
const char *Atom_int   (long n);
}


#endif
