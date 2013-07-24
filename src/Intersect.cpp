#include "Intersect.hpp"

bool triangleRayIntersect(const glm::vec3& ta, const glm::vec3& tb, const glm::vec3& tc,
	const glm::vec3& ro, const glm::vec3& rd)
{
	// Get two edges of the triangle.
	glm::vec3 e1 = tb - ta;
	glm::vec3 e2 = tc - ta;

	glm::vec3 P = glm::cross(rd, e2);
	float det = glm::dot(e1, P);

	if(abs(det) < EPS) return false; //Ray lies in plane of triangle.

	float oneOverDet = 1.0f / det;

	glm::vec3 T = ro - ta;
	// Find barycentric co-ordinates u,v.
	float u = glm::dot(T, P) * oneOverDet;
	if(u < 0.0f || u > 1.0f) return false; // u out of range.

	glm::vec3 Q = glm::cross(T, e1);
	float v = glm::dot(rd, Q) * oneOverDet;
	if(v < 0.0f || u + v > 1.0f) return false; // v out of range.

	return true; // u, v in range - intersection.
}
