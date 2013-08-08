#include "Intersect.hpp"

bool triangleRayIntersect(
	const glm::vec3& ta, const glm::vec3& tb, const glm::vec3& tc,
	const glm::vec3& ro, const glm::vec3& rd)
{
	// Get two edges of the triangle.
	glm::vec3 e1 = tb - ta;
	glm::vec3 e2 = tc - ta;

	glm::vec3 norm = glm::cross(rd, e2);

	if(norm.length() < EPS) return false; //Triangle has no area.

	float det = glm::dot(e1, norm);

	if(det < EPS && det > -EPS) return false; //Ray lies in plane of triangle.

	float oneOverDet = 1.0f / det;

	glm::vec3 toTriangle = ro - ta;
	// Find barycentric co-ordinates u,v.
	float u = glm::dot(toTriangle, norm) * oneOverDet;
	if(u < 0.0f || u > 1.0f) return false; // u out of range.

	glm::vec3 acrossTriangle = glm::cross(toTriangle, e1);
	float v = glm::dot(rd, acrossTriangle) * oneOverDet;
	if(v < 0.0f || u + v > 1.0f) return false; // v out of range.

	float t = glm::dot(e2, acrossTriangle) * oneOverDet;

	if(t < EPS) return false; // intersection lies behind ro.

	return true; // u, v in range - intersection.
}

glm::vec3 getTriangleRayIntersection(
	const glm::vec3& ta, const glm::vec3& tb, const glm::vec3& tc,
	const glm::vec3& ro, const glm::vec3& rd)
{
	// Get two edges of the triangle.
	glm::vec3 e1 = tb - ta;
	glm::vec3 e2 = tc - ta;

	glm::vec3 norm = glm::cross(rd, e2);

	if(norm.length() < EPS) return falseVec; //Triangle has no area.

	float det = glm::dot(e1, norm);

	if(det < EPS && det > -EPS) return falseVec; //Ray lies in plane of triangle.

	float oneOverDet = 1.0f / det;

	glm::vec3 toTriangle = ro - ta;
	// Find barycentric co-ordinates u,v.
	float u = glm::dot(toTriangle, norm) * oneOverDet;
	if(u < 0.0f || u > 1.0f) return falseVec; // u out of range.

	glm::vec3 acrossTriangle = glm::cross(toTriangle, e1);
	float v = glm::dot(rd, acrossTriangle) * oneOverDet;
	if(v < 0.0f || u + v > 1.0f) return falseVec; // v out of range.

	float t = glm::dot(e2, acrossTriangle) * oneOverDet;

	if(t < EPS) return falseVec; // intersection lies behind ro.

	return glm::vec3(u, v, t); // u, v in range - intersection.
}

bool pointInTriangle(const glm::vec2& point, 
	const glm::vec2& ta, const glm::vec2& tb, const glm::vec2& tc,
	float& s, float& t)
{
	glm::vec2 side1 = tb - ta;
	glm::vec2 side2 = tc - ta;

	glm::vec2 triPoint = point - ta;

	s = glm::dot(side1, triPoint) / (side1.x*side1.x + side1.y*side1.y);
	t = glm::dot(side2, triPoint) / (side2.x*side2.x + side2.y*side2.y);

	if(0.0f <= s && s <= 1.0f && 0.0f <= t && t <= 1.0f && s + t <= 1.0f)
	{
		return true;
	}
	else
	{
		return false;
	}
}
