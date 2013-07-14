#include "SHMat.hpp"

SHMat::SHMat(const glm::mat4& rotation, int nBands)
{
	glm::mat3 subMat
		(
		rotation[0][0], rotation[0][1], rotation[0][2],
		rotation[1][0], rotation[1][1], rotation[1][2],
		rotation[2][0], rotation[2][1], rotation[2][2]
		);
	init(subMat, nBands);
}

std::vector<float> SHMat::operator * (const std::vector<float>& p)
{
	if(p.size() != ((blocks.size() + 1) * (blocks.size() + 1))) 
		throw new MatDimException;

	std::vector<float> ans;

	for(int i = 0; i < blocks.size(); ++i)
	{
		std::vector<float> subVec(p.begin() + (i*i), p.begin() + (i+1)*(i+1));
		std::vector<float> subProd = blocks[i] * subVec;

		for(std::vector<float>::iterator i = subProd.begin(); i != subProd.end(); ++i)
			ans.push_back(*i);
	}

	return ans;
}

