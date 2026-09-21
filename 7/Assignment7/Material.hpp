//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_MATERIAL_H
#define RAYTRACING_MATERIAL_H

#include "Vector.hpp"

enum MaterialType { DIFFUSE, MICROFACET };

class Material{
private:

    // Compute reflection direction
    Vector3f reflect(const Vector3f &I, const Vector3f &N) const
    {
        return I - 2 * dotProduct(I, N) * N;
    }

    // Compute refraction direction using Snell's law
    //
    // We need to handle with care the two possible situations:
    //
    //    - When the ray is inside the object
    //
    //    - When the ray is outside.
    //
    // If the ray is outside, you need to make cosi positive cosi = -N.I
    //
    // If the ray is inside, you need to invert the refractive indices and negate the normal N
    Vector3f refract(const Vector3f &I, const Vector3f &N, const float &ior) const
    {
        float cosi = clamp(-1, 1, dotProduct(I, N));
        float etai = 1, etat = ior;
        Vector3f n = N;
        if (cosi < 0) { cosi = -cosi; } else { std::swap(etai, etat); n= -N; }
        float eta = etai / etat;
        float k = 1 - eta * eta * (1 - cosi * cosi);
        return k < 0 ? 0 : eta * I + (eta * cosi - sqrtf(k)) * n;
    }

    // Compute Fresnel equation
    //
    // \param I is the incident view direction
    //
    // \param N is the normal at the intersection point
    //
    // \param ior is the material refractive index
    //
    // \param[out] kr is the amount of light reflected
    void fresnel(const Vector3f &I, const Vector3f &N, const float &ior, float &kr) const
    {
        float cosi = clamp(-1, 1, dotProduct(I, N));
        float etai = 1, etat = ior;
        if (cosi > 0) {  std::swap(etai, etat); }
        // Compute sini using Snell's law
        float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
        // Total internal reflection
        if (sint >= 1) {
            kr = 1;
        }
        else {
            float cost = sqrtf(std::max(0.f, 1 - sint * sint));
            cosi = fabsf(cosi);
            float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
            float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
            kr = (Rs * Rs + Rp * Rp) / 2;
        }
        // As a consequence of the conservation of energy, transmittance is given by:
        // kt = 1 - kr;
    }

    Vector3f toWorld(const Vector3f &a, const Vector3f &N){
        Vector3f B, C;
        if (std::fabs(N.x) > std::fabs(N.y)){
            float invLen = 1.0f / std::sqrt(N.x * N.x + N.z * N.z);
            C = Vector3f(N.z * invLen, 0.0f, -N.x *invLen);
        }
        else {
            float invLen = 1.0f / std::sqrt(N.y * N.y + N.z * N.z);
            C = Vector3f(0.0f, N.z * invLen, -N.y *invLen);
        }
        B = crossProduct(C, N);
        return a.x * B + a.y * C + a.z * N;
    }

    // Microfacet GGX helper functions
    float DistributionGGX(const Vector3f &N, const Vector3f &H, float alpha) const {
        float NdotH = std::max(0.0f, dotProduct(N, H));
        float alpha2 = alpha * alpha;
        float denom = (NdotH * NdotH * (alpha2 - 1.0f) + 1.0f);
        return alpha2 / (M_PI * denom * denom);
    }

    float GeometrySchlickGGX(float NdotV, float k) const {
        return NdotV / (NdotV * (1.0f - k) + k);
    }

    float GeometrySmith(const Vector3f &N, const Vector3f &V, const Vector3f &L, float alpha) const {
        float NdotV = std::max(0.0f, dotProduct(N, V));
        float NdotL = std::max(0.0f, dotProduct(N, L));
        float k = alpha / 2.0f;
        return GeometrySchlickGGX(NdotV, k) * GeometrySchlickGGX(NdotL, k);
    }

    Vector3f FresnelSchlick(float cosTheta, const Vector3f &F0) const {
        float factor = std::pow(std::max(0.0f, 1.0f - cosTheta), 5.0f);
        return F0 + factor * (Vector3f(1.0f) - F0);
    }

public:
    MaterialType m_type;
    //Vector3f m_color;
    Vector3f m_emission;
    float ior;
    Vector3f Kd, Ks;
    float specularExponent;
    //Texture tex;
    bool use_importance_sampling = true;
    float roughness = 0.35f;
    float metallic = 1.0f;

    inline Material(MaterialType t=DIFFUSE, Vector3f e=Vector3f(0,0,0));
    inline MaterialType getType();
    //inline Vector3f getColor();
    inline Vector3f getColorAt(double u, double v);
    inline Vector3f getEmission();
    inline bool hasEmission();

    // sample a ray by Material properties
    inline Vector3f sample(const Vector3f &wi, const Vector3f &N);
    // given a ray, calculate the PdF of this ray
    inline float pdf(const Vector3f &wi, const Vector3f &wo, const Vector3f &N);
    // given a ray, calculate the contribution of this ray
    inline Vector3f eval(const Vector3f &wi, const Vector3f &wo, const Vector3f &N);

};

Material::Material(MaterialType t, Vector3f e){
    m_type = t;
    //m_color = c;
    m_emission = e;
    use_importance_sampling = true;
    roughness = 0.35f;
    metallic = 1.0f;
}

MaterialType Material::getType(){return m_type;}
///Vector3f Material::getColor(){return m_color;}
Vector3f Material::getEmission() {return m_emission;}
bool Material::hasEmission() {
    if (m_emission.norm() > EPSILON) return true;
    else return false;
}

Vector3f Material::getColorAt(double u, double v) {
    return Vector3f();
}


Vector3f Material::sample(const Vector3f &wi, const Vector3f &N){
    switch(m_type){
        case DIFFUSE:
        {
            float x_1 = get_random_float(), x_2 = get_random_float();
            float phi = 2.0f * M_PI * x_2;
            float x, y, z;

            if (use_importance_sampling) {
                // Cosine-weighted hemisphere sampling
                float r = std::sqrt(x_1);
                x = r * std::cos(phi);
                y = r * std::sin(phi);
                z = std::sqrt(std::max(0.0f, 1.0f - x_1));
            } else {
                // Uniform hemisphere sampling
                z = x_1;
                float r = std::sqrt(std::max(0.0f, 1.0f - z * z));
                x = r * std::cos(phi);
                y = r * std::sin(phi);
            }

            Vector3f localRay(x, y, z);
            return toWorld(localRay, N);
        }
        case MICROFACET:
        {
            // GGX NDF importance sampling for half-vector H
            float alpha = std::max(0.001f, roughness * roughness);
            float x_1 = get_random_float(), x_2 = get_random_float();
            float phi = 2.0f * M_PI * x_2;

            float cos_theta = std::sqrt((1.0f - x_1) / (1.0f + (alpha * alpha - 1.0f) * x_1));
            float sin_theta = std::sqrt(std::max(0.0f, 1.0f - cos_theta * cos_theta));

            Vector3f h_local(sin_theta * std::cos(phi), sin_theta * std::sin(phi), cos_theta);
            Vector3f H = normalize(toWorld(h_local, N));

            // Reflect view direction wi (wo in shading point) about microfacet normal H
            Vector3f L = normalize(2.0f * dotProduct(wi, H) * H - wi);
            return L;
        }
    }
}

float Material::pdf(const Vector3f &wi, const Vector3f &wo, const Vector3f &N){
    switch(m_type){
        case DIFFUSE:
        {
            float cos_theta = dotProduct(wi, N);
            float cos_theta_prime = dotProduct(wo, N);
            if (cos_theta > 0.0f && cos_theta_prime > 0.0f) {
                if (use_importance_sampling) {
                    return cos_theta / M_PI;
                } else {
                    return 0.5f / M_PI;
                }
            } else {
                return 0.0f;
            }
            break;
        }
        case MICROFACET:
        {
            float NdotL = dotProduct(wi, N);
            float NdotV = dotProduct(wo, N);
            if (NdotL <= 0.0f || NdotV <= 0.0f)
                return 0.0f;

            Vector3f H = normalize(wi + wo);
            float VdotH = std::max(0.0f, dotProduct(wo, H));
            if (VdotH <= 0.0f) return 0.0f;

            float alpha = std::max(0.001f, roughness * roughness);
            float D = DistributionGGX(N, H, alpha);
            float NdotH = std::max(0.0f, dotProduct(N, H));

            // pdf = D * (N . H) / (4 * (V . H))
            return (D * NdotH) / (4.0f * VdotH);
        }
    }
}

Vector3f Material::eval(const Vector3f &wi, const Vector3f &wo, const Vector3f &N){
    switch(m_type){
        case DIFFUSE:
        {
            // calculate the contribution of diffuse model
            float cos_theta = dotProduct(wi, N);
            float cos_theta_prime = dotProduct(wo, N);
            if (cos_theta > 0.0f && cos_theta_prime > 0.0f) {
                Vector3f diffuse = Kd / M_PI;
                return diffuse;
            }
            else
                return Vector3f(0.0f);
            break;
        }
        case MICROFACET:
        {
            float NdotL = dotProduct(wi, N);
            float NdotV = dotProduct(wo, N);
            if (NdotL <= 0.0f || NdotV <= 0.0f)
                return Vector3f(0.0f);

            Vector3f H = normalize(wi + wo);
            float VdotH = std::max(0.0f, dotProduct(wo, H));

            float alpha = std::max(0.001f, roughness * roughness);
            float D = DistributionGGX(N, H, alpha);
            float G = GeometrySmith(N, wo, wi, alpha);

            Vector3f F0 = lerp(Vector3f(0.04f), Kd, metallic);
            Vector3f F = FresnelSchlick(VdotH, F0);

            Vector3f specular = (D * G / (4.0f * NdotL * NdotV)) * F;
            Vector3f diffuse = (1.0f - metallic) * (Vector3f(1.0f) - F) * (Kd / M_PI);

            return diffuse + specular;
        }
    }
}

#endif //RAYTRACING_MATERIAL_H
