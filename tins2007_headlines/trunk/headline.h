#ifndef HEADLINE_H
#define HEADLINE_H

#include <iostream>

#include <allegro.h>

#include <vector>
#include <string>
#include <map>

struct Headline
{
	Headline();
	Headline( std::string text, std::string src, std::string media_src, std::string url );
	~Headline();

	// headline data
	std::string m_text;
	std::string m_src;  // The name of the RSS source
	std::string m_mediaSrc; // The news organization src (i.e. AP, Reuters)
	std::string m_url; // The url of the story itself

	std::vector<std::string> m_words; // headline words;

	// Only for "live" headlines
	BITMAP *m_wirePic;
	int m_currY;
};

struct WordInfo
{
	std::string m_word; // Text with origional caps
	size_t count;	
};

struct HeadlinePool
{
	HeadlinePool();

	// All the headlines
	std::vector<Headline> m_headlines;

	// The words
	typedef std::vector< std::string >  WordKeys;
	typedef std::map< std::string, WordInfo > WordInfoMap;
	WordInfoMap m_wordmap;
	WordKeys m_words;

	// Initialize headline sources	
	bool populate( std::string srcfile );

	// is all our data ready
	bool m_ready;
};

#endif