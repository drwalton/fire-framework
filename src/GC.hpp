#ifndef GC_HPP
#define GC_HPP

/* GC
 * Small namespace containing global constants used
 * throughout framework.
 */
namespace GC
{
	/* Phong Lighting */
	const int maxPhongLights = 50;

	/* SH Lighting */
	const int nSHBands = 2;
	const int nSHCoeffts = nSHBands * nSHBands;
	const int sqrtSHSamples = 50;
	const int maxSHLights = 10;
}

/* Useful mathematical constants */
const float PI = 3.141592653589793238462f;

#endif
