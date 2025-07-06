// 多种原创性
#ifndef MATERIAL_H
#define MATERIAL_H

#include <vecmath.h>

#include "ray.hpp"
#include "hit.hpp"
#include "texture.hpp"
#include <iostream>
#include <cmath>

// : Implement Shade function that computes Phong introduced in class.
class Material {
public:
    enum MATERIAL_TYPE {
        PHONG_MATERIAL,
        GLOSSY_MATERIAL,
        REFLECTIVE,
        REFRACTIVE,
    };

    enum SAMPLE_TYPE {
        UNIFORM,
        COS_WEIGHTED,
    };

protected:
    static int material_cnt;
    static std::vector<Material*> materials;

    int id;
    MATERIAL_TYPE type;
    SAMPLE_TYPE sample_type = SAMPLE_TYPE::COS_WEIGHTED;

    MaterialData materialData;
    std::shared_ptr<Texture2D> diffuseTexture2D, normalTexture2D;
    // Vector3f emission;
    // Vector3f diffuseColor;
    // Vector3f specularColor;
    // float shininess;

public:

    Material() {}

    // explicit Material(const Vector3f& d_color, const Vector3f& s_color = Vector3f::ZERO, float s = 0) :
    //     diffuseColor(d_color), specularColor(s_color), shininess(s) {
    // }

    virtual ~Material() = default;

    static int get_material_cnt() {
        return material_cnt;
    }

    static Material* get_material(int _id) {
        if (_id < 0 || _id >= material_cnt) {
            printf("ERROR: Invalid material ID.\n");
            exit(1);
        }
        return materials[_id];
    }

    int get_id() const {
        return id;
    }

    MATERIAL_TYPE get_type() const {
        return type;
    }

    // bool is_PBRMaterial() const {
    //     return type == MATERIAL_TYPE::DIFFUSE;
    // }

    bool normalTextureEnabled() const {
        return materialData.enable_normal_texture;
    }

    Vector3f getEmission() const {
        return materialData.emission;
    }

    bool hasEmission() const {
        return materialData.emission != Vector3f::ZERO;
    }

    Vector3f getAmbientColor() const {
        return materialData.ambient;
    }

    Vector3f getDiffuseColor() const {
        return materialData.diffuse;
    }

    Vector3f getDiffuseColor(const Hit& h) const {
        if (has_diffuse_texture() && materialData.enable_diffuse_texture) { // 此时交点一定位于三角网格上
            //std::cout << materialData.diffuse_texname << std::endl;
            if (diffuseTexture2D == nullptr) {
                printf("ERROR: No diffuseTexture2D.\n");
                exit(1);
            }
            // if (h.get_mesh_id() == -1) {
            //     printf("!!!\n");
            // }
            // printf("samp\n");
            return diffuseTexture2D->sample(h.get_texCoords());
        }

        return materialData.diffuse;
    }

    Vector3f getNormal(const Hit& h) const {
        //if (has_normal_texture()) { // 此时交点一定位于三角网格上
        if (normalTexture2D == nullptr) {
            printf("ERROR: No normalTexture2D.\n");
            exit(1);
        }
        return 2 * normalTexture2D->sample(h.get_texCoords()) - Vector3f(1.f, 1.f, 1.f);
        // }

        // return h.getNormal();
    }

    bool has_diffuse_texture() const { return materialData.diffuse_texname != ""; }

    bool has_normal_texture() const { return materialData.normal_texname != ""; }

    bool has_texture() const { return has_diffuse_texture() | has_normal_texture(); }

    //virtual Ray calculate_new_ray(const Ray& ray, const Hit& hit) const { return Ray(Vector3f::ZERO, Vector3f::UP); }

    // virtual Vector3f Shade(const Ray& ray, const Hit& hit, const Vector3f& dirToLight, const Vector3f& lightColor) = 0;
    // virtual Vector3f Shade(const Ray& ray, const Hit& hit, const Vector3f& dirToLight, const Vector3f& lightColor, const Vector2f& texCoords) {
    //     return Vector3f::ZERO;
    // }

    virtual float pdf(const Vector3f& wi, const Vector3f& N) const {
        // 独立实现
        if (sample_type == SAMPLE_TYPE::UNIFORM) {
            return M_1_2PI;
        }
        // 独立实现
        // 参考：https://zhuanlan.zhihu.com/p/503163354
        else {
            return std::max(0.f, Vector3f::dot(wi, N)) * M_1_PI;
        }
    }

    // 根据出射光方向，采样入射光。由光路可逆性原理，等价于给定入射光，采样出射光
    virtual Vector3f sampleBSDF(const Vector3f& wo, const Vector3f& N) const { // 在半球面上均匀采样（仅光线追踪）
        // 参考已有代码：GAMES101作业框架
        if (sample_type == SAMPLE_TYPE::UNIFORM) {
            float x_1 = get_random_float(), x_2 = get_random_float();
            float z = std::fabs(1.0f - 2.0f * x_1);
            float r = std::sqrt(1.0f - z * z), phi = 2 * M_PI * x_2;
            Vector3f localRay(r * std::cos(phi), r * std::sin(phi), z); // z=cosθ，r=sinθ，localRay为在以N=(0,0,1)为法向的半球面上的wi
            Vector3f B, C;
            if (std::fabs(N.x()) > std::fabs(N.y())) {
                float invLen = 1.0f / std::sqrt(N.x() * N.x() + N.z() * N.z());
                C = Vector3f(N.z() * invLen, 0.0f, -N.x() * invLen);
            }
            else {
                float invLen = 1.0f / std::sqrt(N.y() * N.y() + N.z() * N.z());
                C = Vector3f(0.0f, N.z() * invLen, -N.y() * invLen);
            }
            B = Vector3f::cross(C, N);
            //printf(">>>\n");
            return localRay.x() * B + localRay.y() * C + localRay.z() * N;
        }
        // 参考已有代码：GAMES101作业框架
        // 参考：https://zhuanlan.zhihu.com/p/503163354
        else {
            float x_1 = get_random_float(), x_2 = get_random_float();
            float z = std::sqrt(1 - x_1), r = std::sqrt(1 - z * z);
            float phi = M_2PI * x_2;
            Vector3f localRay(r * std::cos(phi), r * std::sin(phi), z);
            Vector3f B, C;
            if (std::fabs(N.x()) > std::fabs(N.y())) {
                float invLen = 1.0f / std::sqrt(N.x() * N.x() + N.z() * N.z());
                C = Vector3f(N.z() * invLen, 0.0f, -N.x() * invLen);
            }
            else {
                float invLen = 1.0f / std::sqrt(N.y() * N.y() + N.z() * N.z());
                C = Vector3f(0.0f, N.z() * invLen, -N.y() * invLen);
            }
            B = Vector3f::cross(C, N);
            //printf(">>>\n");
            return localRay.x() * B + localRay.y() * C + localRay.z() * N;
        }
    }

    // wi是入射光方向，wo是出射光方向。简化地理解（bounce=1的情形），wi从物体指向光源，wo从物体指向相机
    // 并未考虑cos项。cos项在渲染过程中单独计算，最终和其他值乘在一起
    virtual Vector3f evalBSDF(const Vector3f& wi, const Vector3f& wo, const Hit& h) const = 0;

    // virtual Vector3f evalBRDF(const Vector3f& wi, const Vector3f& wo, const Vector3f& N, const Vector2f& texCoords) const { return Vector3f::ZERO; }

    virtual Vector3f evalBSDF_Whitted(const Vector3f& wi, const Vector3f& wo, const Hit& h) const = 0;

    // virtual Vector3f evalBRDF_Whitted(const Vector3f& wi, const Vector3f& wo, const Vector3f& N, const Vector2f& texCoords) const { return Vector3f::ZERO; }
};

// 独立实现
// 参考：课程内容
// 对于光线追踪的情形，PhongMaterial即diffuse材质，只考虑diffuseColor，其他属性自动忽略
// 这只是一个权宜之计：严格来说，光线追踪中的material应为diffuse material等PBR类型的meterial。但额外定义其他material子类会使得进行对比实验更加麻烦
// 之后可能会改进
class PhongMaterial : public Material {
public:

    PhongMaterial(const Vector3f& _emission, const Vector3f& d_color, const Vector3f& s_color = Vector3f::ZERO, float s = 0) {
        id = material_cnt++;
        materials.push_back(this);
        type = MATERIAL_TYPE::PHONG_MATERIAL;

        diffuseTexture2D = nullptr;
        normalTexture2D = nullptr;
        materialData.emission = _emission;
        materialData.diffuse = d_color;
        materialData.specular = s_color;
        materialData.shininess = s;
        materialData.transparent = 0.f;
        materialData.dissolve = 1.f;
        // printf("PhongMaterial: ");
        // printvec3(diffuseColor);
        // printvec3(specularColor);
        // printf("\n");
    }

    PhongMaterial(const MaterialData& _materialData) {
        id = material_cnt++;
        materials.push_back(this);
        type = MATERIAL_TYPE::PHONG_MATERIAL;
        materialData = _materialData;
    }

    PhongMaterial(const MaterialData& _materialData, const std::string& texture_directory, Sampler2D::SAMPLER2D_TYPE _sampler2d_type) {
        id = material_cnt++;
        materials.push_back(this);
        type = MATERIAL_TYPE::PHONG_MATERIAL;
        materialData = _materialData;

        const std::string path_to_diffuseTexture = texture_directory + materialData.diffuse_texname;
        if (has_diffuse_texture()) {
            if (Texture2D::texture2d_map.find(path_to_diffuseTexture) != Texture2D::texture2d_map.end()) {
                diffuseTexture2D = Texture2D::texture2d_map[path_to_diffuseTexture];
            }
            else {
                diffuseTexture2D = std::make_shared<Texture2D>(_sampler2d_type, path_to_diffuseTexture);
                Texture2D::texture2d_map[path_to_diffuseTexture] = diffuseTexture2D;
            }
        }

        const std::string path_to_normalTexture = texture_directory + materialData.normal_texname;
        if (has_normal_texture()) {
            if (Texture2D::texture2d_map.find(path_to_normalTexture) != Texture2D::texture2d_map.end()) {
                normalTexture2D = Texture2D::texture2d_map[path_to_normalTexture];
            }
            else {
                normalTexture2D = std::make_shared<Texture2D>(_sampler2d_type, path_to_normalTexture);
                Texture2D::texture2d_map[path_to_normalTexture] = normalTexture2D;
            }
        }
    }

    // Vector3f getDiffuseColor() const {
    //     return diffuseColor;
    // }
    // Vector3f getSpecularColor() const {
    //     return specularColor;
    // }
    // Vector3f getShininess() const {
    //     return shininess;
    // }
    // Vector3f Shade(const Ray& ray, const Hit& hit, const Vector3f& dirToLight, const Vector3f& lightColor) override {
    //     Vector3f shaded = Vector3f::ZERO;
    //     Vector3f r = (2 * Vector3f::dot(hit.getNormal(), dirToLight) * hit.getNormal() - dirToLight).normalized();
    //     shaded += materialData.diffuse * std::max(0.f, Vector3f::dot(dirToLight, hit.getNormal()));
    //     shaded += materialData.specular * std::pow(std::max(0.f, Vector3f::dot(-ray.getDirection(), r)), materialData.shininess);
    //     // printf("[");
    //     // printvec3(lightColor); printf("%f", Vector3f::dot(dirToLight, hit.getNormal()));
    //     // printf("]\n");
    //     return lightColor * shaded;
    // }
    // Vector3f Shade(const Ray& ray, const Hit& hit, const Vector3f& dirToLight, const Vector3f& lightColor, const Vector2f& texCoords) {
    // }

    // // 参考已有代码：GAMES101作业框架
    // Vector3f sampleBSDF(const Vector3f& wo, const Vector3f& N) const override { // 在半球面上均匀采样（仅光线追踪）
    //     float x_1 = get_random_float(), x_2 = get_random_float();
    //     float z = std::fabs(1.0f - 2.0f * x_1);
    //     float r = std::sqrt(1.0f - z * z), phi = 2 * M_PI * x_2;
    //     Vector3f localRay(r * std::cos(phi), r * std::sin(phi), z);
    //     Vector3f B, C;
    //     if (std::fabs(N.x()) > std::fabs(N.y())) {
    //         float invLen = 1.0f / std::sqrt(N.x() * N.x() + N.z() * N.z());
    //         C = Vector3f(N.z() * invLen, 0.0f, -N.x() * invLen);
    //     }
    //     else {
    //         float invLen = 1.0f / std::sqrt(N.y() * N.y() + N.z() * N.z());
    //         C = Vector3f(0.0f, N.z() * invLen, -N.y() * invLen);
    //     }
    //     B = Vector3f::cross(C, N);
    //     //printf(">>>\n");
    //     return localRay.x() * B + localRay.y() * C + localRay.z() * N;
    // }

    Vector3f evalBSDF(const Vector3f& wi, const Vector3f& wo, const Hit& h) const override {
        // Vector3f diffuseColor = materialData.diffuse;
        // if (has_diffuse_texture()) {
        //     assert(texture2d != nullptr);
        //     diffuseColor = texture2d->sample();
        // }

        //return getDiffuseColor(h);
        return Vector3f::dot(h.getFaceNormal(), wo) > 0.f ? getDiffuseColor(h) * M_1_PI : Vector3f::ZERO;
    }

    // Vector3f evalBRDF(const Vector3f& wi, const Vector3f& wo, const Vector3f& N, const Vector2f& texCoords) const override {
    //     return Vector3f::dot(N, wo) > 0.f ? materialData.diffuse * M_1_PI : Vector3f::ZERO;
    // }

    Vector3f evalBSDF_Whitted(const Vector3f& wi, const Vector3f& wo, const Hit& h) const override { // Whitted-style的BRDF，考虑了物体的高光
        // Vector3f diffuseColor = materialData.diffuse;
        // if (has_diffuse_texture()) {
        //     assert(texture2d != nullptr);
        //     diffuseColor = texture2d->sample();
        // }
        //printf("e1\n");
        Vector3f brdf = Vector3f::ZERO;
        Vector3f r = (2 * Vector3f::dot(h.getNormal(), wi) * h.getNormal() - wi).normalized();
        brdf += getDiffuseColor(h) * std::max(0.f, Vector3f::dot(wi, h.getNormal()));
        brdf += materialData.specular * std::pow(std::max(0.f, Vector3f::dot(wo, r)), materialData.shininess);
        //printf("e2\n");
        // if (has_diffuse_texture()) {
        //     printvec3(getDiffuseColor(h) * std::max(0.f, Vector3f::dot(wi, h.getNormal())));
        //     printvec3(materialData.specular * std::pow(std::max(0.f, Vector3f::dot(wo, r)), materialData.shininess));
        //     printf("\n");
        // }
        return brdf;
    }

    // Vector3f evalBRDF_Whitted(const Vector3f& wi, const Vector3f& wo, const Vector3f& N, const Vector2f& texCoords) const override { // Whitted-style的BRDF，考虑了物体的高光
    //     Vector3f brdf = Vector3f::ZERO;
    //     Vector3f r = (2 * Vector3f::dot(N, wi) * N - wi).normalized();
    //     brdf += materialData.diffuse * std::max(0.f, Vector3f::dot(wi, N));
    //     brdf += materialData.specular * std::pow(std::max(0.f, Vector3f::dot(wo, r)), materialData.shininess);

    //     return brdf;
    // }

};

// 独立实现
// 参考：https://zhuanlan.zhihu.com/p/152226698、https://blog.csdn.net/qq_39300235/article/details/105451795
class GlossyMaterial : public Material {
private:
    float K, alpha2;

    float geometrySubTerm(const float costheta) const {
        return costheta / (costheta * (1.f - K) + K);
    }

    float GGX(const float costheta) const {
        float c = 1.0 / (costheta * costheta * (alpha2 - 1) + 1);
        return alpha2 * M_1_PI * c * c;
    }

public:

    GlossyMaterial(const Vector3f& _emission, const Vector3f& _albedo, float r, float m) {
        id = material_cnt++;
        materials.push_back(this);
        type = MATERIAL_TYPE::GLOSSY_MATERIAL;

        diffuseTexture2D = nullptr;
        normalTexture2D = nullptr;
        materialData.emission = _emission;
        materialData.albedo = _albedo;
        materialData.F0 = (1.f - m) * Vector3f(0.04) + m * _albedo;
        materialData.roughness = r;
        materialData.metallic = m;

        K = (r + 1) * (r + 1) / 8;
        alpha2 = r * r;
    }

    Vector3f evalBSDF(const Vector3f& wi, const Vector3f& wo, const Hit& h) const override {
        float cos_wi_n = Vector3f::dot(h.getFaceNormal(), wi), cos_wo_n = Vector3f::dot(h.getFaceNormal(), wo);
        if (cos_wi_n <= 0.f || cos_wo_n <= 0.f) {
            return Vector3f::ZERO;
        }

        Vector3f m = (wi + wo).normalized();

        Vector3f F = materialData.F0 + (Vector3f(1.f) - materialData.F0) * std::pow(1 - Vector3f::dot(m, wi), 5);
        float G = geometrySubTerm(cos_wi_n) * geometrySubTerm(cos_wo_n);
        float D = GGX(Vector3f::dot(h.getFaceNormal(), m));

        Vector3f Kd = (1.f - F) * (1.f - materialData.metallic);
        Vector3f CT = F * G * D / (4 * cos_wi_n * cos_wo_n);

        return Kd * materialData.albedo * M_1_PI + CT;
    }

    Vector3f evalBSDF_Whitted(const Vector3f& wi, const Vector3f& wo, const Hit& h) const override {
        printf("ERROR: Whitted-Style raytracing does not support glossy materials.\n");
        exit(1);
        return Vector3f::ZERO;
    }
};

// 独立实现
// 参考：课程内容
// 完美反射材质：仅发生反射
class ReflectiveMaterial : public Material {
public:
    ReflectiveMaterial(const Vector3f& _emission, const Vector3f& tf, float tr) {
        id = material_cnt++;
        materials.push_back(this);
        type = MATERIAL_TYPE::REFLECTIVE;

        diffuseTexture2D = nullptr;
        normalTexture2D = nullptr;
        materialData.emission = _emission;
        materialData.transmittance = tf;
        materialData.transparent = tr;
        materialData.dissolve = 1.f - tr;
    }

    // Ray calculate_new_ray(const Ray& ray, const Hit& hit) const { // ray从相机/上一个交点指向当前交点
    //     Vector3f new_dir;
    //     float cos_theta_I = Vector3f::dot(ray.getDirection(), hit.getNormal());
    //     new_dir = ray.getDirection() - 2 * cos_theta_I * hit.getNormal();
    //     new_dir.normalize();
    //     return Ray(hit.getPoint(), new_dir);
    // }

    float pdf(const Vector3f& wi, const Vector3f& N) const override {
        return 1.f;
    }

    Vector3f sampleBSDF(const Vector3f& wo, const Vector3f& N) const override {
        return (2 * Vector3f::dot(wo, N) * N - wo).normalized();
    }

    Vector3f evalBSDF(const Vector3f& wi, const Vector3f& wo, const Hit& h) const override {
        return materialData.transmittance; // 此时一定没有纹理贴图
    }

    Vector3f evalBSDF_Whitted(const Vector3f& wi, const Vector3f& wo, const Hit& h) const override {
        return materialData.transmittance; // *std::max(0.f, Vector3f::dot(wi, h.getNormal())); // 此时一定没有纹理贴图
    }

    // Vector3f Shade(const Ray& ray, const Hit& hit, const Vector3f& dirToLight, const Vector3f& lightColor) {
    //     return Vector3f::ZERO;
    // }
};


// 独立实现
// 参考：课程内容
// 完美折射材质：仅发生折射或全反射
class RefractiveMaterial : public Material {
public:
    RefractiveMaterial(const Vector3f& _emission, const Vector3f& tf, float r, float tr) {
        id = material_cnt++;
        materials.push_back(this);
        type = MATERIAL_TYPE::REFRACTIVE;

        diffuseTexture2D = nullptr;
        normalTexture2D = nullptr;
        materialData.emission = _emission;
        materialData.transmittance = tf;
        materialData.ior = r;
        materialData.transparent = tr;
        materialData.dissolve = 1.f - tr;
    }

    // Ray calculate_new_ray(const Ray& ray, const Hit& hit) const { // 潜在的问题：两个折射率不等的透明物体紧挨在一起，无法实现光线穿过第二个物体的效果
    //     // ray从相机/上一个交点指向当前交点
    //     Vector3f new_dir = Vector3f::ZERO;
    //     float cos_theta_I = Vector3f::dot(ray.getDirection(), hit.getNormal());
    //     float inv_relative_refractive_index = cos_theta_I >= 0.f ? materialData.ior : 1.f / materialData.ior; //出射介质相对入射介质的相对折射率的倒数，等于=ri_入射/ri_出射
    //     float temp = (1 - cos_theta_I * cos_theta_I) * inv_relative_refractive_index * inv_relative_refractive_index;
    //     if (temp >= 0.f && temp <= 1.f) { //发生折射
    //         float cos_theta_T = std::sqrt(1.f - temp);
    //         //printf("[REFRA %f %f %f] ", inv_relative_refractive_index, cos_theta_I, cos_theta_T);
    //         //printvec3(ray.getDirection()); printvec3(hit.getNormal());
    //         new_dir = inv_relative_refractive_index * ray.getDirection()
    //             + (inv_relative_refractive_index * std::fabs(cos_theta_I) - cos_theta_T) * hit.getNormal();
    //     }
    //     else { //发生全反射
    //         new_dir = ray.getDirection() - 2 * cos_theta_I * hit.getNormal();
    //     }
    //     new_dir.normalize();
    //     //printvec3(ray.getDirection()); printvec3(new_dir); printf("\n");
    //     return Ray(hit.getPoint(), new_dir);
    // }

    float pdf(const Vector3f& wi, const Vector3f& N) const override {
        return 1.f;
    }

    Vector3f sampleBSDF(const Vector3f& wo, const Vector3f& N) const override {
        Vector3f new_dir = Vector3f::ZERO;

        float cos_theta_I = Vector3f::dot(wo, N);
        float inv_relative_refractive_index = cos_theta_I >= 0.f ? 1.f / materialData.ior : materialData.ior; //出射介质相对入射介质的相对折射率的倒数，等于=ri_入射/ri_出射
        float temp = (1 - cos_theta_I * cos_theta_I) * inv_relative_refractive_index * inv_relative_refractive_index;

        if (temp >= 0.f && temp <= 1.f) { //发生折射
            float cos_theta_T = std::sqrt(1.f - temp);
            //printf("[REFRA %f %f %f] ", inv_relative_refractive_index, cos_theta_I, cos_theta_T);
            //printvec3(ray.getDirection()); printvec3(hit.getNormal());
            return ((inv_relative_refractive_index * std::fabs(cos_theta_I) - cos_theta_T) * N
                - inv_relative_refractive_index * wo).normalized();
        }

        //发生全反射
        return (2 * cos_theta_I * N - wo).normalized();

        // new_dir.normalize();
        // //printvec3(ray.getDirection()); printvec3(new_dir); printf("\n");
        //return Ray(hit.getPoint(), new_dir);
    }

    Vector3f evalBSDF(const Vector3f& wi, const Vector3f& wo, const Hit& h) const override {
        return materialData.transmittance; // 此时一定没有纹理贴图
    }

    Vector3f evalBSDF_Whitted(const Vector3f& wi, const Vector3f& wo, const Hit& h) const override {
        return materialData.transmittance; // *std::max(0.f, Vector3f::dot(wi, h.getNormal())); // 此时一定没有纹理贴图
    }

    // Vector3f Shade(const Ray& ray, const Hit& hit, const Vector3f& dirToLight, const Vector3f& lightColor) {
    //     return Vector3f::ZERO;
    // }
};

// class DiffuseMaterial : public Material {

// };

// class MicrofacetMaterial : public Material {

// };


#endif // MATERIAL_H
