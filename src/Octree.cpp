#include "Octree.hpp"

namespace
{
	void swap(float& a, float& b)
	{
		float temp = a;
		a = b;
		b = temp;
	}
}

bool AABB::intersect(const AABB& other)
{
	return (
		corner.x <= other.corner.x + other.dimensions.x &&
		corner.y <= other.corner.y + other.dimensions.y &&
		corner.z <= other.corner.z + other.dimensions.z &&
		other.corner.x <= corner.x + dimensions.x &&
		other.corner.y <= corner.y + dimensions.y &&
		other.corner.z <= corner.z + dimensions.z);
}

/* Algorithm due to Kay and Kajiya */
bool AABB::intersect(glm::vec3 orig, glm::vec3 dir)
{
	float tNear = FLT_MIN;
	float tFar  = FLT_MAX;

	for(int i = 0; i < 3; ++i) //For each axis
	{
		if(abs(dir[i]) < EPS) //Ray dir lies in plane perp to this axis.
			if(!(orig[i] >= corner[i] && orig[i] <= corner[i] + dimensions[i]))
				return false;

		float t1 = (corner[i] - orig[i]) / dir[i];
		float t2 = ((corner[i] + dimensions[i]) - orig[i]) / dir[i];

		if(t1 > t2) swap(t1, t2);
		if(t1 > tNear) tNear = t1;
		if(t2 < tFar) tFar = t2;

		if(tNear > tFar) return false;
		if(tFar	< 0) return false; //box behind ray.
	}

	return true;
}

AABB AABB::subdivide(int index)
{
	AABB ret;

	ret.corner.x = index % 2 == 0 ? corner.x : corner.x + (dimensions.x / 2);
	ret.corner.y = index < 4      ? corner.y : corner.y + (dimensions.y / 2);
	ret.corner.z = index % 4 < 2  ? corner.z : corner.z + (dimensions.z / 2);
	ret.dimensions = dimensions * 0.5f;

	return ret;
}
