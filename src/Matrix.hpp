#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <exception>

class MatDimException : public std::exception {};

/* Matrix
 * Fairly bare-bones arbitrary size matrix class.
 * Used by SHMat.
 * N.B. : This matrix class follows the usual row-major convention,
 * differing from the column-major glm::mat* classes.
 */
template <typename T>
class Matrix
{
public:
	Matrix(unsigned rows, unsigned cols) : r(rows), c(cols) {zeroData();}
	Matrix(unsigned n) : r(n), c(n) {zeroData();}
	Matrix(unsigned n, T v);
	Matrix(const Matrix& other);
	~Matrix();

	const T& operator () (unsigned i, unsigned j) const;

	Matrix<T>& operator += (const Matrix<T>& m);
	Matrix<T> operator + (const Matrix<T>& m);
	Matrix<T> operator * (const Matrix<T>& m);
	std::vector<T> operator * (const std::vector<T>& v);

	const unsigned r, c;
private:
	T** data;
	void allocData();
	void zeroData();
};

template <typename T>
Matrix<T>::Matrix(unsigned n, T v)
	:r(n), c(n)
{
	allocData();
	for(unsigned i = 0; i < r; ++i)
		for(unsigned j = 0; j < c; ++j)
			data[i][j] = i == j ? v : 0;
}

template <typename T>
Matrix<T>::Matrix(const Matrix& other)
	:r(other.r), c(other.c)
{
	allocData();
	for(unsigned i = 0; i < r; ++i)
		for(unsigned j = 0; j < c; ++j)
			data[i][j] = other(i, j);
}

template <typename T>
Matrix<T>::~Matrix()
{
	for(unsigned i = 0; i < r; ++i)
		delete [] data[i];
	delete data;
}

template <typename T>
const T& Matrix<T>::operator () (unsigned i, unsigned j) const
{
	if(i >= r || j >= c) throw new MatDimException;
	return data[i][j];
}

template <typename T>
Matrix<T>& Matrix<T>::operator += (const Matrix& m)
{
	if(r != m.r || c != m.c) throw new MatDimException;
	for(unsigned i = 0; i < r; ++i)
		for(unsigned j = 0; j < c; ++j)
			data[i][j] += m.data[i][j];
	return *this;
}

template <typename T>
Matrix<T> Matrix<T>::operator + (const Matrix& m)
{
	Matrix temp(*this);
	return (temp += m);
}

template <typename T>
Matrix<T> Matrix<T>::operator * (const Matrix<T>& m)
{
	if(c != m.r) throw new MatDimException;
	Matrix ans(r, m.c);
	for(unsigned i = 0; i < ans.r; ++i)
		for(unsigned j = 0; j < ans.c; ++j)
			for(unsigned k = 0; k < c; ++k)
				ans.data[i][j] += data[i][k] * m.data[k][j];
	return ans;
}

template <typename T>
std::vector<T> Matrix<T>::operator * (const std::vector<T>& v)
{
	if(v.size() != c) throw new MatDimException;
	std::vector<T> ans(r, 0);

	for(unsigned i = 0; i < r; ++i)
		for(unsigned j = 0; j < c; ++j)
			ans[i] += data[i][j] * v[j];

	return ans;
}

template <typename T>
void Matrix<T>::allocData()
{
	data = new T*[r];
	for(unsigned i = 0; i < r; ++i)
		data[i] = new T[c];
}

template <typename T>
void Matrix<T>::zeroData()
{
	allocData();
	for(unsigned i = 0; i < r; ++i)
		for(unsigned j = 0; j < c; ++j)
			data[i][j] = 0;
}

#endif
