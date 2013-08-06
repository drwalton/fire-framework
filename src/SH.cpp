#include "SH.hpp"

glm::vec3 SH::evaluate(std::vector<glm::vec3> projection,
	float theta, float phi)
{
	glm::vec3 value(0.0f);

	for(unsigned i = 0; i < projection.size(); ++i)
	{
		int l = static_cast<int>(sqrt(static_cast<float>(i)));
		int m = i - l*(l+1);
		value += projection[i] * (float) SH::realSH(l, m, theta, phi);
	}

	return value;
}

float SH::realSH(int l, int m, float theta, float phi)
{
	if(l < 0 || l < m || -l > m) 
		throw(new BadArgumentException(
			"l,m out of range in call to realSH(). Require -l <= m <= l."));
	if(m > 0)
		return SQRT_TWO * K(l, m) * cos( m * phi) * P(l,  m, cos(theta));
	if(m < 0)
		return SQRT_TWO * K(l, m) * sin(-m * phi) * P(l, -m, cos(theta));
	// m == 0
		return K(l, 0) * P(l, 0, cos(theta));
}

float SH::K(int l, int m)
{
	return sqrt(
		(((float) (2*l + 1)) / (4.0f * PI)) *
		((float) fact(l - abs(m)) / (float) fact(l + abs(m)))
		);
}

float SH::P(int l, int m, float x)
{
	if(l < m) 
		throw(new BadArgumentException(
			"l,m out of range in call to P(). Require l < m."));

	if(l == m)
		return (m % 2 ? -1.0f : 1.0f) *
			(float) dblFact(2*m - 1) *
			pow((float) 1.0f - x*x, (float) m / 2);
	if(l == m+1)
		return x *
			((float) (2*m + 1)) *
			P(m, m, x);
	//else
	return (1 / ((float) (l - m))) *
		((x *
		((float) (2*l - 1)) *
		P(l-1, m, x)) 
		-
		(((float) l + m - 1) *
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

float randf(float low, float high)
{
	float r = (float) rand() / (float) RAND_MAX;
	return low + ((high - low) * r);
}
