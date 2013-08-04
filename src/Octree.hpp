#ifndef OCTREE_HPP
#define OCTREE_HPP

#include <exception>
#include <vector>
#include <array>
#include <float.h>

#include <glm.hpp>

#include "GC.hpp"

class BadTreeException : std::exception {};

/* Axis-Aligned Bounding Box */
struct AABB
{
	glm::vec3 corner; //Min x,y,z values in box.
	glm::vec3 dimensions;
	bool intersect(const AABB& other);
	bool intersect(glm::vec3 orig, glm::vec3 dirn);
	AABB subdivide(int index);
};

namespace
{
	template <typename T>
	class OctreeNode
	{
	public:
		OctreeNode(AABB box) :box(box) {};
		virtual void insert(std::vector<T> data, AABB box) = 0;
		virtual std::vector<T> fetch(AABB box) = 0;
		AABB box;
	};

	template <typename T>
	class OctreeBranch : public OctreeNode<T>
	{
	public:
		OctreeBranch(AABB box, int depth);
		void insert(std::vector<T> data, AABB box);
		std::vector<T> fetch(AABB box);
	private:
		std::array<OctreeNode<T>*, 8> children;
	};

	template <typename T>
	class OctreeLeaf : public OctreeNode<T>
	{
	public:
		OctreeLeaf(AABB box) :OctreeNode(box) {};
		void insert(std::vector<T> data, AABB box);
		std::vector<T> fetch(AABB box);
	private:
		std::vector<T> data;
	};

	void swap(float& a, float& b);
}

/* Simple Octree data structure.
 * Stores data relating to a given 3D AABB.
 * Data are stored by providing data items, 
 *   along with an AABB enclosing their 
 *   conceptual volume.
 * Access data by providing an AABB or ray.
 */
template <typename T>
class Octree
{
public:
	Octree(AABB box, int depth);
	/* Data are inserted by providing a bounding box
	 *   and data vector. This vector is appended to all
	 *   leaves which intersect the given box.
	 */
	void insert(std::vector<T> data, AABB box)
		{root->insert(data, box)};

	/* Fetches concatenated data from all leaves
	 *   which intersect the given AABB.
	 */
	std::vector<T> fetch(AABB box)
		{root->fetch(data, box)};
private:
	OctreeNode<T>* root;
};

template <typename T>
Octree<T>::Octree(AABB box, int depth)
{
	if(depth < 0)
		throw(new BadTreeException);
	if(depth == 0)
		root = new OctreeLeaf(box);
	else
		root = new OctreeBranch(box, depth);
}

template <typename T>
OctreeBranch<T>::OctreeBranch(AABB box, int depth)
	:OctreeNode(box)
{
	if(depth > 1)
		for(int i = 0; i < 8; ++i)
			children[i] = new OctreeBranch(box.subdivide(i), depth - 1);

	else
		for(int i = 0; i < 8 ; ++i)
			children[i] = new OctreeLeaf(box.subdivide(i));
}

template <typename T>
void OctreeBranch<T>::insert(std::vector<T> data, AABB box)
{
	for(int i = 0; i < 8; ++i)
		children[i]->insert(data, box);
}

template <typename T>
std::vector<T> OctreeBranch<T>::fetch(AABB box)
{
	std::vector<T> result;
	//fetch from children and append each to result.
	for(int i = 0; i < 8; ++i)
	{
		std::vector<T> cResult = children[i]->fetch(box);
		result.insert(result.end(), cResult.begin(), cResult.end());
	}

	return result;
}

template <typename T>
void OctreeLeaf<T>::insert(std::vector<T> data, AABB box)
{
	if(this->box.intersect(box))
		this->data.insert(this->data.end(), data.begin(), data.end());
}

template <typename T>
std::vector<T> OctreeLeaf<T>::fetch(AABB box)
{
	if(this->box.intersect(box))
		return data;
	else
		return std::vector<T>();
}

#endif
