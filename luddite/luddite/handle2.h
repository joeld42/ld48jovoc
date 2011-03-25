#ifndef HANDLE_H
#define HANDLE_H

// based on scott bilas' handle class in
// http://scottbilas.com/publications/gem-resmgr/
// but with stuff added (refcounts)

#include <cassert>
#include <vector>

// Stupid "Handle" is a built in type for mac  :(

// ===============================================================
template <typename TAG>
class JBDHandle
{
    enum
    {
        // Sizes to use for bit fields
        MAX_BITS_INDEX = 16,
        MAX_BITS_MAGIC = 16,

        // sizes to compare aganst for asserting derefs
        MAX_INDEX = ( 1 << MAX_BITS_INDEX) - 1,
        MAX_MAGIC = ( 1 << MAX_BITS_MAGIC) - 1
    };
        

    union
    {
        struct
        {
            unsigned m_index : MAX_BITS_INDEX;
            unsigned m_magic : MAX_BITS_MAGIC;            
        };
        
        unsigned int m_handle;
    };
    
    
public:

    // Lifetime
    JBDHandle( void ) : m_handle( 0 ) {}
    void init( unsigned int index );
    
    // Queries
    unsigned int getIndex(void) const { return ( m_index ); }
    unsigned int getMagic(void) const { return ( m_magic ); }
    unsigned int getHandle(void) const { return ( m_handle ); }
    bool isNull(void) const { return ( !m_handle ); }

    operator unsigned int (void) const { return ( m_handle ); }
};

template <typename TAG>
void JBDHandle<TAG>::init( unsigned int index )
{
    assert( isNull() );
    assert( index <= MAX_INDEX );
    
    // generate a magic number
    static unsigned int s_autoMagic = 0;
    if ( ++s_autoMagic > MAX_MAGIC)
    {
        // 0 reserved for null handle
        s_autoMagic=1;        
    }
    
    m_index = index;
    m_magic = s_autoMagic;    
}

template <typename TAG>
inline bool operator != ( JBDHandle<TAG> a, JBDHandle<TAG> b )
{
    return ( a.getHandle() != b.getHandle() );    
}

template <typename TAG>
inline bool operator == ( JBDHandle<TAG> a, JBDHandle<TAG> b )
{
    return ( a.getHandle() == b.getHandle() );    
}

// ===============================================================
template <typename DATA, typename HANDLE>
class JBDHandleMgr
{
private:
    typedef std::vector<DATA>   UserVec;
    typedef std::vector<unsigned int> MagicVec;
    typedef std::vector<unsigned int> FreeVec;

    // private data
    UserVec   m_userData;
    MagicVec  m_magicNumbers;
    FreeVec   m_freeSlots;
    
public:

    // Lifetime
    JBDHandleMgr( void ) {}
    ~JBDHandleMgr( void ) {}
    
    // aquisition
    DATA *acquire( HANDLE &handle );
    void release( HANDLE  handle );

    // deref
    DATA *deref( HANDLE handle );
    const DATA *deref( HANDLE handle ) const;
    
    // other queries
    unsigned int getUsedHandleCount( void ) const 
    {
        return ( m_magicNumbers.size() - m_freeSlots.size() );
    }

    bool hasUsedHandles( void ) const
    {
        return ( !!getUsedHandleCount() );
    }
};

template <typename DATA, typename HANDLE>
DATA *JBDHandleMgr< DATA, HANDLE>::acquire( HANDLE &handle )
{
    // if free list is empty, add a new one otherwise use first 
    // one found
    unsigned int index;
    if (m_freeSlots.empty() )
    {
        // No free slots, grow the array
        index = m_magicNumbers.size();
        
        // Initialize the handle
        handle.init( index );
        m_userData.push_back( DATA() );
        m_magicNumbers.push_back( handle.getMagic() );
    }
    else
    {
        // Reuse a handle from the free list
        index = m_freeSlots.back();
        handle.init( index );
        m_freeSlots.pop_back();
        m_magicNumbers[ index ] = handle.getMagic();
    }
    
    return &(m_userData[index]);    
}



template <typename DATA, typename HANDLE>
void JBDHandleMgr<DATA, HANDLE>::release( HANDLE handle )
{
    // which one?
    unsigned int index = handle.getIndex();
    
    // make sure it's valid
    assert( index < m_userData.size() );
    assert( m_magicNumbers[ index ] == handle.getMagic() );
    
    // ok remove it
    m_magicNumbers[ index ] = 0;
    m_freeSlots.push_back( index );    
}

template <typename DATA, typename HANDLE >
inline DATA *JBDHandleMgr<DATA, HANDLE>::deref( HANDLE handle )
{
    if (handle.isNull() ) return NULL;
    
    // check handle validity -- this may be removed for
    // speed in final build
    unsigned int index = handle.getIndex();
    if ( (index >= m_userData.size() ) ||
         ( m_magicNumbers[ index ] != handle.getMagic() ) )
    {
        // No good! invalid handle
        assert( false );
        return NULL;        
    }

    return &(m_userData[index]);    
}

template <typename DATA, typename HANDLE>
inline const DATA *JBDHandleMgr<DATA, HANDLE>::deref( HANDLE handle ) const
{
    typedef JBDHandleMgr <DATA, HANDLE> ThisType;
    return ( const_cast<ThisType*>( this )->deref( handle ) );    
}



#endif


