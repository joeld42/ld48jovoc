#include <tweakval.h>

void ReloadChangedTweakableValues()
{
    
}


#ifdef _TV_TEST_TWEAKVAL
#include <stdio.h>
#include <sys/time.h>
int main( int argc, char *argv[] )
{
    while (1)
    {
        int val = _TV( 5 );
        float fval = _TV( 3.1415 );
        
        printf("value is %d, valuef is %f\n", val, fval );
        sleep(1);
        
        // call this once per frame (or something)
        ReloadChangedTweakableValues();        
    }
    
}
#endif
