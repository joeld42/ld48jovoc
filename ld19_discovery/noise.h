#ifndef NOISE_H
#define NOISE_H

void initNoise();

// Perlin noise adapted from http://mrl.nyu.edu/~perlin/noise/
float pnoise( float x, float y, float z);

float pturb( float x, float y, float z, int octs, bool sgn );

float clamp( float val, float minV = 0.0, float maxV = 1.0 );

#endif


