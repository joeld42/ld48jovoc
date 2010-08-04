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

template <typename DATA, typename HANDLE, int BUCKET_SZ, int MIN_BUCKETS> 
class ResourceMgr< DATA, HANDLE>
{
private:
    typedef HandleMgr<DATA,HANDLE> HMgrType;
    
    struct ResourceHashTraits<HANDLE>
    {
        static const size_t bucket_size = BUCKET_SZ;
        static const size_t min_buckets = MIN_BUCKETS;

        // comparison operator
        bool operator( const std::string &a, const std::string &b ) const
        {
            return ( ::stricmp( a.c_str(), b.c_str() ) < 0 );            
        }

        // hash operator
        size_t operator()( const std::string &a ) const
        {
            return std::hash<const char*>( a.c_str() );            
        }        
    };
    
    typedef ns_ext::hash_map<std::string,HANDLE, ResourceHashTraits> ResourceHash;
    typedef std::pair< ResourceHash::iterator, bool> NameIndexInsertRc;
    
    ResourceHash m_nameIndex;
    
public:
    ResourceMgr() {}
    ~ResourceMgr();
    
    // Gets a resource, loading it if needed
    HANDLE getResource( const char *name );
    void   release( HANDLE );
    
    

};

template <typename DATA, typename HANDLE, int BUCKET_SZ, int MIN_BUCKETS> 
ResourceMgr<DATA,HANDLE,BUCKET_SZ,MIN_BUCKETS>::~ResourceMgr
{
    // Release all of our resources
    
}


template <typename DATA, typename HANDLE, int BUCKET_SZ, int MIN_BUCKETS> 
HANDLE ResourceMgr<DATA,HANDLE,BUCKET_SZ,MIN_BUCKETS>::getResource( const char *name )
{
    
}



#endif


