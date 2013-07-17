#ifndef SHMAT_HPP
#define SHMAT_HPP

#include "Matrix.hpp"

/* SHMat
 * Stores SH rotation (block diagonal sparse) matrices.
 */
class SHMat
{
public:
	SHMat(const glm::mat3& rotation, int nBands) {init(rotation, nBands);}
	SHMat(const glm::mat4& rotation, int nBands);

	~SHMat() {};

	std::vector<float> operator * (const std::vector<float>& p);
private:
	std::vector<Matrix<float>> blocks;

	void init(const glm::mat3& rotation, int nBands);

	inline float del(int a, int b) { return a==b ? 1.0f : 0.0f; }
	inline float abs(int a) { return a >= 0 ? (float) a : (float) -a; }

	float M(int l, int m, int n, const Matrix<float>& R);
	float P(int i, int l, int m, int n, const Matrix<float>& R);
	float u(int l, int m, int n);
	float v(int l, int m, int n);
	float w(int l, int m, int n);
	float U(int l, int m, int n, const Matrix<float>& R);
	float V(int l, int m, int n, const Matrix<float>& R);
	float W(int l, int m, int n, const Matrix<float>& R);

};

#endif
