#include <stdio.h>
#include <stdlib.h>

#include <luddite/debug.h>

#include <luddite/handle2.h>
#include <luddite/resource2.h>
#include <luddite/singleton.h>

// ---------------------------------------
// A silly dummy resource to test with

struct Waffle
{
    std::string m_toppings;    
};

// typedefs to use the resource stuff
    
struct tag_WAFFLE;
typedef JBDHandle<tag_WAFFLE> HWaffle;


typedef JBDHandleMgr<Waffle, HWaffle> HWaffleMgr;

// ---------------------------------------
// Resource stuff

typedef ResourceMgr<Waffle, HWaffle> BaseWaffleDB;
class WaffleDB : public BaseWaffleDB, public Singleton<WaffleDB>
{
public:
    HWaffle getWaffle( const char *name );
    void freeWaffle( HWaffle hWaf );
};

HWaffle WaffleDB::getWaffle( const char *name )
{
    return BaseWaffleDB::getResource( name );
}

void WaffleDB::freeWaffle( HWaffle hw )
{
    return BaseWaffleDB::freeResource( hw );    
}


bool loadResource( const char *filename, Waffle *waffle )
{
    printf("Loaded waffle \"%s\".\n", filename );
    waffle->m_toppings = filename;    
    return true;    
}

bool unloadResource( Waffle *waffle )
{
    printf("Free waffle \"%s\".\n", waffle->m_toppings.c_str() );    
}

// ---------------------------------------
// Global resources
WaffleDB g_waffleDB;

// ---------------------------------------

int main( int argc, char *argv[] )
{
    DBG::info( "hello luddite\n" );

    WaffleDB &wdb = WaffleDB::singleton();
    
    HWaffle hw  = wdb.getWaffle( "pecan" );
    HWaffle hw2 = wdb.getWaffle( "blueberry" );
    HWaffle hw3 = wdb.getWaffle( "plain" );

    wdb.dbgPrintUsage();    

    // release some, get some more
    HWaffle hw4 = wdb.getWaffle( "blueberry" );
    wdb.freeWaffle( hw3 );

    wdb.dbgPrintUsage();    

    printf( "Unloading unused...\n" );
    wdb.unloadUnusedResources();
    
    wdb.dbgPrintUsage();    
}




