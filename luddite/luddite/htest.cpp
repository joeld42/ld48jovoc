#include <vector>
#include <string>

#include "handle.h"
#include "resource.h"

struct SomeThinggy
{
    SomeThinggy() : m_value(0) {}
    
    
    std::string m_name;
    int m_value;    
};

struct tag_THINGGY {};
typedef Handle<tag_THINGGY> HThinggy;

typedef HandleMgr<SomeThinggy, HThinggy> ThinggyMgr;

// ---------------------------------------------
bool loadResource( const char *filename, SomeThinggy *thing )
{
    printf("In loadResource for thinggy\n" );
    
    thing->m_name = "hoopla";
    thing->m_value = 99;    
}

void unloadResource( SomeThinggy *thing )
{
    printf("In unloadResource for thinggy\n" );
    printf("UNLOAD: %s\n", thing->m_name.c_str() );    
}

typedef ResourceMgr< SomeThinggy, HThinggy, 1024, 32 > ThinggyDatabase;

// ---------------------------------------------
int main()
{

    // Test out simple handle mgr
    ThinggyMgr thinggyMgr;
 
    HThinggy hFoo;
    SomeThinggy *myThinggy;
    
    myThinggy = thinggyMgr.acquire( hFoo );    
    printf("Got thinggy %zu id %d magic %d\n", 
           hFoo.getHandle(),
           hFoo.getIndex(), 
           hFoo.getMagic() );
    
    
    // test out resource mgr
    ThinggyDatabase db;

    hFoo = db.getResource( "foo.txt" );
    printf("Got thinggy from db %zu\n", hFoo.getHandle() );
    
    //db.freeResource( myThinggy );
    
}





