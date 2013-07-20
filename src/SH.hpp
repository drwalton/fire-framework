#ifndef SH_HPP
#define SH_HPP

#include <functional>
#include <vector>
#include <boost/math/special_functions/spherical_harmonic.hpp>
#include <glm.hpp>

namespace SH
{
	const double SQRT_TWO = 1.4142135623730950;
	const double PI = 3.1415926535897932;

	/* Finds the SH projection of func */
	/* where func evaluates to function of type: double func(double theta, double phi) */
	template<typename Fn>
	std::vector<glm::vec3> shProject(int sqrtNSamples, int nBands,
		Fn func);

	/* Computes the real spherical harmonic SH_l^m(\theta, \phi) */
	static double realSH(int l, int m, double theta, double phi);

	/* Computes the _normalised_ associated Legendre polynomial P_l^m at x*/
	static double aLegendre(int l, int m, double x);

	static double randd(double low, double high);
}

template<typename Fn>
std::vector<glm::vec3> SH::shProject(int sqrtNSamples, int nBands,
	Fn func)
{
	/* Initialise vector of coeffts with zeros */
	std::vector<glm::vec3> coeffts;
	for(int l = 0; l < nBands; ++l)
		for(int m = -l; m <= l; ++m)
			coeffts.push_back(glm::vec3(0.0f));

	/* Perform stratified random sampling over the sphere */
	double sqrWidth = 1 / (double) sqrtNSamples;
	double u, v, theta, phi;

	for(int i = 0; i < sqrtNSamples; ++i)
		for(int j = 0; j < sqrtNSamples; ++j)
		{
			/* Remove comments below for jittered sampling */
			u = (i * sqrWidth) + randd(0, sqrWidth);
			v = (j * sqrWidth) + randd(0, sqrWidth);
			theta = acos((2 * u) - 1);
			phi = 2 * SH::PI * v;
			for(int l = 0; l < nBands; ++l)
				for(int m = -l; m <= l; ++m)
				{
					coeffts[l*(l+1) + m] += func(theta, phi) * float(realSH(l, m, theta, phi));
				}
		}

	/* Normalize coefficients */
	double nSamples = sqrtNSamples * sqrtNSamples;
	for(std::vector<glm::vec3>::iterator i = coeffts.begin(); i != coeffts.end(); ++i)
	{
		(*i) *= 4.0 * PI / nSamples;
	}

	return coeffts;
}

double SH::realSH(int l, int m, double theta, double phi)
{
	//Check values of l, m are sensible,
	if(l < 0 || l < m || -l > m) 
		throw(new std::bad_function_call("l,m out of range. Require -l <= m <= l."));
	if(m > 0) 
		return SQRT_TWO * boost::math::spherical_harmonic_r(l, m, theta, phi);
	else if(m < 0)
		return SQRT_TWO * boost::math::spherical_harmonic_i(l, -m, theta, phi);
	else // m == 0
		return boost::math::spherical_harmonic_r(l, m, theta, phi);
}

double SH::aLegendre(int l, int m, double x)
{
	return boost::math::legendre_p(l, m, x);
}

double SH::randd(double low, double high)
{
	double r = (double) rand() / (double) RAND_MAX;
	return low + ((high - low) * r);
}

#endif
