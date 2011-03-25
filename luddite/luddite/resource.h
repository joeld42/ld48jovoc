#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>
#include <algorithm>
#include <map>

#include "handle.h"
#include "debug.h"

// The 'resource mgr' is built on top of the handle mgr and
// provides on-demand loading and name-based lookup
// 
// DATA -- the internal data type to store, just like in
// handle. Should be somewhat lightweight as it gets passed by
// value. There must exist the functions:
//
//    bool loadResource( const char *filename, DATA *item );
//    void unloadResource( DATA *item );
//
// These functions populate the default-constructed DATA with real
// resources, and free those resources, respectively.
//
// HANDLE -- The type of the handle, for example
// Handle<tag_TEXTURE>. This is the type that the user will interact
// with.
//
// To use this class, you only need to implement the loadResource and
// unloadResource functions above. You may also want to subclass it to
// provide accessors to the internal data from the handle.
//
template <typename DATA, typename HANDLE > 
class ResourceMgr
{

private:
    typedef HandleMgr<DATA,HANDLE> HMgrType;
    
    // Was using a std_ext::hash, caused all sorts of headaches, 
    // now back to good ol' std::map
	typedef std::map< std::string, HANDLE> ResourceHash;

protected:
    HMgrType     m_resMgr;    
    ResourceHash m_nameIndex;
    
public:
    ResourceMgr() {}
    ~ResourceMgr();
    
    // Gets a resource, loading it if needed
    HANDLE getResource( const char *name, bool _dontLoad=false );
    void   freeResource( HANDLE );
	
	// Reports resource usage .. will call ReporterFunc once for
	// each resource and include name and reference count
	typedef void (*ReporterFunc)( const std::string &, unsigned int, HANDLE );
	void doReport( ReporterFunc cbReportFunc=NULL);
};

template <typename DATA, typename HANDLE >
ResourceMgr<DATA,HANDLE>::~ResourceMgr()
{
    // Release all of our resources
    for ( typename ResourceHash::iterator ri = m_nameIndex.begin();
          ri != m_nameIndex.end(); ++ri )
    {
        HANDLE hRes = (*ri).second;
        if (!hRes.isNull())
        {            
            DATA *data = m_resMgr.deref( hRes );        

            // If the res mgr is getting deleted, just nuke everything,
            // don't worry about refcount
            unloadResource( data );

            // don't need to deallocate, handleMgr will do so            
        }
        
    }    
}

template <typename DATA, typename HANDLE>
HANDLE ResourceMgr<DATA,HANDLE>::getResource( const char *name, bool _dontLoad )
{
    HANDLE hRes;

	typename ResourceHash::iterator ri = m_nameIndex.find( name );
	if ( ri != m_nameIndex.end() )
	{
		hRes = (*ri).second;		
	}
    
    // if the handle is NULL, either because this is something new or 
    // previously deleted, aquire it's resource
    if (hRes.isNull())
    {    
        DATA *data = m_resMgr.acquire( hRes );

		if (!_dontLoad)
		{
			DBG::info( "Not Found, loading\n", name );

			// Load the resource
			if (!loadResource( name, data ))
			{
				// todo: error check
				DBG::warn("failed to load resource %s", name );
				return HANDLE();            
			}

		}

		// Add it to the map
		m_nameIndex[ name ] = hRes;
		DBG::info( "adding to map\n" );
	}

	DBG::info( "Now have resource %d\n", hRes.getHandle() );

    // increment the ownership for this 
    m_resMgr.addrefCount( hRes );    
    
    // return it
    return hRes;    
}

template <typename DATA, typename HANDLE>
void ResourceMgr<DATA,HANDLE>::freeResource( HANDLE hRes )
{
    // decref (and possibly unload) the refcount
    DATA *data = m_resMgr.decrefCount( hRes );
    
    // Unload data if we need to
    if (data) 
    {
        // Set it to a NULL handle in the resource index
        // CBB: make constant time
        for ( typename ResourceHash::iterator ni = m_nameIndex.begin();
              ni != m_nameIndex.end(); ++ni )
        {
            // If this is our handle, set it to a null handle
            if (ni->second == hRes)
            {
                ni->second = HANDLE();
                break;                
            }
            
        }
        

        // Unload any type-specific resources
        unloadResource( data );
    }    
}

template <typename DATA, typename HANDLE>
void ResourceMgr<DATA,HANDLE>::doReport( typename ResourceMgr<DATA,HANDLE>::ReporterFunc cbReportFunc )
{	
	AssertPtr( cbReportFunc );
	
	for ( typename ResourceHash::iterator ri = m_nameIndex.begin();
          ri != m_nameIndex.end(); ++ri )
    {
		
		HANDLE hRes = (*ri).second;
		
		cbReportFunc( (*ri).first, m_resMgr._refCount( hRes ), hRes );
		//DBG::info("%10s | %d\n", 
		//		(*ri).first.c_str(), 
		//		m_resMgr._refCount( hRes ) );
	}	
}

#endif


