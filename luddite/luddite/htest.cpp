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
    return true;    
}

void unloadResource( SomeThinggy *thing )
{
    printf("unloadResource::Thing is %p\n", thing );
    
    printf("In unloadResource for thinggy\n" );
    printf("UNLOAD: %s\n", thing->m_name.c_str() );    
}

typedef ResourceMgr< SomeThinggy, HThinggy, 1024, 32 > ThinggyDatabase;

// ---------------------------------------------
int main()
{

    // Test out simple handle mgr
    ThinggyMgr thinggyMgr;
 
    for (int i=0; i < 10; i++)
    {
        HThinggy hFoo;
        SomeThinggy *myThinggy;

        myThinggy = thinggyMgr.acquire( hFoo );    
        printf("%d Got thinggy %zu id %d magic %d\n", i,
               hFoo.getHandle(),
               hFoo.getIndex(), 
               hFoo.getMagic() );
    }
    
    
    
    // test out resource mgr
    printf(" ---------- resmgr ------------\n" );
    
    HThinggy hFoo2, hFoo3, hFoo4;    
    ThinggyDatabase db;

    hFoo2 = db.getResource( "foo.txt" );
    printf("Got thinggy from db %zu\n", hFoo2.getHandle() );

    hFoo3 = db.getResource( "bar.txt" );
    printf("Got thinggy bar from db %zu\n", hFoo3.getHandle() );

    // This gets a duplicate of foo.txt so it shouldn't load again
    hFoo4 = db.getResource( "foo.txt" );
    printf("Got thinggy from db %zu\n", hFoo4.getHandle() );

    // Ok, now we're done with them
    db.freeResource( hFoo2 );
    db.freeResource( hFoo3 );
    db.freeResource( hFoo4 );

    // Okay now reload
    HThinggy hBar;    
    hBar = db.getResource( "bar.txt" );
    printf("Got thinggy bar from db %zu\n", hFoo3.getHandle() );
    db.freeResource( hBar );

    printf(" ---------- done ------------\n" );    
}






