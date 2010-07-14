#ifndef RANDOM_H
#define RANDOM_H

// Generates a uniform random number between 0 and 1
float uniform();

// Generates a uniform random number between minVal and maxVal
float uniform( float minVal, float maxVal );

// Generates a random number from a normal (gaussian) distribution
// with a mean of 0 and a std deviation of 1
// (Box-Muller transform)
// NOTE: not threadsafe
float normal();

// Generates a random number from a normal distribution
// with the given mean and stddev
//
// NOTE: not threadsafe
float normal( float mean, float stddev );

#endif

