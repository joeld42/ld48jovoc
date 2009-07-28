#ifndef FOCUS_H
#define FOCUS_H

// global configuration

// 30 ticks per sim frame
#define STEPTIME (33)

// useful funcs
int irand( int min, int max );
float frand( float min, float max );

float clamp( float n );
int iclamp( int n );

#define min3(a, b, c) ((a)<(b) ? ((a)<(c) ? (a) : (c)) : ((b)<(c) ? (b) : (c)))
#define max3(a, b, c) ((a)>(b) ? ((a)>(c) ? (a) : (c)) : ((b)>(c) ? (b) : (c)))

#define lerp(a, b, t) ((t)*(a) + (1.0-t)*(b))


#endif