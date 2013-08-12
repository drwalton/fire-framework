#ifndef SHMAT_HPP
#define SHMAT_HPP

#include "Matrix.hpp"
#include "GC.hpp"

/* SHMat
 * Stores SH rotation (block diagonal sparse) matrices.
 */
class SHMat
{
public:
	SHMat(int nBands);
	SHMat(const glm::mat3& rotation, int nBands) {init(rotation, nBands);}
	SHMat(const glm::mat4& rotation, int nBands);

	~SHMat() {};

	std::vector<float> operator * (const std::vector<float>& p);
	std::vector<glm::vec3> operator * (const std::vector<glm::vec3>& p);

	void print();
private:
	std::vector<Matrix<float>> blocks;

	void init(const glm::mat3& rotation, int nBands);

	static inline float del(int a, int b) 
		{ return a==b ? 1.0f : 0.0f; };
	static inline float abs(int a) 
		{ return a >= 0 ? static_cast<float>(a) : static_cast<float>(-a); };

	static float M(int l, int m, int n, const Matrix<float>& R);
	static float P(int i, int l, int m, int n, const Matrix<float>& R);
	static float u(int l, int m, int n);
	static float v(int l, int m, int n);
	static float w(int l, int m, int n);
	static float U(int l, int m, int n, const Matrix<float>& R);
	static float V(int l, int m, int n, const Matrix<float>& R);
	static float W(int l, int m, int n, const Matrix<float>& R);

};

#endif
