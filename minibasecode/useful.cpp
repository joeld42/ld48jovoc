#include <stdlib.h>
#include <math.h>
#include <prmath/prmath.hpp>
#include <useful.h>

float randUniform()
{
    return (float)rand() / (float)RAND_MAX;    
}

float randUniform( float minVal, float maxVal )
{
    return minVal + (randUniform() * (maxVal-minVal));    
}

float randNormal()
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
            x1 = (2.0f * randUniform()) - 1.0;
            x2 = (2.0f * randUniform()) - 1.0;
            w = x1*x1 + x2*x2;        
        } while (w >= 1.0 );
        
        w = sqrt( (-2.0 * logf(w)) / w );
        y1 = x1 * w;
        y2 = x2 * w;

        return y1;
    }
}

float randNormal( float mean, float stddev )
{
    return (randNormal() * stddev) + mean;
}

// From http://www.opengl.org/wiki/GluPerspective_code

void glhFrustumf2(matrix4x4f &matrix, 
                  float left, float right, float bottom, float top,
                  float znear, float zfar)
{
    float temp, temp2, temp3, temp4;
    temp = 2.0 * znear;
    temp2 = right - left;
    temp3 = top - bottom;
    temp4 = zfar - znear;
    matrix[0][0] = temp / temp2;
    matrix[0][1] = 0.0;
    matrix[0][2] = 0.0;
    matrix[0][3] = 0.0;
    matrix[1][0] = 0.0;
    matrix[1][1] = temp / temp3;
    matrix[1][2] = 0.0;
    matrix[1][3] = 0.0;
    matrix[2][0] = (right + left) / temp2;
    matrix[2][1] = (top + bottom) / temp3;
    matrix[2][2] = (-zfar - znear) / temp4;
    matrix[2][3] = -1.0;
    matrix[3][0] = 0.0;
    matrix[3][1] = 0.0;
    matrix[3][2] = (-temp * zfar) / temp4;
    matrix[3][3] = 0.0;
}

//matrix will receive the calculated perspective matrix.
//You would have to upload to your shader
// or use glLoadMatrixf if you aren't using shaders.
void glhPerspectivef2(matrix4x4f &matrix, float fovyInDegrees, float aspectRatio,
                      float znear, float zfar)
{
    float ymax, xmax;
    float temp, temp2, temp3, temp4;
    ymax = znear * tanf(fovyInDegrees * M_PI / 360.0);
    //ymin = -ymax;
    //xmin = -ymax * aspectRatio;
    
    xmax = ymax * aspectRatio;
    glhFrustumf2(matrix, -xmax, xmax, -ymax, ymax, znear, zfar);
}


