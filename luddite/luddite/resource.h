#ifndef RESOURCE_H
#define RESOURCE_H

// Simplest possible resource manager -- pass in an (atomized) filename and a 
// function that will load that file and it will load it if it hasn't already
// been loaded.
//
// There's no free/release ... i might add one but generally resources seem to
// stick around for the life of the app, at least in LD48 stuff. 

// For resources addressed by a pointer
typedef void* (*ResourcePtrFunc)(const char *);
void *ResourcePtr_get( const char *atomized_name, 
				   ResourcePtrFunc resfunc );


// For resources addressed by a handle (i.e. GLuint)
// for example, GL Textures
typedef unsigned int (*ResourceFunc)(const char *);
unsigned int Resource_get( const char *atomized_name, 
						   ResourceFunc resfunc );

#endif
