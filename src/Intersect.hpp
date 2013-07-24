#ifndef INTERSECT_HPP
#define INTERSECT_HPP

#include <glm.hpp>

namespace
{
	const float EPS = 10e-6f;
}

/* M\"{o}ller-Trumbore triangle-ray intersection test. */
/* Code copied from paper, ported to C++ */
bool triangleRayIntersect(const glm::vec3& ta, const glm::vec3& tb, const glm::vec3& tc,
	const glm::vec3& ro, const glm::vec3& rd);

#endif
