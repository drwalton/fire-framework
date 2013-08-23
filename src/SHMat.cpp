#include "SHMat.hpp"

#include <iostream>

SHMat::SHMat(int nBands)
{
	// Push nBands identity matrices into blocks.
	for(int b = 0; b < nBands; ++b)
	{
		blocks.push_back(Matrix<float>(2*b + 1, 1.0f));
	}
}

SHMat::SHMat(const glm::mat4& rotation, int nBands)
{
	glm::mat3 subMat(rotation);
	init(subMat, nBands);
}

std::vector<float> SHMat::operator * (const std::vector<float>& p)
{
	if(p.size() != blocks.size()*blocks.size()) 
		throw new MatDimException;

	std::vector<float> ans;

	for(size_t i = 0; i < blocks.size(); ++i)
	{
		std::vector<float> subVec(p.begin() + (i*i), p.begin() + (i+1)*(i+1));
		std::vector<float> subProd = blocks[i] * subVec;

		for(auto i = subProd.begin(); i != subProd.end(); ++i)
			ans.push_back(*i);
	}

	return ans;
}

std::vector<glm::vec3> SHMat::operator * (const std::vector<glm::vec3>& p)
{
	if(p.size() != blocks.size()*blocks.size()) 
		throw new MatDimException;

	std::vector<glm::vec3> ans;

	for(size_t i = 0; i < blocks.size(); ++i)
	{
		std::vector<glm::vec3> subVec(p.begin() + (i*i), p.begin() + (i+1)*(i+1));
		std::vector<glm::vec3> subProd = blocks[i] * subVec;

		for(auto i = subProd.begin(); i != subProd.end(); ++i)
			ans.push_back(*i);
	}

	return ans;
}

void SHMat::print()
{
	for(auto i = blocks.begin(); i != blocks.end(); ++i)
		i->print();
}

void SHMat::init(const glm::mat3& rotation, int nBands)
{
	Matrix<float> R_o(rotation);
	/* Rearrange coeffts of R */
	Matrix<float> R(3,3);
	R(0,0) = R_o(1,1);
	R(0,1) = -R_o(1,2);
	R(0,2) = R_o(1,0);
	R(1,0) = -R_o(2,1);
	R(1,1) = R_o(2,2);
	R(1,2) = -R_o(2,0);
	R(2,0) = R_o(0,1);
	R(2,1) = -R_o(0,2);
	R(2,2) = R_o(0,0);

	blocks.reserve(nBands);

	blocks.push_back(Matrix<float>(1, 1.0f));
	blocks.push_back(R);

	for(int l = 2; l < nBands; ++l)
	{
		Matrix<float> mat(2*l + 1, 2*l + 1);

		for(int m = -l; m <= l; ++m)
			for(int n = -l; n <= l; ++n)
				mat.i(m,n) = M(l,m,n,R);

		blocks.push_back(mat);
	}
}

float SHMat::M(int l, int m, int n, const Matrix<float>& R)
{
	if(m > l || n > l || -m > l || -n > l) throw(new MatDimException);

	if(l == 0) return 1.0f;

	if(l == 1) return R.i(m,n);

	float _u = u(l,m,n);
	float _v = v(l,m,n);
	float _w = w(l,m,n);

	if(_u > EPS || _u < -EPS) _u *= U(l,m,n,R);
	if(_v > EPS || _v < -EPS) _v *= V(l,m,n,R);
	if(_w > EPS || _w < -EPS) _w *= W(l,m,n,R);

	return _u + _v + _w;
}

float SHMat::P(int i, int l, int m, int n, const Matrix<float>& R)
{
	if(n == l)
		return R.i(i,1) * M(l-1, m, l-1, R) - R.i(i,-1) * M(l-1, m, -l+1, R);
	if(n == -l)
		return R.i(i,1) * M(l-1, m, -l+1, R) + R.i(i,-1) * M(l-1, m, l-1, R);
	else
		return R.i(i,0) * M(l-1, m, n, R);
}

float SHMat::u(int l, int m, int n)
{
	float num = static_cast<float>((l + m) * (l - m));
	float den;
	if(n == l || n == -l)
		den = static_cast<float>((2*l) * (2*l - 1));
	else 
		den = static_cast<float>((l + n) * (l - n));
	return sqrt(num / den);
}

float SHMat::v(int l, int m, int n)
{
	float num = (1.0f + del(m,0)) * 
		(static_cast<float>(l) + abs(m) - 1.0f) * 
		(static_cast<float>(l) + abs(m));
	float den;

	if(n == l || n == -l)
		den = static_cast<float>((2*l) * (2*l - 1));
	else
		den = static_cast<float>((l + n) * (l - n));

	return 0.5f * sqrt(num / den) * (1.0f - 2.0f*del(m,0));
}

float SHMat::w(int l, int m, int n)
{
	float num = static_cast<float>(
		(static_cast<float>(l) - abs(m) - 1.0f) * 
		(static_cast<float>(l) - abs(m)));
	float den;

	if(n == l || n == -l)
		den = static_cast<float>((2*l) * (2*l - 1));
	else
		den = static_cast<float>((l + n) * (l - n));

	return -0.5f * sqrt(num / den) * (1.0f - del(m,0));
}

float SHMat::U(int l, int m, int n, const Matrix<float>& R)
{
	return P(0, l, m, n, R);
}

float SHMat::V(int l, int m, int n, const Matrix<float>& R)
{
	if(m == 0)
		return P(1, l, 1, n, R) + P(-1, l, -1, n, R);
	else if(m > 0)
		return (P(1, l, m-1, n, R) * sqrt(1.0f + del(m,1))) - 
			(P(-1, l, (-m)+1, n, R) * (1.0f - del(m,1)));
	else //m < 0
		return P(1, l, m+1, n, R) * (1.0f - del(m,-1)) + 
			P(-1, l, (-m)-1, n, R) * sqrt(1.0f + del(m,-1)); 
}

float SHMat::W(int l, int m, int n, const Matrix<float>& R)
{
	/* Shouldn't be called with m == 0 */
	if(m == 0)
		throw(new MatDimException);
	else if(m > 0)
		return P(1, l, m+1, n, R) + P(-1, l, -m-1, n, R);
	else //m < 0
		return P(1, l, m-1, n, R) - P(-1, l, -m+1, n, R);
}
