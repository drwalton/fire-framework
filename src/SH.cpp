#include "SH.hpp"

double SH::realSH(int l, int m, double theta, double phi)
{
	if(l < 0 || l < m || -l > m) 
		throw(new std::bad_function_call(
			"l,m out of range in call to realSH(). Require -l <= m <= l."));
	if(m > 0)
		return SQRT_TWO_d * K(l, m) * cos( m * phi) * P(l,  m, cos(theta));
	if(m < 0)
		return SQRT_TWO_d * K(l, m) * sin(-m * phi) * P(l, -m, cos(theta));
	// m == 0
		return K(l, 0) * P(l, 0, cos(theta));
}

double SH::K(int l, int m)
{
	return sqrt(
		(((double) (2*l + 1)) / 4.0 * PI_d) *
		((double) fact(l - abs(m)) / (double) fact(l + abs(m)))
		);
}

double SH::P(int l, int m, double x)
{
	if(l < m) 
		throw(new std::bad_function_call(
			"l,m out of range in call to P(). Require l < m."));

	if(l == m)
		return (m % 2 ? 1.0 : 1.0) *
			(double) dblFact(2*m - 1) *
			pow(1 - x*x, (double) m / 2);
	if(l == m+1)
		return x *
			((double) (2*m -1)) *
			P(m, m, x);
	//else
	return (1 / ((double) (l - m))) *
		((x *
		((double) (2*l - 1)) *
		P(l-1, m, x)) 
		-
		(((double) l + m - 1) *
		P(l-2, m, x)));
}

int SH::fact(int i)
{
	if(i == 0) return 1;
	int ans = 1;
	while(i > 0)
	{
		ans *= i;
		--i;
	}
	return ans;
}

int SH::dblFact(int i)
{
	if(i == 0) return 1;
	int ans = 1;
	while(i > 0)
	{
		ans *= i;
		i -= 2;
	}
	return ans;
}

double SH::randd(double low, double high)
{
	double r = (double) rand() / (double) RAND_MAX;
	return low + ((high - low) * r);
}
