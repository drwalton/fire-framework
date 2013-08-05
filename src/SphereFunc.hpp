#ifndef SPHEREFUNC_HPP
#define SPHEREFUNC_HPP

#include <glm.hpp>

/* A small selection of useful spherical functions.
 */

float pulse(float theta, float phi, glm::vec3 pulseDir, float width, float scale)
{
	glm::vec3 dir
		(
		sin(theta) * cos(phi),
		sin(theta) * sin(phi),
		cos(theta)
		); 

	float dot = glm::dot(pulseDir, dir);
	dot = dot > 0.0f ? dot : 0.0f;
	dot = pow(dot, width);
	dot *= scale;

	return dot;
};

float squareWave(float x, float interval, float xLow = -1.0f, float xHigh = 1.0f)
{
	if(x < 0) x = -x;
	while(x > interval) x -= interval;
	return x > (interval / 2) ? xLow : xHigh;
};

float swirls(float theta, float phi, float tightness = 0.5f)
{
	return squareWave((theta/PI) + (phi/(2*PI)), tightness);
};

float patches(float theta, float phi, float patchSize = 1.0f)
{
	if(theta < 0) theta = -theta;
	while(theta > patchSize * 2) theta -= patchSize * 2;
	bool t = theta > patchSize;

	if(phi < 0) phi = -phi;
	while(phi > patchSize * 2) phi -= patchSize * 2;
	bool p = phi > patchSize;

	return t ^ p ? 1.0f : -1.0f;
};
#endif
