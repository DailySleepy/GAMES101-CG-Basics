#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include "BVH.hpp"

BVHAccel::BVHAccel(std::vector<Object*> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
      primitives(std::move(p))
{
    auto start = std::chrono::high_resolution_clock::now();
    if (primitives.empty())
        return;

    root = recursiveBuild(primitives);

    auto stop = std::chrono::high_resolution_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    std::cout << "\rBVH Generation complete: \nTime Taken: " << diff << " ms ("
              << (splitMethod == SplitMethod::SAH ? "SAH" : "NAIVE") << ")\n\n";
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
        node->area = objects[0]->getArea();
        return node;
    }
    else if (objects.size() == 2) {
        node->left = recursiveBuild(std::vector{objects[0]});
        node->right = recursiveBuild(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        node->area = node->left->area + node->right->area;
        return node;
    }
    else {
        std::vector<Object*> leftshapes;
        std::vector<Object*> rightshapes;

        if (splitMethod == SplitMethod::NAIVE) {
            Bounds3 centroidBounds;
            for (int i = 0; i < objects.size(); ++i)
                centroidBounds =
                    Union(centroidBounds, objects[i]->getBounds().Centroid());
            int dim = centroidBounds.maxExtent();
            switch (dim) {
            case 0:
                std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                    return f1->getBounds().Centroid().x <
                           f2->getBounds().Centroid().x;
                });
                break;
            case 1:
                std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                    return f1->getBounds().Centroid().y <
                           f2->getBounds().Centroid().y;
                });
                break;
            case 2:
                std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                    return f1->getBounds().Centroid().z <
                           f2->getBounds().Centroid().z;
                });
                break;
            }

            auto beginning = objects.begin();
            auto middling = objects.begin() + (objects.size() / 2);
            auto ending = objects.end();

            leftshapes = std::vector<Object*>(beginning, middling);
            rightshapes = std::vector<Object*>(middling, ending);
        }
        else if (splitMethod == SplitMethod::SAH) {
            constexpr float TRAVERSAL_COST = 1.0f;
            constexpr float INTERSECTION_COST = 1.0f;
            float minCost = std::numeric_limits<float>::infinity();
            float totalArea = bounds.SurfaceArea();
            if (totalArea <= 0.0f) totalArea = 1e-4f;
            int bestAxis = 0;
            int bestIndex = 1;
            int n = objects.size();

            for (int axis = 0; axis < 3; axis++)
            {
                std::sort(objects.begin(), objects.end(), [axis](auto f1, auto f2)
                {
                    return f1->getBounds().Centroid()[axis] <
                        f2->getBounds().Centroid()[axis];
                });

                std::vector<Bounds3> prefixBounds(n), suffixBounds(n);

                prefixBounds[0] = objects[0]->getBounds();
                for (int i = 1; i < n; i++)
                {
                    prefixBounds[i] = Union(prefixBounds[i - 1], objects[i]->getBounds());
                }

                suffixBounds[n - 1] = objects[n - 1]->getBounds();
                for (int i = n - 2; i >= 0; i--)
                {
                    suffixBounds[i] = Union(suffixBounds[i + 1], objects[i]->getBounds());
                }

                for (int i = 1; i < n; i++)
                {
                    float sA = prefixBounds[i - 1].SurfaceArea();
                    float sB = suffixBounds[i].SurfaceArea();
                    float nA = i;
                    float nB = n - i;

                    float cost = TRAVERSAL_COST + INTERSECTION_COST * (sA * nA + sB * nB) / totalArea;
                    if (cost < minCost)
                    {
                        minCost = cost;
                        bestAxis = axis;
                        bestIndex = i;
                    }
                }
            }

            std::sort(objects.begin(), objects.end(), [bestAxis](auto f1, auto f2)
            {
                return f1->getBounds().Centroid()[bestAxis] <
                    f2->getBounds().Centroid()[bestAxis];
            });

            auto beginning = objects.begin();
            auto middling = objects.begin() + bestIndex;
            auto ending = objects.end();

            leftshapes = std::vector<Object*>(beginning, middling);
            rightshapes = std::vector<Object*>(middling, ending);
        }

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuild(leftshapes);
        node->right = recursiveBuild(rightshapes);

        node->bounds = Union(node->left->bounds, node->right->bounds);
        node->area = node->left->area + node->right->area;
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

Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
    Intersection isect;
    if (node != nullptr && node->bounds.IntersectP(ray, ray.direction_inv, ray.dirIsNeg))
    {
        if (node->object)
        {
            return node->object->getIntersection(ray);
        }
        auto hit1 = getIntersection(node->left, ray);
        auto hit2 = getIntersection(node->right, ray);
        isect = hit1.distance < hit2.distance ? hit1 : hit2;
    }
    return isect;
}


void BVHAccel::getSample(BVHBuildNode* node, float p, Intersection &pos, float &pdf){
    if(node->left == nullptr || node->right == nullptr){
        node->object->Sample(pos, pdf);
        pdf *= node->area;
        return;
    }
    if(p < node->left->area) getSample(node->left, p, pos, pdf);
    else getSample(node->right, p - node->left->area, pos, pdf);
}

void BVHAccel::Sample(Intersection &pos, float &pdf){
    float p = get_random_float() * root->area;
    getSample(root, p, pos, pdf);
    pdf /= root->area;
}