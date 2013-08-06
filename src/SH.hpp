#ifndef SH_HPP
#define SH_HPP

#include <vector>
#include <string>

#include <glm.hpp>

#include "GC.hpp"

namespace SH
{
	/* Finds the SH projection of func 
	 * where func evaluates to some function
	 * of type: double func(double theta, double phi) 
	 */
	template<typename Fn>
	std::vector<glm::vec3> shProject(int sqrtNSamples, int nBands,
		Fn func);

	glm::vec3 evaluate(std::vector<glm::vec3> projection,
		float theta, float phi);

	/* Computes the real spherical harmonic SH_l^m(\theta, \phi) */
	double realSH(int l, int m, double theta, double phi);

	double K(int l, int m);
	double P(int l, int m, double x);
	int fact(int i);
	int dblFact(int i);

	inline int SHI(int l, int m) 
	{
		if(l == 0) return 0;
		else return (l+1)*l + m;
	}
}

double randd(double low, double high);

class BadArgumentException
{
public:
	BadArgumentException(const std::string& _what) :what(_what) {};
	std::string what;
};

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
			u = (i * sqrWidth);
			v = (j * sqrWidth);
			if(GC::jitterSamples)
			{
				u += randd(0, sqrWidth);
				v += randd(0, sqrWidth);
			}
			theta = acos((2 * u) - 1);
			phi = 2 * PI_d * v;
			for(int l = 0; l < nBands; ++l)
				for(int m = -l; m <= l; ++m)
				{
					glm::vec3 val = func(theta, phi);
					/* Do not calculate SH if unnecessary (val is 0) */
					if(abs(val.x) < EPS && 
					   abs(val.y) < EPS && 
					   abs(val.z) < EPS) continue;
					coeffts[l*(l+1) + m] += 
						val * glm::vec3(realSH(l, m, theta, phi));
				}
		}

	/* Normalize coefficients */
	double nSamples = sqrtNSamples * sqrtNSamples;
	for(auto i = coeffts.begin(); i != coeffts.end(); ++i)
	{
		(*i) *= 4.0 * PI / nSamples;
	}

	return coeffts;
}


#endif
