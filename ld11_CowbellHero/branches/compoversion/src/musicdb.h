#ifndef FOCUS_MUSICDB_H
#define FOCUS_MUSICDB_H

#include <string>
#include <vector>
#include <map>

struct Song
{	
	std::string title;
	std::string filename;
};

typedef std::map< std::string, std::vector<Song> > SongList;

size_t LoadSongList( SongList &songs );

#endif