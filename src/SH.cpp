#include "SH.hpp"

std::vector<float> SH::shProject(int sqrtNSamples, int nBands,
	std::function<double(double, double)>& func)
{
	/* Initialise vector of coeffts with zeros */
	std::vector<double> coeffts;
	for(int l = 0; l < nBands; ++l)
		for(int m = -l; m <= l; ++m)
			coeffts.push_back(0);

	/* Perform stratified random sampling over the sphere */
	double sqrWidth = 1 / (double) sqrtNSamples;
	double u, v, theta, phi, coefft;

	for(int i = 0; i < sqrtNSamples; ++i)
		for(int j = 0; j < sqrtNSamples; ++j)
		{
			u = (i * sqrWidth) + randd(0, sqrWidth);
			v = (j * sqrWidth) + randd(0, sqrWidth);
			theta = 2 * SH::PI * u;
			phi = acos((2 * v) - 1);
			for(int l = 0; l < nBands; ++l)
				for(int m = -l; m <= l; ++m)
					coeffts[l*(l+1) + m] += func(theta, phi) * realSH(l, m, theta, phi);
		}

	/* Normalize coefficients */
	double nSamples = sqrtNSamples * sqrtNSamples;
	for(std::vector<double>::iterator i = coeffts.begin(); i != coeffts.end(); ++i)
	{
		(*i) *= 4.0 * PI / nSamples;
	}

	/* Cast coefficients to floats */
	std::vector<float> coeffts_f;
	for(std::vector<double>::iterator i = coeffts.begin(); i != coeffts.end(); ++i)
	{
		coeffts_f.push_back((float) (*i));
	}

	return coeffts_f;
}

double SH::realSH(int l, int m, double theta, double phi)
{
	//Check values of l, m are sensible,
	if(m < 0 || l > m || -l > m) return; //TODO: exception
	if(m > 0) 
		return SQRT_TWO * AssLegendre(l,  m, cos(theta)) * cos( m * phi);
	else if(m < 0)
		return SQRT_TWO * AssLegendre(l, -m, cos(theta)) * sin(-m * phi);
	else // m == 0
		return AssLegendre(l, m, cos(theta));
}

double SH::aLegendre(int l, int m, double x)
{
	return gsl_sf_legendre_sphPlm(l, m, x);
}

double SH:randd(double low, double high)
{
	double r = (double) rand() / (double) RAND_MAX;
	return low + ((high - low) * r);
}