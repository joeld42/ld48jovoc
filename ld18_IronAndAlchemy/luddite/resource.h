#ifndef RESOURCE_H
#define RESOURCE_H

#ifdef __GNUC__
# include <ext/hash_map>
namespace ns_ext = __gnu_cxx;
#else
# include <hash_map>
namespace ns_ext = stdext;
#endif

#include <string>

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
// BUCKET_SZ, MIN_BUCKETS -- for the internal std_ext::hash_map. Use
// with appropriate size for expected population.
//
// To use this class, you only need to implement the loadResource and
// unloadResource functions above. You may also want to subclass it to
// provide accessors to the internal data from the handle.
//
template <typename DATA, typename HANDLE, int BUCKET_SZ, int MIN_BUCKETS> 
class ResourceMgr
{

private:
    typedef HandleMgr<DATA,HANDLE> HMgrType;
    
    struct ResourceHashTraits
    {
        static const size_t bucket_size = BUCKET_SZ;
        static const size_t min_buckets = MIN_BUCKETS;

        // comparison operator
        bool operator()( const std::string &a, const std::string &b ) const
        {
            return ( ::stricmp( a.c_str(), b.c_str() ) < 0 );            
        }

        // hash operator
        size_t operator()( const std::string &a ) const
        {
#ifndef WIN32
            return ns_ext::hash<const char*>()( a.c_str() );            
#else
			//blah
			size_t h = 0;
			std::string::const_iterator p, p_end;
			for(p = a.begin(), p_end = a.end(); p != p_end; ++p)
			{
				h = 31 * h + (*p);
			}
			return h;
#endif
        }        
    };
     
   typedef ns_ext::hash_map<std::string,HANDLE, ResourceHashTraits> ResourceHash;

protected:
    HMgrType     m_resMgr;    
    ResourceHash m_nameIndex;
    
public:
    ResourceMgr() {}
    ~ResourceMgr();
    
    // Gets a resource, loading it if needed
    HANDLE getResource( const char *name );
    void   freeResource( HANDLE );
};

template <typename DATA, typename HANDLE, int BUCKET_SZ, int MIN_BUCKETS>
ResourceMgr<DATA,HANDLE,BUCKET_SZ,MIN_BUCKETS>::~ResourceMgr()
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

template <typename DATA, typename HANDLE, int BUCKET_SZ, int MIN_BUCKETS> 
HANDLE ResourceMgr<DATA,HANDLE,BUCKET_SZ,MIN_BUCKETS>::getResource( const char *name )
{
    HANDLE hRes;
    std::pair<typename ResourceHash::iterator, bool> rc = m_nameIndex.insert(
        std::make_pair( name, HANDLE() ) );

    // If the handle is already in the map, retreive it
    if (!rc.second)
    {
        // Get the handle
        hRes = rc.first->second;        
    }
    
    // if the handle is NULL, either because this is something new or 
    // previously deleted, aquire it's resource
    if (hRes.isNull())
    {    
        DATA *data = m_resMgr.acquire( hRes );

        // Load the resource
        if (!loadResource( rc.first->first.c_str(), data ))
        {
            // todo: error check
            DBG::warn("failed to load resource %s", rc.first->first.c_str() );
            return HANDLE();            
        }
    }

    // increment the ownership for this 
    m_resMgr.addrefCount( hRes );    
    
    // return it
    return hRes;    
}

template <typename DATA, typename HANDLE, int BUCKET_SZ, int MIN_BUCKETS> 
void ResourceMgr<DATA,HANDLE,BUCKET_SZ,MIN_BUCKETS>::freeResource( HANDLE hRes )
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



#endif


