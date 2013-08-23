#ifndef SPHEREFUNC_HPP
#define SPHEREFUNC_HPP

#include <glm.hpp>

/* A small selection of useful spherical functions.
 */

// A pulse in the direction dir. Higher values of width actually 
// decrease the width of the pulse.
float pulse(float theta, float phi, glm::vec3 pulseDir, float width, float scale);

float circle(float theta, float phi, glm::vec3 centerDir, float angle);

// A simple 1D square wave function used to generate other functions.
float squareWave(float x, float interval, float xLow = -1.0f, float xHigh = 1.0f);

// A positive/negative swirl down the sphere
float swirls(float theta, float phi, float tightness = 0.5f);

// Square patches of positive/negative values across the sphere.
float patches(float theta, float phi, float patchSize = 1.0f);
#endif
