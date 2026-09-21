//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::DEFAULT_SPLIT_METHOD);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth, Intersection inter) const
{
    // TO DO Implement Path Tracing Algorithm here
    if (depth >= maxDepth) return Vector3f();

    Intersection iShadingPoint = inter.happened ? inter : intersect(ray);
    if (!iShadingPoint.happened)
    {
        //return backgroundColor;
        return Vector3f(0);
    }

    Vector3f wo = -ray.direction; // 是否应该取反?
    Vector3f N = iShadingPoint.normal;
    Material* material = iShadingPoint.m;

    Intersection iLight;
    float pdf_light;
    sampleLight(iLight, pdf_light);

    Vector3f L_dir(0);
    if (pdf_light >= EPSILON)
    {
        Vector3f lightVec = iLight.coords - iShadingPoint.coords;
        Vector3f ws = normalize(lightVec);
        Vector3f NN = iLight.normal;
        float emitDistance = lightVec.norm();
        Intersection iBlockMiddle = intersect(Ray(iShadingPoint.coords + N * EPSILON, ws));

        if (abs(emitDistance - iBlockMiddle.distance) < 0.01f)
        {
            Vector3f brdf = material->eval(ws, wo, N);
            float cos_theta = dotProduct(ws, N);
            float cos_theta_prime = dotProduct(-ws, NN);
            L_dir = iLight.emit * brdf * cos_theta * cos_theta_prime / (emitDistance * emitDistance * pdf_light);
        }
    }

    Vector3f L_indir(0);
    if (get_random_float() < RussianRoulette)
    {
        Vector3f wi = normalize(material->sample(wo, N));
        Ray reflectedRay(iShadingPoint.coords + N * EPSILON, wi);
        Intersection iBouncePoint = intersect(reflectedRay);

        if (iBouncePoint.happened && !iBouncePoint.m->hasEmission())
        {
            float pdf_bounce = material->pdf(wi, wo, N);
            if (pdf_bounce >= EPSILON)
            {
                Vector3f bouncePointShadingResult = castRay(reflectedRay, depth + 1, iBouncePoint);
                Vector3f brdf = material->eval(wi, wo, N);
                float cos_theta = std::max(0.0f, dotProduct(wi, N));
                L_indir = bouncePointShadingResult * brdf * cos_theta / (pdf_bounce * RussianRoulette);
            }
        }
    }

    return material->getEmission() + L_dir + L_indir;
}