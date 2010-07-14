#include <stdlib.h>
#include <math.h>

float uniform()
{
    return (float)rand() / (float)RAND_MAX;    
}

float uniform( float minVal, float maxVal )
{
    return minVal + (uniform() * (maxVal-minVal));    
}

float normal()
{
    static float y1, y2;
    
    // This generates 2 numbers at a time, so 
    // alternate between generating a new one and
    // just returning the last one
    static bool doGen = true;
    if (!doGen)
    {
        // return the other number we generated
        // last time
        return y2;        
    }
    else
    {        
        float x1, x2, w;

        // reject points outside the unit circle
        do {
            x1 = (2.0f * uniform()) - 1.0;
            x2 = (2.0f * uniform()) - 1.0;
            w = x1*x1 + x2*x2;        
        } while (w >= 1.0 );
        
        w = sqrt( (-2.0 * logf(w)) / w );
        y1 = x1 * w;
        y2 = x2 * w;

        return y1;
    }
}

float normal( float mean, float stddev )
{
    return (normal() * stddev) + mean;
}



