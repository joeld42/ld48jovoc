#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>
#include <algorithm>
#include <map>

#include "handle2.h"
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
    typedef JBDHandleMgr<DATA,HANDLE> HMgrType;
   
    struct ResInfo
    {
        HANDLE m_hRes;
        size_t m_refCount;
        std::string m_resName;
    };
    
    // Was using a std_ext::hash, caused all sorts of headaches, 
    // now back to good ol' std::map
	typedef std::map<std::string, ResInfo> ResourceHash;

protected:
    HMgrType     m_resMgr;    
    ResourceHash m_nameIndex;
    
public:
    ResourceMgr() {}
    ~ResourceMgr();
    
    // Gets a resource, loading it if needed
    HANDLE getResource( const char *name );
 
    // derefs but doesn't actually unload resource 
    void   freeResource( HANDLE );

    // unloads all unused resources
    void unloadUnusedResources();

    // for debugging
    void dbgPrintUsage();
    
};

template <typename DATA, typename HANDLE >
ResourceMgr<DATA,HANDLE>::~ResourceMgr()
{
    // Release all of our resources
    for ( typename ResourceHash::iterator ri = m_nameIndex.begin();
          ri != m_nameIndex.end(); ++ri )
    {
        HANDLE hRes = (*ri).second.m_hRes;
        if (!hRes.isNull())
        {            
            // FIXME: do something here...
        }
    }    
}

template <typename DATA, typename HANDLE>
HANDLE ResourceMgr<DATA,HANDLE>::getResource( const char *name )
{
    HANDLE hRes;

	typename ResourceHash::iterator ri = m_nameIndex.find( name );
	if ( ri != m_nameIndex.end() )
	{
        ResInfo &info = (*ri).second;
        
        // get the resource
		hRes = info.m_hRes;

        // incr the refence count
        info.m_refCount++;        
	}
	
    // if the handle is NULL, either because this is something new or 
    // previously deleted, aquire it's resource
    if (hRes.isNull())
    {    
        DATA *data = m_resMgr.acquire( hRes );

		// Load the resource
        if (!loadResource( name, data ))
        {
            // todo: error check
            return HANDLE();            
        }
    
		// Add it to the map
        ResInfo info;
        info.m_hRes = hRes;
        info.m_refCount = 1; // Count this reference
        info.m_resName = name;
        
        m_nameIndex[ name ] = info;
        
	}

    // return it
    return hRes;    
}

template <typename DATA, typename HANDLE>
void ResourceMgr<DATA,HANDLE>::freeResource( HANDLE hRes )
{

    // FIXME: need to add another lookup-by-handle
    for ( typename ResourceHash::iterator ri = m_nameIndex.begin();
          ri != m_nameIndex.end(); ++ri )
    {
        if ((*ri).second.m_hRes == hRes)
        {
            ResInfo &info = (*ri).second;
            assert( info.m_refCount > 0 );            
            info.m_refCount--;            
        }        
    }
}

template <typename DATA, typename HANDLE>
void ResourceMgr<DATA,HANDLE>::unloadUnusedResources()
{

    for ( typename ResourceHash::iterator ri = m_nameIndex.begin();
          ri != m_nameIndex.end(); ++ri )
    {
        ResInfo &info = (*ri).second;

        DATA *data = info.m_hRes.deref();
        unloadResource( data );

        // TODO: remove from map
    }
}


template <typename DATA, typename HANDLE>
void ResourceMgr<DATA,HANDLE>::dbgPrintUsage()
{
    printf("========================================================\n" );
    printf(" Resource usage\n" );    
    printf("========================================================\n" );
    for ( typename ResourceHash::iterator ri = m_nameIndex.begin();
          ri != m_nameIndex.end(); ++ri )
    {
        ResInfo &info = (*ri).second;
        printf("%20s ... %d\n", info.m_resName.c_str(), info.m_refCount );        
    }
    printf("========================================================\n" );
}


#endif


