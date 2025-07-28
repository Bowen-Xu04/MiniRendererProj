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

    // enum SAMPLE_TYPE {
    //     UNIFORM,
    //     COS_WEIGHTED,
    //     BRDF,
    //     MIS
    // };

protected:
    static int material_cnt;
    static std::vector<Material*> materials;

    int id;
    MATERIAL_TYPE type;
    //SAMPLE_TYPE sample_type; // = SAMPLE_TYPE::COS_WEIGHTED;

    MaterialData materialData;
    std::shared_ptr<Texture2D> diffuseTexture2D, normalTexture2D, emissiveTexture2D;

public:
    Material() = delete;

    Material(MATERIAL_TYPE _type) : id(material_cnt++), type(_type) {}

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

    bool normalTextureEnabled() const {
        return materialData.enable_normal_texture;
    }

    Vector3f getEmission() const {
        return materialData.emission;
    }

    Vector3f getEmission(const Hit& h) const {
        if (has_emissive_texture()) {
            if (emissiveTexture2D == nullptr) {
                printf("ERROR: No emissiveTexture2D.\n");
                exit(1);
            }
            return emissiveTexture2D->sample(h.get_texCoords());
        }

        return materialData.emission;
    }

    bool hasEmission() const {
        return materialData.emission != Vector3f::ZERO || has_emissive_texture();
    }

    Vector3f getAmbientColor() const {
        return materialData.ambient;
    }

    Vector3f getDiffuseColor() const {
        return materialData.diffuse;
    }

    Vector3f getDiffuseColor(const Hit& h) const {
        if (has_diffuse_texture() && materialData.enable_diffuse_texture) { // 此时交点一定位于三角网格上
            if (diffuseTexture2D == nullptr) {
                printf("ERROR: No diffuseTexture2D.\n");
                exit(1);
            }
            return diffuseTexture2D->sample(h.get_texCoords());
        }

        return materialData.diffuse;
    }

    Vector3f getNormal(const Hit& h) const {
        if (normalTexture2D == nullptr) {
            printf("ERROR: No normalTexture2D.\n");
            exit(1);
        }
        return 2 * normalTexture2D->sample(h.get_texCoords()) - Vector3f(1.f, 1.f, 1.f);
    }

    bool has_diffuse_texture() const { return materialData.diffuse_texname != ""; }

    bool has_normal_texture() const { return materialData.normal_texname != ""; }

    bool has_emissive_texture() const { return materialData.emissive_texname != ""; }

    bool has_texture() const { return has_diffuse_texture() | has_normal_texture() | has_emissive_texture(); }

    virtual float pdf(const Vector3f& wi, const Vector3f& wo, const Vector3f& N, bool tag) const = 0;

    virtual float average_pdf(const Vector3f& wi, const Vector3f& wo, const Vector3f& N) const = 0;  // 仅供采用MIS采样方法求解pdf时使用
    //     printf("ERROR: Material type must be GLOSSY_MATERIAL.\n");
    //     exit(1);
    //     return 0.f;
    // }

    virtual Vector3f sampleBSDF(const Vector3f& wo, const Vector3f& N, bool& tag) const = 0;

    // wi是入射光方向，wo是出射光方向。简化地理解（bounce=1的情形），wi从物体指向光源，wo从物体指向相机
    // 并未考虑cos项。cos项在渲染过程中单独计算，最终和其他值乘在一起
    virtual Vector3f evalBSDF(const Vector3f& wi, const Vector3f& wo, const Hit& h) const = 0;

    virtual Vector3f evalBSDF_Whitted(const Vector3f& wi, const Vector3f& wo, const Hit& h) const = 0;
};

// 独立实现
// 参考：课程内容
// 对于光线追踪的情形，PhongMaterial即diffuse材质，只考虑diffuseColor，其他属性自动忽略
// 这只是一个权宜之计：严格来说，光线追踪中的material应为diffuse material等PBR类型的meterial。但额外定义其他material子类会使得进行对比实验更加麻烦
// 之后可能会改进
class PhongMaterial : public Material {
public:
    PhongMaterial(const Vector3f& _emission, const Vector3f& d_color, const Vector3f& s_color, float s) :
        Material(MATERIAL_TYPE::PHONG_MATERIAL) {
        //id = material_cnt++;
        materials.push_back(this);
        //type = MATERIAL_TYPE::PHONG_MATERIAL;

        diffuseTexture2D = nullptr;
        normalTexture2D = nullptr;
        materialData.emission = _emission;
        materialData.diffuse = d_color;
        materialData.specular = s_color;
        materialData.shininess = s;
        materialData.transparent = 0.f;
        materialData.dissolve = 1.f;
    }

    PhongMaterial(const MaterialData& _materialData) : Material(MATERIAL_TYPE::PHONG_MATERIAL) {
        //id = material_cnt++;
        materials.push_back(this);
        //type = MATERIAL_TYPE::PHONG_MATERIAL;
        materialData = _materialData;
    }

    PhongMaterial(const MaterialData& _materialData, const std::string& texture_directory, Sampler2D::SAMPLER2D_TYPE _sampler2d_type) :
        Material(MATERIAL_TYPE::PHONG_MATERIAL) {
        //id = material_cnt++;
        materials.push_back(this);
        //type = MATERIAL_TYPE::PHONG_MATERIAL;
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

        const std::string path_to_emissiveTexture = texture_directory + materialData.emissive_texname;
        if (has_emissive_texture()) {
            if (Texture2D::texture2d_map.find(path_to_emissiveTexture) != Texture2D::texture2d_map.end()) {
                emissiveTexture2D = Texture2D::texture2d_map[path_to_emissiveTexture];
            }
            else {
                emissiveTexture2D = std::make_shared<Texture2D>(_sampler2d_type, path_to_emissiveTexture);
                Texture2D::texture2d_map[path_to_emissiveTexture] = emissiveTexture2D;
            }
        }
    }

    float pdf(const Vector3f& wi, const Vector3f& wo, const Vector3f& N, bool tag) const override {
        switch (renderer_sample_type) {
            // 独立实现
        case RENDERER_SAMPLE_TYPE::UNIFORM:
        case RENDERER_SAMPLE_TYPE::TRIVIAL_NEE:
            return M_1_2PI;
            break;

            // 独立实现
            // 参考：https://zhuanlan.zhihu.com/p/503163354
        default:
            return std::max(0.f, Vector3f::dot(wi, N)) * M_1_PI;
        }
    }

    float average_pdf(const Vector3f& wi, const Vector3f& wo, const Vector3f& N) const override {
        return pdf(wi, wo, N, false);
    }

    Vector3f sampleBSDF(const Vector3f& wo, const Vector3f& N, bool& tag) const override {
        Vector3f localRay;

        switch (renderer_sample_type) {
        case RENDERER_SAMPLE_TYPE::UNIFORM:
        case RENDERER_SAMPLE_TYPE::TRIVIAL_NEE: { // 参考已有代码：GAMES101作业框架
            float x_1 = get_random_float(), x_2 = get_random_float();
            float z = std::fabs(1.0f - 2.0f * x_1);
            float r = std::sqrt(1.0f - z * z), phi = 2 * M_PI * x_2;
            localRay = Vector3f(r * std::cos(phi), r * std::sin(phi), z); // z=cosθ，r=sinθ，localRay为在以N=(0,0,1)为法向的半球面上的wi

            break;
        }
        default: { // cos-weighted采样。参考已有代码：GAMES101作业框架。参考：https://zhuanlan.zhihu.com/p/503163354
            float x_1 = get_random_float(), x_2 = get_random_float();
            float z = std::sqrt(1 - x_1), r = std::sqrt(1 - z * z);
            float phi = M_2PI * x_2;
            localRay = Vector3f(r * std::cos(phi), r * std::sin(phi), z);
        }
        }

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

        return localRay.x() * B + localRay.y() * C + localRay.z() * N;
    }

    Vector3f evalBSDF(const Vector3f& wi, const Vector3f& wo, const Hit& h) const override {
        return Vector3f::dot(h.getFaceNormal(), wo) > 0.f ? getDiffuseColor(h) * M_1_PI : Vector3f::ZERO;
    }

    Vector3f evalBSDF_Whitted(const Vector3f& wi, const Vector3f& wo, const Hit& h) const override { // Whitted-style的BRDF，考虑了物体的高光
        Vector3f brdf = Vector3f::ZERO;
        Vector3f r = (2 * Vector3f::dot(h.getNormal(), wi) * h.getNormal() - wi).normalized();
        brdf += getDiffuseColor(h) * std::max(0.f, Vector3f::dot(wi, h.getNormal()));
        brdf += materialData.specular * std::pow(std::max(0.f, Vector3f::dot(wo, r)), materialData.shininess);

        return brdf;
    }
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
    GlossyMaterial(const Vector3f& _emission, const Vector3f& _albedo, float r, float m) :
        Material(Material::MATERIAL_TYPE::GLOSSY_MATERIAL) {
        //id = material_cnt++;
        materials.push_back(this);
        //type = MATERIAL_TYPE::GLOSSY_MATERIAL;

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

    float pdf(const Vector3f& wi, const Vector3f& wo, const Vector3f& N, bool tag) const override {
        switch (renderer_sample_type) {
            // 独立实现
        case RENDERER_SAMPLE_TYPE::UNIFORM:
        case RENDERER_SAMPLE_TYPE::TRIVIAL_NEE:
            return M_1_2PI;
            break;

            // 独立实现
            // 参考：https://zhuanlan.zhihu.com/p/503163354
        case RENDERER_SAMPLE_TYPE::COS_WEIGHTED:
            return std::max(0.f, Vector3f::dot(wi, N)) * M_1_PI;
            break;

        default: {
            if (tag) {
                Vector3f h = (wi + wo).normalized();
                float cos_h_n = Vector3f::dot(N, h); // 这里需要注意：根据采样的方法，cos_h_n可能<0，但此时wi必在宏观平面以下，从而渲染结果一定为黑色
                float cos_h_wi = Vector3f::dot(wi, h); // 同理
                if (cos_h_n <= 0.f || cos_h_wi == 0.f) {
                    return 1.f;
                }

                return GGX(cos_h_n) * cos_h_n * 0.25 / cos_h_wi;
            }
            else {
                return std::max(0.f, Vector3f::dot(wi, N)) * M_1_PI;
            }

            break;
        }
        }
    }

    float average_pdf(const Vector3f& wi, const Vector3f& wo, const Vector3f& N) const override {
        if (renderer_sample_type != RENDERER_SAMPLE_TYPE::MIS) {
            printf("ERROR: Render sample type must be MIS.\n");
            exit(1);
        }

        float metal_pdf = 0.f, diffuse_pdf = 0.f;

        Vector3f h = (wi + wo).normalized();
        float cos_h_n = Vector3f::dot(N, h);
        float cos_h_wi = Vector3f::dot(wi, h);
        if (cos_h_n <= 0.f || cos_h_wi == 0.f) {
            metal_pdf = 1.f;
        }
        else {
            metal_pdf = GGX(cos_h_n) * cos_h_n * 0.25 / cos_h_wi;
        }

        diffuse_pdf = std::max(0.f, Vector3f::dot(wi, N)) * M_1_PI;

        return materialData.metallic * metal_pdf + (1.f - materialData.metallic) * diffuse_pdf;
    }

    Vector3f sampleBSDF(const Vector3f& wo, const Vector3f& N, bool& tag) const override {
        Vector3f localRay;

        switch (renderer_sample_type) {
        case RENDERER_SAMPLE_TYPE::UNIFORM:
        case RENDERER_SAMPLE_TYPE::TRIVIAL_NEE: { // 参考已有代码：GAMES101作业框架 
            float x_1 = get_random_float(), x_2 = get_random_float();
            float z = std::fabs(1.0f - 2.0f * x_1);
            float r = std::sqrt(1.0f - z * z), phi = 2 * M_PI * x_2;
            localRay = Vector3f(r * std::cos(phi), r * std::sin(phi), z); // z=cosθ，r=sinθ，localRay为在以N=(0,0,1)为法向的半球面上的wi

            break;
        }
        case RENDERER_SAMPLE_TYPE::COS_WEIGHTED: { // 参考已有代码：GAMES101作业框架。参考：https://zhuanlan.zhihu.com/p/503163354
            float x_1 = get_random_float(), x_2 = get_random_float();
            float z = std::sqrt(1 - x_1), r = std::sqrt(1 - z * z);
            float phi = M_2PI * x_2;
            localRay = Vector3f(r * std::cos(phi), r * std::sin(phi), z);

            break;
        }
        case RENDERER_SAMPLE_TYPE::BRDF:
        case RENDERER_SAMPLE_TYPE::MIS: {
            float r1 = get_random_float();
            if (r1 <= materialData.metallic) {
                tag = true;
                float x_1 = get_random_float(), x_2 = get_random_float();
                float tan2theta = alpha2 * x_1 / (1 - x_1);
                float z = 1.f / std::sqrt(1.f + tan2theta), r = std::sqrt(1 - z * z);
                float phi = M_2PI * x_2;

                //Vector3f localH(r * std::cos(phi), r * std::sin(phi), z);
                localRay = Vector3f(r * std::cos(phi), r * std::sin(phi), z);
            }
            else {
                float x_1 = get_random_float(), x_2 = get_random_float();
                float z = std::sqrt(1 - x_1), r = std::sqrt(1 - z * z);
                float phi = M_2PI * x_2;
                localRay = Vector3f(r * std::cos(phi), r * std::sin(phi), z);
            }

            break;
        }
        }

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

        if (renderer_sample_type == RENDERER_SAMPLE_TYPE::BRDF || renderer_sample_type == RENDERER_SAMPLE_TYPE::MIS) {
            Vector3f H = localRay.x() * B + localRay.y() * C + localRay.z() * N;
            return (2 * H * Vector3f::dot(H, wo) - wo).normalized();
        }
        else {
            return localRay.x() * B + localRay.y() * C + localRay.z() * N;
        }
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
    ReflectiveMaterial(const Vector3f& _emission, const Vector3f& tf, float tr) : Material(Material::MATERIAL_TYPE::REFLECTIVE) {
        //id = material_cnt++;
        materials.push_back(this);
        //type = MATERIAL_TYPE::REFLECTIVE;

        diffuseTexture2D = nullptr;
        normalTexture2D = nullptr;
        materialData.emission = _emission;
        materialData.transmittance = tf;
        materialData.transparent = tr;
        materialData.dissolve = 1.f - tr;
    }

    float pdf(const Vector3f& wi, const Vector3f& wo, const Vector3f& N, bool tag) const override {
        return 1.f;
    }

    float average_pdf(const Vector3f& wi, const Vector3f& wo, const Vector3f& N) const override {
        return 1.f;
    }

    Vector3f sampleBSDF(const Vector3f& wo, const Vector3f& N, bool& tag) const override {
        return (2 * Vector3f::dot(wo, N) * N - wo).normalized();
    }

    Vector3f evalBSDF(const Vector3f& wi, const Vector3f& wo, const Hit& h) const override {
        return materialData.transmittance; // 此时一定没有纹理贴图
    }

    Vector3f evalBSDF_Whitted(const Vector3f& wi, const Vector3f& wo, const Hit& h) const override {
        return materialData.transmittance;
    }
};


// 独立实现
// 参考：课程内容
// 完美折射材质：仅发生折射或全反射
class RefractiveMaterial : public Material {
public:
    RefractiveMaterial(const Vector3f& _emission, const Vector3f& tf, float r, float tr) : Material(Material::MATERIAL_TYPE::REFRACTIVE) {
        //id = material_cnt++;
        materials.push_back(this);
        //type = MATERIAL_TYPE::REFRACTIVE;

        diffuseTexture2D = nullptr;
        normalTexture2D = nullptr;
        materialData.emission = _emission;
        materialData.transmittance = tf;
        materialData.ior = r;
        materialData.transparent = tr;
        materialData.dissolve = 1.f - tr;
    }

    float pdf(const Vector3f& wi, const Vector3f& wo, const Vector3f& N, bool tag) const override {
        return 1.f;
    }

    float average_pdf(const Vector3f& wi, const Vector3f& wo, const Vector3f& N) const override {
        return 1.f;
    }

    Vector3f sampleBSDF(const Vector3f& wo, const Vector3f& N, bool& tag) const override {
        Vector3f new_dir = Vector3f::ZERO;

        float cos_theta_I = Vector3f::dot(wo, N);
        //printf("[%d]\n", cos_theta_I);
        float inv_relative_refractive_index = cos_theta_I >= 0.f ? 1.f / materialData.ior : materialData.ior; // 出射介质相对入射介质的相对折射率的倒数，等于=ri_入射/ri_出射
        float temp = (1 - cos_theta_I * cos_theta_I) * inv_relative_refractive_index * inv_relative_refractive_index;

        if (temp >= 0.f && temp <= 1.f) { //发生折射
            float cos_theta_T = std::sqrt(1.f - temp);
            return ((inv_relative_refractive_index * std::fabs(cos_theta_I) - cos_theta_T) * N
                - inv_relative_refractive_index * wo).normalized();
        }

        //发生全反射
        return (2 * cos_theta_I * N - wo).normalized();
    }

    Vector3f evalBSDF(const Vector3f& wi, const Vector3f& wo, const Hit& h) const override {
        return materialData.transmittance; // 此时一定没有纹理贴图
    }

    Vector3f evalBSDF_Whitted(const Vector3f& wi, const Vector3f& wo, const Hit& h) const override {
        return materialData.transmittance;
    }
};

#endif // MATERIAL_H
