#include <glm.hpp>
#include <functional>
#include <vector>
#include <boost/math/special_functions/legendre.hpp>

namespace SH
{
	const double SQRT_TWO = 1.4142135623730950;
	const double PI = 3.1415926535897932;

	/* Finds the SH projection of func, which should be a function func(theta, phi) */
	std::vector<float> shProject(int sqrtNSamples, int nBands,
		std::function<double(double, double)>& func);

	/* Computes the real spherical harmonic SH_l^m(\theta, \phi) */
	double realSH(int l, int m, double theta, double phi);

	/* Computes the _normalised_ associated Legendre polynomial P_l^m at x*/
	double aLegendre(int l, int m, double x);

	double randd(double low, double high);
}