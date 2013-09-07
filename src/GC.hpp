#ifndef GC_HPP
#define GC_HPP

/* GC
 * Small namespace containing global constants used
 * throughout framework.
 */
namespace GC
{
	/* Phong Lighting */
	const int maxPhongLights = 200;
	const int maxMaterials = 4;

	/* SH Lighting */
	const int nSHBands = 5;
	const int nSHCoeffts = nSHBands * nSHBands;
	const int sqrtSHSamples = 30;
	const int nSHSamples = sqrtSHSamples * sqrtSHSamples;
	const int maxSHLights = 10;
	const int nSHBounces = 5;
	const bool jitterSamples = false;
	const int cubemapSize = 256;
	const int cubemapPixels = cubemapSize * cubemapSize;

	/* AO */
	const int sqrtAOSamples = 10;
	const int nAOSamples = sqrtAOSamples * sqrtAOSamples / 2;
}

/* Other frequently used constants
 *./

/* Used for testing if floats are 0 */
const float EPS = 10e-6f;

/* Useful mathematical constants */
const float PI = 3.141592653589793238462f;
const float SQRT_TWO = 1.4142135623730950f;

#endif
