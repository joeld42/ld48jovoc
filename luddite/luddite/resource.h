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
            return ( ::strcasecmp( a.c_str(), b.c_str() ) < 0 );            
        }

        // hash operator
        size_t operator()( const std::string &a ) const
        {
            return ns_ext::hash<const char*>( a.c_str() );            
        }        
    };
    
    typedef ns_ext::hash_map<std::string,HANDLE, ResourceHashTraits> ResourceHash;
    //typedef std::pair< ResourceHash::iterator, bool> NameIndexInsertRc;    

    HMgrType     m_resMgr;    
    ResourceHash m_nameIndex;
    
public:
    ResourceMgr() {}
    ~ResourceMgr();
    
    // Gets a resource, loading it if needed
    HANDLE getResource( const char *name );
    void   release( HANDLE );
};

template <typename DATA, typename HANDLE, int BUCKET_SZ, int MIN_BUCKETS>
ResourceMgr<DATA,HANDLE,BUCKET_SZ,MIN_BUCKETS>::~ResourceMgr()
{
    // Release all of our resources
    for ( typename ResourceHash::iterator ri = m_nameIndex.begin();
          ri != m_nameIndex.end(); ++ri )
    {
        // TODO: delete
    }    
}



template <typename DATA, typename HANDLE, int BUCKET_SZ, int MIN_BUCKETS> 
HANDLE ResourceMgr<DATA,HANDLE,BUCKET_SZ,MIN_BUCKETS>::getResource( const char *name )
{
    std::pair<typename ResourceHash::iterator, bool> rc = m_nameIndex.insert(
        std::make_pair( name, HANDLE() ) );
    if (rc.second)
    {
        DATA *data = m_resMgr.aquire( rc.first->second );
        printf("TODO: load %s\n", rc.first->first );
        // TODO: error check
    }

    return rc.first->second;    
}



#endif


