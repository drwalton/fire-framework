#ifndef INTERSECT_HPP
#define INTERSECT_HPP

#include <glm.hpp>
#include "GC.hpp"

namespace
{
	const glm::vec3 falseVec(-1.0f, -1.0f, -1.0f);
}

/* M\"{o}ller-Trumbore triangle-ray intersection test. */
/* Code copied from paper, ported to C++ */
bool triangleRayIntersect(
	const glm::vec3& ta, const glm::vec3& tb, const glm::vec3& tc,
	const glm::vec3& ro, const glm::vec3& rd);

/* Similar to above, but returns intersection in the form (u, v, t) 
 * Where u, v are barycentric co-ordinates on the triangle and 
 * t is the distance of the intersection along the ray.
 * N.B. Assumes rd, the ray direction, is normalised.
 *      If not, the value of t will be incorrect.
 */
glm::vec3 getTriangleRayIntersection(
	const glm::vec3& ta, const glm::vec3& tb, const glm::vec3& tc,
	const glm::vec3& ro, const glm::vec3& rd);

bool pointInTriangle(const glm::vec2& point, 
	const glm::vec2& ta, const glm::vec2& tb, const glm::vec2& tc,
	float& s, float& t);

#endif
