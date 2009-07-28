#include <stdio.h>
#include <map>
#include <algorithm>
#include <assert.h>

#include "musicdb.h"

using namespace std;

void cleanString( char *str )
{
	if (str[strlen(str)-1]=='\n')
	{
		str[strlen(str)-1] = 0;
	}

	for (char *ch = str; *ch; ++ch )
	{	
		// TODO: incomplete
		if (*ch =='ö') *ch = 'o'; // Bjork fix
		if ((*ch =='é') || (*ch == 'è') || ( *ch =='ê') || (*ch =='ë') ) *ch = 'e';
		if ((*ch =='á') || (*ch == 'à')) *ch = 'a';
		if (*ch =='/') *ch = '\\';

		// replace unhandled ones with space
		if (*ch <= 0) *ch = 32;
	}

}

size_t LoadSongList( SongList &songs )
{
	size_t count = 0;

	FILE *fp = fopen( "musicdb.txt", "rt" );

	if (!fp) return 0;

	string artist;
	string fullpath;
	string title;
	char line[6144], *ch;

	while (!feof(fp))
	{
		fgets( line, 6144, fp );
		cleanString( line );

		ch = strtok( line, "\t" );
		if (ch) artist = ch; else continue;
		
		ch = strtok( NULL, "\t" );
		if (ch) title = ch; else continue;

		ch = strtok( NULL, "\t" );
		if (ch) fullpath = ch; else continue;		

		printf ("Artist: %s\nTitle: %s\nFullPath: %s\n",
				artist.c_str(), 
				title.c_str(), 
				fullpath.c_str() + 60 );
		count++;
		
		SongList::iterator sli;
		sli = songs.find( artist );
		if (sli == songs.end())
		{
			songs[artist] = std::vector<Song>();
			sli = songs.find( artist );

			assert( sli != songs.end() );
		}
		
		Song s;
		s.filename = fullpath;
		s.title = title;
		(*sli).second.push_back( s );
	}
	fclose(fp);

	return count;
}