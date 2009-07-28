#include <iostream>
#include <string>
#include <tinyxml.h>

#include <allegro.h>
#include <alfont.h>

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

extern ALFONT_FONT *g_fontWireTiny;
extern ALFONT_FONT *g_fontWireHeadline; 

// Put curl in its own namespace, because it 
// includes windows, which defines BITMAP, which
// conflicts with allegro.
namespace curl {
#include <curl/curl.h>
}

#include "headline.h"

using namespace std;

Headline::Headline( std::string text, std::string src, 
				   std::string media_src, std::string url ) :
		m_text( text ), m_src( src ), m_mediaSrc( media_src ), m_url( url ),
		m_wirePic( NULL )
{

}

Headline::Headline() : m_wirePic( NULL )
{}			


Headline::~Headline() 
{
	if (m_wirePic)
	{
		destroy_bitmap( m_wirePic );
	}
}

HeadlinePool::HeadlinePool() :
	m_ready( false )
{

}

int progress_func( BITMAP *statusBmp, double tot, double now, double ultotal, double ulnow )
{

	printf( "Progress:: %f %f %f %f\n", tot, now, ultotal, ulnow );

	// Do we know the total # of bytes?
	alfont_set_font_size( g_fontWireTiny, 10 );

	// clear progress area
	rectfill( statusBmp, 11, 51, 449, 69, makecol32( 50, 60,100 ) );

	if (tot > 0.1)
	{
		// yes, draw a progress bar
		rectfill( statusBmp, 
			12, 52, 12 + (435.0 * (now/tot)), 68, makecol32( 100, 255, 255) );
	}
	
	// report size
	static char buff[256];
	sprintf( buff, "%0.0f bytes received.", now );
	
	alfont_textout( statusBmp, g_fontWireTiny, buff,
					 15, 55, makecol32( 255,0, 255 ) );

	acquire_screen();
	blit( statusBmp, screen, 0, 0, 150, 350, 500, 200 );
	release_screen();
	return 0;
}

// from curl examples
struct RSSData {
	char *memory;
	size_t size;
};

size_t
WriteMemoryCallback( void *ptr, size_t size, size_t nmemb, void *data )
{
	size_t realsize = size * nmemb;
	
	RSSData *rssdata = (RSSData*)data;
	rssdata->memory = (char *)realloc( rssdata->memory, rssdata->size + realsize + 1 );
	if (rssdata->memory)
	{
		memcpy( &(rssdata->memory[ rssdata->size]), ptr, realsize );
		rssdata->size += realsize;
		rssdata->memory[rssdata->size] = 0;
	}

	return realsize;
}

bool HeadlinePool::populate( std::string srcfile )
{
	cout << "In fill pool " << srcfile << endl;

	// Make a status bitmap
	BITMAP *statusBmp = create_bitmap( 500, 200 );
	rectfill( statusBmp, 0,0, 500, 200, makecol32( 100, 150, 255 ) );
	rect( statusBmp, 0, 0, 499, 199, makecol32( 100, 255, 255 ) );
	//textout( statusBmp, font, "Gathering news...", 10, 10, makecol32( 100, 255, 255 ) );

	alfont_set_font_size( g_fontWireHeadline, 24 );
	alfont_textout( statusBmp, g_fontWireHeadline, 
					"Gathering news...", 10, 10, makecol32( 100, 255, 255 ) );

	// init curl
	curl::curl_global_init( CURL_GLOBAL_WIN32 );


	// Get the feeds 
	TiXmlDocument *xmlDoc = new TiXmlDocument( srcfile.c_str() );
	if (!xmlDoc->LoadFile() ) {
		allegro_message( "ERROR: can't load file: %s\n", srcfile.c_str() );
		destroy_bitmap( statusBmp );
		return false;
	}

	TiXmlElement *xHeadlineFeeds, *xFeed;
	xHeadlineFeeds = xmlDoc->FirstChildElement( "HeadlineFeeds" );
	assert( xHeadlineFeeds );

	xFeed = xHeadlineFeeds->FirstChildElement( "Feed" );	
	while (xFeed)
	{
		std::string feedname = xFeed->Attribute( "name" );
		std::string feedurl = xFeed->Attribute( "url" );
		
		// How the title is formated
		//   "dash"   "Headline copy - Attribution"  (google style)
		//   "paren"  "Headline copy (Attribution)"  (yahoo style)
		//   "plain"  No attribution (assume feed name is attribution)
		std::string feedfmt = xFeed->Attribute( "format" );

		cout << "Feed URL " << feedurl << endl;

		// update progress
		rectfill( statusBmp, 10, 38, 400, 50, makecol32( 100, 150, 255 ) );
		

		std::string msg = "Checking feed ";
		msg += feedname;
		alfont_set_font_size( g_fontWireTiny, 10 );
		alfont_textout( statusBmp, g_fontWireTiny, msg.c_str(), 
			            10, 38, makecol32( 100, 255, 255 ) );		

		// draw the empty progress bar
		rectfill( statusBmp, 10, 50, 450, 70, makecol32( 50, 60,100 ) );
		rect( statusBmp, 10, 50, 450, 70, makecol32( 100, 255, 255 ) );

		acquire_screen();
		blit( statusBmp, screen, 0, 0, 150, 350, 500, 200 );
		release_screen();
		

		// use curl to slurp the feed.
		curl::CURL *creq_feed_url;
		curl::CURLcode result;
		RSSData rssdata;
		rssdata.memory = NULL;
		rssdata.size = 0;

		creq_feed_url = curl::curl_easy_init();
		if (creq_feed_url)
		{
			curl::curl_easy_setopt( creq_feed_url, curl::CURLOPT_URL, feedurl.c_str() );

			curl::curl_easy_setopt(creq_feed_url, curl::CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
			curl::curl_easy_setopt(creq_feed_url, curl::CURLOPT_WRITEDATA, (void *)&rssdata );		

			curl::curl_easy_setopt( creq_feed_url, curl::CURLOPT_NOPROGRESS, FALSE);
			curl::curl_easy_setopt( creq_feed_url, curl::CURLOPT_PROGRESSFUNCTION, progress_func );
			curl::curl_easy_setopt( creq_feed_url, curl::CURLOPT_PROGRESSDATA, statusBmp );

			curl::curl_easy_setopt( creq_feed_url, curl::CURLOPT_USERAGENT, "libcurl-agent/1.0");

			result = curl::curl_easy_perform( creq_feed_url );
			curl::curl_easy_cleanup( creq_feed_url );
		}

		
		// Go to next feed if this didn't work
		if (!rssdata.memory) continue;
		
		// DBG: output rss data for debugging
#if 0
		std::string fn = "dbgdata/";
		fn += feedname + "_rss.xml";		
		FILE *fp = fopen( fn.c_str(), "w" );
		fputs( rssdata.memory, fp );
		fclose( fp );
#endif

		// sanitize xml data -- TODO: do better
		for (char *sch = rssdata.memory; *sch; sch++)
		{
			unsigned char *ch = (unsigned char *)sch;

			// replace some special chars (accented vowels)
			if ((*ch>=142)&&(*ch<=145)) *ch = 'e';
			else if ((*ch>=135)&&(*ch<=140)) *ch = 'a';
			else if ((*ch>=151)&&(*ch<=155)) *ch = 'o';
			else if ((*ch>=146)&&(*ch<=149)) *ch = 'i';
			else if ((*ch>=156)&&(*ch<=159)) *ch = 'i';
			else if (*ch > 127) *ch = ' ';
		}

		// now make XML from rssdata
		TiXmlDocument *xmlFeedDoc = new TiXmlDocument();
		xmlFeedDoc->Parse( rssdata.memory );
		free (rssdata.memory );				

		TiXmlElement *xRss = xmlFeedDoc->FirstChildElement( "rss" );
		if (xRss)
		{
			TiXmlElement *xChannel = xRss->FirstChildElement( "channel" );			
			while (xChannel)
			{
				TiXmlElement *xItem = xChannel->FirstChildElement( "item" );			
				while (xItem)
				{
#if 0
					// DBG: List all child nodes
					TiXmlNode *xChild = xItem->FirstChild();
					while (xChild )
					{
						cout << "Value:" << xChild->Value() << endl;
						xChild = xChild->NextSibling();
					}					
					cout << "------" << endl;
#endif
					// the headline
					Headline hline;

					// Source
					hline.m_src = feedname;

					// get headline text
					TiXmlElement *xTitle = xItem->FirstChildElement("title");
					TiXmlNode *xText = xTitle->FirstChild();					
					//cout << " Title: " << xText->Value() << endl;
					hline.m_text = xText->Value();

					// Clean up special html characters in headlines
					// should be more general but (spartan voice) 
					// this.... is... speedhack!!!!
					boost::algorithm::replace_all(  hline.m_text, "&#39;", "'" );
					boost::algorithm::replace_all(  hline.m_text, "&#039;", "'" );
					boost::algorithm::replace_all(  hline.m_text, "&#96;", "`" );
					boost::algorithm::replace_all(  hline.m_text, "&#096;", "`" );
					boost::algorithm::replace_all(  hline.m_text, "&quot;", "\"" );
					boost::algorithm::replace_all(  hline.m_text, "&lt;", "<" );
					boost::algorithm::replace_all(  hline.m_text, "&gt;", ">" );
					
					// weird crap washington post puts in
					boost::algorithm::replace_all(  hline.m_text, "![CDATA[", "" );
					boost::algorithm::replace_all(  hline.m_text, "]]", "" );

					// handle attribution
					if ( (feedfmt == "dash") || (feedfmt == "paren"))
					{
						char sep;
						if (feedfmt == "dash") sep = '-'; else sep = '(';

						size_t dashpos = hline.m_text.rfind( sep );
						if (dashpos == string::npos)
						{
							// didn't find a sep, guess we're using feed as attribution
							hline.m_mediaSrc = feedname;
						}
						else
						{							
							size_t dashpos2 = dashpos+1;
							while ( isspace(hline.m_text[dashpos2])) dashpos2++;
							hline.m_mediaSrc = hline.m_text.substr( dashpos2, string::npos );							

							// clean up close paren for paren sources
							if ((feedfmt == "paren")&&(hline.m_mediaSrc.size()>2))
							{
								hline.m_mediaSrc = hline.m_mediaSrc.substr( 0, hline.m_mediaSrc.size()-1 );
							}

							while ((dashpos) && (isspace(hline.m_text[dashpos]))) dashpos--;
							hline.m_text = hline.m_text.substr( 0, dashpos );
						}
					} else {
						// assume format == "plain", use feed source as
						// attribution
						hline.m_mediaSrc = feedname;
					}

					// get url
					TiXmlElement *xLink = xItem->FirstChildElement("link");
					if (xLink)
					{
						xText = xLink->FirstChild();
						hline.m_url = xText->Value();
					}			

					m_headlines.push_back( hline );
															
					// next item
					xItem = xItem->NextSiblingElement( "item" );
				}
				xChannel = xChannel->NextSiblingElement( "channel" );
			}
		}

		
		// Next feed
		xFeed = xFeed->NextSiblingElement( "Feed" );
	}

	xmlDoc->Clear();
	delete xmlDoc;

#if 1
	cout <<  "GOT " << m_headlines.size() << " headlines!" << endl;
	for (std::vector<Headline>::iterator hi = m_headlines.begin();
			hi != m_headlines.end(); hi++)
	{
		cout << "HEADLINE: " << (*hi).m_text << endl;
		cout << "FEED:     " << (*hi).m_src << endl;
		cout << "SOURCE:   " << (*hi).m_mediaSrc << endl;
		cout << "LINK:     " << (*hi).m_url << endl << endl; 
	}
#endif

	// Create word list from assembled headlines
	for (std::vector<Headline>::iterator hi = m_headlines.begin();
										hi != m_headlines.end(); hi++)
	{
		Headline &h = (*hi);
		string line = h.m_text;
		string cleanline;

		for (int i=0; i < line.size(); i++)
		{
			if (isalpha(line[i]) || (isdigit(line[i])) )
			{
				cleanline += line[i];
			}
			else if (line[i]!='\'')
			{				
				cleanline += ' ';
			}
		}

		std::vector<std::string> splitline;
		boost::algorithm::split( splitline, cleanline, boost::algorithm::is_any_of(" \t"), 
			boost::algorithm::token_compress_on );
		for (int i=0; i < splitline.size(); i++)
		{			
			string hword = splitline[i];
			if (!hword.size()) continue;

			string wkey = splitline[i];
			std::transform( wkey.begin(), wkey.end(), wkey.begin(), tolower );
			//cout << i << "  >>" << wkey << "<< " << endl;

			WordInfoMap::iterator wi;
			wi = m_wordmap.find( wkey );

			// track headline words
			hi->m_words.push_back( wkey );

			if (wi == m_wordmap.end() )
			{
				// Not found
				WordInfo w;
				w.count = 1;
				w.m_word = splitline[i];
				
				// Add the word
				m_wordmap[ wkey ] = w;
				m_words.push_back( wkey );				
			}
			else
			{
				// found
				(*wi).second.count++;
			}
		}
	}

#if 0
	// print word list	
	for (WordInfoMap::iterator wi = m_wordmap.begin();
		 wi != m_wordmap.end(); wi++) 
	{
		cout << (*wi).second.count << "  " << (*wi).second.m_word << endl;
	}
#endif

	destroy_bitmap( statusBmp );
	return true;
}