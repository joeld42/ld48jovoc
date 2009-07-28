#include <stdio.h>
#include <stdlib.h>

#include "random.h"

int random( int minVal, int maxVal )
{
	return (int)(frandom() * (maxVal - minVal)) + minVal;
}

int random( int maxVal )
{
	return random( 0, maxVal );
}

float frandom()
{
	return (float)rand() / (float)RAND_MAX;
}