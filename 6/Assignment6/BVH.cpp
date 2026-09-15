#include <algorithm>
#include <cassert>
#include "BVH.hpp"

BVHAccel::BVHAccel(std::vector<Object*> p, int maxPrimsInNode,
	SplitMethod splitMethod)
	: maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
	primitives(std::move(p))
{
	time_t start, stop;
	time(&start);
	if (primitives.empty())
		return;

	root = recursiveBuild(primitives);

	time(&stop);
	double diff = difftime(stop, start);
	int hrs = (int)diff / 3600;
	int mins = ((int)diff / 60) - (hrs * 60);
	int secs = (int)diff - (hrs * 3600) - (mins * 60);

	printf(
		"\rBVH Generation complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n",
		hrs, mins, secs);
}

BVHBuildNode* BVHAccel::recursiveBuild(std::vector<Object*> objects)
{
	BVHBuildNode* node = new BVHBuildNode();

	// Compute bounds of all primitives in BVH node
	Bounds3 bounds;
	for (int i = 0; i < objects.size(); ++i)
		bounds = Union(bounds, objects[i]->getBounds());
	if (objects.size() == 1) {
		// Create leaf _BVHBuildNode_
		node->bounds = objects[0]->getBounds();
		node->object = objects[0];
		node->left = nullptr;
		node->right = nullptr;
		return node;
	}
	else if (objects.size() == 2) {
		node->left = recursiveBuild(std::vector{ objects[0] });
		node->right = recursiveBuild(std::vector{ objects[1] });

		node->bounds = Union(node->left->bounds, node->right->bounds);
		return node;
	}
	else {
		double minCost = std::numeric_limits<double>::max();
		int bestSplit = -1;
		Bounds3 centroidBounds;
		for (int i = 0; i < objects.size(); ++i)
			centroidBounds =
			Union(centroidBounds, objects[i]->getBounds().Centroid());
		int dim = centroidBounds.maxExtent();
		/*switch (dim) {
		case 0:
			std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
				return f1->getBounds().Centroid().x <
					f2->getBounds().Centroid().x;
				});
			break;
		case 1:
			break;
		case 2:
			break;
		}*/
		std::sort(objects.begin(), objects.end(), [dim](Object* a, Object* b) {
			return a->getBounds().Centroid()[dim] < b->getBounds().Centroid()[dim];
			});

		for (int i = 1; i < objects.size(); i++) {
			Bounds3 leftBounds, rightBounds;
			for (int j = 0; j < i; j++)leftBounds = Union(leftBounds, objects[j]->getBounds());
			for (int j = i; j < objects.size(); j++)rightBounds = Union(rightBounds, objects[j]->getBounds());
			double cost = calculateSAH(i, objects.size() - i, leftBounds, rightBounds);
			if (cost < minCost) {
				minCost = cost;
				bestSplit = i;
			}
		}

		if (bestSplit > 0) {
			auto beginning = objects.begin();
			auto middling = objects.begin() + bestSplit;
			auto ending = objects.end();

			auto leftshapes = std::vector<Object*>(beginning, middling);
			auto rightshapes = std::vector<Object*>(middling, ending);

			assert(objects.size() == (leftshapes.size() + rightshapes.size()));

			node->left = recursiveBuild(leftshapes);
			node->right = recursiveBuild(rightshapes);

			node->bounds = Union(node->left->bounds, node->right->bounds);
		}
	}

	return node;
}

Intersection BVHAccel::Intersect(const Ray& ray) const
{
	Intersection isect;
	if (!root)
		return isect;
	isect = BVHAccel::getIntersection(root, ray);
	return isect;
}

//struct BVHBuildNode {
//    Bounds3 bounds;
//    BVHBuildNode* left;
//    BVHBuildNode* right;
//    Object* object;
//
//public:
//    int splitAxis = 0, firstPrimOffset = 0, nPrimitives = 0;
//    // BVHBuildNode Public Methods
//    BVHBuildNode() {
//        bounds = Bounds3();
//        left = nullptr; right = nullptr;
//        object = nullptr;
//    }
//};

Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
	// std::cout << "is a node" << std::endl;
	// TODO Traverse the BVH to find intersection
	Intersection inter;
	std::array<int, 3> dirIsNeg{ int(ray.direction.x > 0), int(ray.direction.y > 0), int(ray.direction.z > 0) };
	if (node == nullptr)return inter;
	if (!node->bounds.IntersectP(ray, ray.direction_inv,dirIsNeg))return inter;

	if (node->left == nullptr && node->right == nullptr) { // is a leaf node
		//std::cout << "is a leaf node" << std::endl;
		return node->object->getIntersection(ray);
	}
	//std::cout << "is a parent node" << std::endl;
	Intersection hit1 = getIntersection(node->left, ray);
	Intersection hit2 = getIntersection(node->right, ray);

	return hit1.distance < hit2.distance ? hit1 : hit2;
}