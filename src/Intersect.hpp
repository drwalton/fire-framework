#ifndef INTERSECT_HPP
#define INTERSECT_HPP

#include <glm.hpp>
#include "GC.hpp"

/* M\"{o}ller-Trumbore triangle-ray intersection test. */
/* Code copied from paper, ported to C++ */
bool triangleRayIntersect(const glm::vec3& ta, const glm::vec3& tb, const glm::vec3& tc,
	const glm::vec3& ro, const glm::vec3& rd);

#endif
