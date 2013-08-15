#ifndef SPHEREFUNC_HPP
#define SPHEREFUNC_HPP

#include <glm.hpp>

/* A small selection of useful spherical functions.
 */

float pulse(float theta, float phi, glm::vec3 pulseDir, float width, float scale);

float squareWave(float x, float interval, float xLow = -1.0f, float xHigh = 1.0f);

float swirls(float theta, float phi, float tightness = 0.5f);

float patches(float theta, float phi, float patchSize = 1.0f);
#endif
