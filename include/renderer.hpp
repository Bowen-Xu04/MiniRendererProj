// 独立实现
// 参考：GAMES101课件
#ifndef RENDERER_H
#define RENDERER_H

#include <iostream>
#include <vector>
#include <chrono>
#include <vecmath.h>

#include "ray.hpp"
#include "light.hpp"
#include "glsl.hpp"
#include "scene_parser.hpp"
#include "image.hpp"

class Renderer {
protected:
    std::vector<Vector3f> framebuffer;
    SceneParser* scene;
    Group* baseGroup;
    Camera* camera;

    int MSAA;
    float invMSAA;
    std::vector<Vector2f> MSAA_grid;
    //std::vector<int> sampledID;

public:
    Renderer() {}
    virtual ~Renderer() = default;

    void calculate_MSAA_grid() {
        MSAA_grid.clear();
        int size = std::sqrt(MSAA);
        float length = 1.0 / (2 * size);

        for (int i = 0;i < size;i++) {
            for (int j = 0;j < size;j++) {
                MSAA_grid.push_back(Vector2f((2 * i + 1) * length, (2 * j + 1) * length));
            }
        }
    }

    virtual void render(SceneParser& _scene, Image& img) = 0;
    virtual Vector3f castRay(const Ray& r, Hit& h, int depth, bool calc_light) { return Vector3f::ZERO; };
};

class WhittedStyleRenderer : public Renderer {
private:
    int MAX_LIGHT_BOUNCE = 10;

public:
    WhittedStyleRenderer(int _MSAA) {
        MSAA = _MSAA;
        invMSAA = 1.0 / MSAA;
        calculate_MSAA_grid();
    }

    void render(SceneParser& _scene, Image& img) override {
        printf("Rendering... (Progress may not be updated in real-time)\n");

        scene = &_scene;
        baseGroup = scene->getGroup();
        camera = scene->getCamera();

        // if (MSAA * camera->getWidth() * camera->getHeight() > (1 << 30)) {
        //     printf("ERROR: Too large memory cost.\n");
        //     exit(1);
        // }
        //printf(".\n");
        framebuffer.resize(camera->getWidth() * camera->getHeight());
        //printf(".\n");
        //sampledID.resize(MSAA * camera->getWidth() * camera->getHeight());
        //float invHeight = 1.f / camera->getHeight(); // invTotal = 1.0 / (camera->getHeight() * camera->getWidth());

        auto start = std::chrono::system_clock::now();
        int progress = 0, percent = 0, total = camera->getHeight() * camera->getWidth();

        UpdateProgress(0);
        std::cout << std::flush;

#ifdef OMP_ACCELERATION
#pragma omp parallel for num_threads(MAX_THREAD_NUM) shared(progress)
#endif
        for (int index = 0;index < total;++index) {
            int i = index % camera->getWidth(), j = index / camera->getWidth();

            if (MSAA == 1) {
                Ray camRay = camera->generateRay(Vector2f(i + 0.5, j + 0.5));
                Hit hit;

                framebuffer[index] = castRay(camRay, hit, 0, true);
            }
            else {
                std::unordered_map<int, std::pair<int, int>> ID_cnt;
                ID_cnt.clear();

                for (int k = 0;k < MSAA;++k) {
                    Ray camRay = camera->generateRay(Vector2f(i, j) + MSAA_grid[k]);
                    Hit hit;

                    castRay(camRay, hit, 0, false);

                    if (ID_cnt.find(hit.get_id()) != ID_cnt.end()) {
                        ++ID_cnt[hit.get_id()].first;
                    }
                    else {
                        ID_cnt[hit.get_id()] = std::make_pair(1, k);
                    }
                }

                framebuffer[index] = Vector3f::ZERO;
                for (const std::pair<int, std::pair<int, int>>& kv : ID_cnt) {
                    Ray camRay = camera->generateRay(Vector2f(i, j) + MSAA_grid[kv.second.second]);
                    Hit hit;

                    framebuffer[index] +=
                        (kv.first == -1 ? scene->getBackgroundColor() : castRay(camRay, hit, 0, true)) * kv.second.first;
                }
                framebuffer[index] *= invMSAA;
            }

            //    bool isIntersect = baseGroup->intersect(camRay, hit);
            // if (isIntersect) {
            //     framebuffer
            // }

            gamma_correction(framebuffer[index]);
            img.SetPixel(i, j, framebuffer[index]);

            // if (0) {
            //     printf("[%d,%d]", i, j);
            //     printf("(%f,%f,%f)\n", framebuffer[index].x(), framebuffer[index].y(), framebuffer[index].z());
            // }
            //}

#ifdef OMP_ACCELERATION
#pragma omp atomic
#endif
            ++progress;

#ifdef OMP_ACCELERATION
#pragma omp critical
            {
                if (omp_get_thread_num() == CHECK_PROGRESS_THREAD_INDEX) {
                    int now_percent = progress * 100 / total;
                    if (now_percent > percent) {
                        percent = now_percent;
                        UpdateProgress(percent);
                    }
                }
            }
#else
            int now_percent = progress * 100 / total;
            if (now_percent > percent) {
                percent = now_percent;
                UpdateProgress(percent);
            }
#endif
        }

        UpdateProgress(100);

        auto end = std::chrono::system_clock::now();
        int time_cost = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << std::endl << "Whitted-style rendering done. Time: " << (float)time_cost / 1000 << "s." << std::endl;
    }

    Vector3f castRay(const Ray& r, Hit& h, int depth, bool calc_light) override {
        //assert(!baseGroup->get_tlas().empty());
        //return Vector3f::ZERO;
        //printf("BEGIN\n");
        bool isIntersect = baseGroup->intersect(r, h);

        if (!calc_light) {
            return Vector3f::ZERO;
        }
        //printf("h\n");
        if (isIntersect) {
            //printf("[%d %d]\n", h.get_id(), h.get_mesh_id());
            // if ()) {
            //     printf("!!!\n");
            // }
            Vector3f finalColor = Vector3f::ZERO;
            Vector3f point = h.getPoint();
            Material* material = h.getMaterial();
            // if (h.get_id() == 0) {
            //     printf("!!! %d\n", material != nullptr);
            // }

            //std::cout << material->get_type() << std::endl;
            // if (material == nullptr) {
            //     printf("!!!!!\n");
            // }
            switch (material->get_type()) {
            case Material::MATERIAL_TYPE::PHONG_MATERIAL: {
                //if (h.get_id() == 0) { printf("PHONG\n"); }
                for (int li = 0;li < scene->getNumLights();++li) {
                    //printf("(%d) 1\n", li);
                    Light* light = scene->getLight(li);
                    //printf("[%d,%d,%d]\n", li, light != nullptr, h.get_id());
                    Vector3f L, lightColor;
                    float t;
                    light->getIllumination(point, L, lightColor, t);
                    //printf("[%d]\n", li);
                    Hit new_hit;
                    //printf("(%d) 2\n", li);
                    // if (h.get_id() == 0) {
                    //     printf("???\n");
                    // }
                    //printf("new_inter beg\n");
                    bool new_intersect = baseGroup->intersect(Ray(point, L), new_hit);
                    //printf("new_inter end\n");
                    //printf("(%d) 2_1\n", li);
                    if ((!new_intersect) || (new_intersect && new_hit.getT() >= t - EPI)) { // 处理阴影
                        //printf("!!!\n");
                        // if (h.get_id() == 0) {
                        //     printf("...\n");
                        // }

                        //if (depth == 0) {
                        //if (material->has_diffuse_texture()) { // 此时交点一定位于三角网格上，从而材质一定为PhongMaterial
                        //printf("(%d) 3\n", li);
                        finalColor += lightColor * material->evalBSDF_Whitted(L, -r.getDirection(), h);
                        // if (material->has_diffuse_texture()) {
                        //     printvec3(lightColor);
                        //     printvec3(finalColor);
                        //     printf("\n");
                        // }
                        //printf("(%d) 4\n", li);
                        // }
                        // else {
                        //     finalColor += lightColor * material->evalBRDF_Whitted(L, -r.getDirection(), h.getNormal());
                        // }

                        // printf("r.dir: "); printvec3(r.getDirection());
                        // printf("hit.point: "); printvec3(point);
                        // printf("hit.normal: "); printvec3(h.getNormal());
                        // printf("dirToLight: "); printvec3(L);
                        // printf("F: "); printvec3(finalColor);
                        // printf("\n");
                        //}
                        //std::cout << finalColor.xyz() << std::endl;
                        //printf("(%f,%f,%f)\n", finalColor.x(), finalColor.y(), finalColor.z());
                    }
                    // if (h.get_id() == 0) {
                    //     printf("???\n");
                    // }
                    //return h.getMaterial()->Shade(r, h);
                    //printf("(%d) 4\n", li);
                }
                finalColor += material->getEmission();

                break;
            }
            case Material::MATERIAL_TYPE::REFLECTIVE:
            case Material::MATERIAL_TYPE::REFRACTIVE:
            {
                // if (h.get_mesh_id() != -1) {
                //     printf("[%d] hhh1\n", h.get_mesh_id());
                // }
                //ReflectiveMaterial* trans_m = dynamic_cast<ReflectiveMaterial*>(material);
                Hit new_hit;
                Vector3f wi = material->sampleBSDF(-r.getDirection(), h.getNormal());

                Ray new_ray(h.getPoint(), wi);
                // if (h.get_mesh_id() != -1) {
                //     printf("hhh2\n");
                // }
                if (depth < MAX_LIGHT_BOUNCE) {
                    //Vector3f new_color = castRay(new_ray, new_hit, depth + 1);
                    // 此时一定没有纹理贴图
                    finalColor = castRay(new_ray, new_hit, depth + 1, true) * material->evalBSDF_Whitted(wi, -r.getDirection(), h);

                    // printf("(%f,%f,%f) (%f,%f,%f)\n", new_ray.getOrigin().x(), new_ray.getOrigin().y(), new_ray.getOrigin().z(),
                    //     new_ray.getDirection().x(), new_ray.getDirection().y(), new_ray.getDirection().z());
                    // printf("[%d](%f) ", depth, new_hit.getT()); printvec3(new_color); printvec3(finalColor); printf("\n");
                }
                // if (h.get_mesh_id() != -1) {
                //     printf("hhh3\n");
                // }
                finalColor += material->getEmission();
                // if (h.get_mesh_id() != -1) {
                //     printf("hhh4\n");
                // }
                break;
            }
            // case Material::MATERIAL_TYPE::REFRACTIVE: {
            //     //RefractiveMaterial* trans_m = dynamic_cast<RefractiveMaterial*>(material);
            //     Hit new_hit;
            //     Ray new_ray(h.getPoint(), material->sampleBRDF(-r.getOrigin(), h.getNormal()));
            //     if (depth < MAX_LIGHT_BOUNCE) {
            //         //Vector3f new_color = ;
            //         finalColor = material->get_decay_rate() * material->getDiffuseColor() * castRay(new_ray, new_hit, depth + 1, true);
            //         // printf("(%f,%f,%f) (%f,%f,%f)\n", new_ray.getOrigin().x(), new_ray.getOrigin().y(), new_ray.getOrigin().z(),
            //         //     new_ray.getDirection().x(), new_ray.getDirection().y(), new_ray.getDirection().z());
            //         // printf("[%d](%f) ", depth, new_hit.getT()); printvec3(new_color); printvec3(finalColor); printf("\n");
            //     }
            //     finalColor += material->getAmbientColor() + material->getEmission();
            //     break;
            // }
            default: {
                printf("ERROR: Unsupported material type.\n");
                exit(1);
            }
            }
            // if (h.get_id() == 0) {
            //     printf("end\n");
            // }
            return finalColor;
        }
        else {
            return scene->getBackgroundColor();
        }
    }
};

class PathTracer_CPU : public Renderer {
private:
    bool NEE; // 是否专门对光源进行采样
    int spp;
    float inv_spp, RR, invRR;
public:
    PathTracer_CPU(int _spp, int _MSAA, bool _NEE, float _RR) :
        spp(_spp), NEE(_NEE), RR(_RR), invRR(1.0 / _RR) {
        MSAA = _MSAA;
        invMSAA = 1.0 / MSAA;
        inv_spp = 1.0 / spp;
        calculate_MSAA_grid();
    }

    void render(SceneParser& _scene, Image& img) override {
        printf("Rendering... (Progress may not be updated in real-time)\n");

        scene = &_scene;
        baseGroup = scene->getGroup();
        camera = scene->getCamera();

        framebuffer.resize(camera->getWidth() * camera->getHeight());

        auto start = std::chrono::system_clock::now();
        int progress = 0, percent = 0, total = camera->getHeight() * camera->getWidth();

        UpdateProgress(0);
        std::cout << std::flush;

#ifdef OMP_ACCELERATION
#pragma omp parallel for num_threads(MAX_THREAD_NUM) shared(progress)
#endif
        for (int index = 0;index < total;++index) {
            //for (int i = 0;i < camera->getWidth();++i) {
                //int index = j * camera->getWidth() + i;
            int i = index % camera->getWidth(), j = index / camera->getWidth();
#ifdef HIT_DATA
            printf("[%d,%d]\n", i, camera->getHeight() - 1 - j);
#endif
            framebuffer[index] = Vector3f::ZERO;

            if (MSAA == 1) {
                Ray camRay = camera->generateRay(Vector2f(i + 0.5, j + 0.5));
                Hit hit;
#ifdef HIT_DATA
                printf("SPP: 0\n");
                Vector3f col = castRay(camRay, hit, 0, true);
                printvec3(col);
                printf("\n");
                framebuffer[index] += col;
#else
                framebuffer[index] += castRay(camRay, hit, 0, true);
#endif

                if (hit.happened()) {
                    for (int s = 1;s < spp;++s) {
#ifdef HIT_DATA
                        printf("SPP: %d\n", s);
                        col = castRay(camRay, hit, 0, true);
                        printvec3(col);
                        printf("\n");
                        framebuffer[index] += col;
#else
                        framebuffer[index] += castRay(camRay, hit, 0, true);
#endif
                    }
                }
                else {
                    framebuffer[index] *= spp;
                }
            }
            else {
                std::unordered_map<int, std::pair<int, int>> ID_cnt;
                ID_cnt.clear();

                for (int k = 0;k < MSAA;++k) {
                    Ray camRay = camera->generateRay(Vector2f(i, j) + MSAA_grid[k]);
                    Hit hit;

                    castRay(camRay, hit, 0, false);

                    if (ID_cnt.find(hit.get_id()) != ID_cnt.end()) {
                        ++ID_cnt[hit.get_id()].first;
                    }
                    else {
                        //printf("%d ", hit.get_id());
                        ID_cnt[hit.get_id()] = std::make_pair(1, k);
                    }
                }
                //printf("[%d,%d] %f ", i, camera->getHeight() - 1 - j);
                for (const std::pair<int, std::pair<int, int>>& kv : ID_cnt) {
                    //printf("(%d)(%d,%d) ", kv.first, kv.second.first, kv.second.second);
                    Ray camRay = camera->generateRay(Vector2f(i, j) + MSAA_grid[kv.second.second]);
                    Hit hit;

                    if (kv.first == -1) {
                        framebuffer[index] += scene->getBackgroundColor() * kv.second.first;
                    }
                    else {
                        Vector3f thisColor(0.f);
                        for (int s = 0;s < spp;++s) {
                            thisColor += castRay(camRay, hit, 0, true);
                        }
                        framebuffer[index] += thisColor * kv.second.first;
                    }
                }
                //printf("\n");
                framebuffer[index] *= invMSAA;
            }

            framebuffer[index] *= inv_spp;
            //printvec3(framebuffer[index]); printf("\n");
#ifndef HIT_DATA
            gamma_correction(framebuffer[index]);
#endif
            img.SetPixel(i, j, framebuffer[index]);
#ifdef HIT_DATA
            printvec3(framebuffer[index]);
            //++m;
            printf("\n================================\n");
#endif
            // if (1) {
            //     printf("[%d,%d]", i, j);
            //     printf("(%f,%f,%f)\n", framebuffer[index].x(), framebuffer[index].y(), framebuffer[index].z());
            // }
            //}

#ifdef OMP_ACCELERATION
#pragma omp atomic
#endif
            ++progress;

#ifdef OMP_ACCELERATION
#pragma omp critical
            {
                if (omp_get_thread_num() == CHECK_PROGRESS_THREAD_INDEX) {
                    int now_percent = progress * 100 / total;
                    if (now_percent > percent) {
                        percent = now_percent;
                        UpdateProgress(percent);
                    }
                }
            }
#else
            int now_percent = progress * 100 / total;
            if (now_percent > percent) {
                percent = now_percent;
                UpdateProgress(percent);
            }
#endif
        }

        UpdateProgress(100);

        auto end = std::chrono::system_clock::now();
        int time_cost = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << std::endl << "Pathtracing (CPU) rendering done. Time: " << (float)time_cost / 1000 << "s." << std::endl;
    }

    Vector3f castRay(const Ray& r, Hit& h, int depth, bool calc_light) override {
        // if (depth >= 2) {
        //     return Vector3f::ZERO;
        // }
        // 每次castRay：
        // 1. 从当前光线出发，计算与场景的第一个交点
        // 2. 在交点处启用NEE，先对光源进行采样，之后，如果RR判定成功，则计算下一根光线，递归调用castRay
        //assert(!baseGroup->get_tlas().empty());
        bool isIntersect = baseGroup->intersect(r, h);

        if (!calc_light) {
            return Vector3f::ZERO;
        }
#ifdef HIT_DATA
        for (int i = 0;i < depth;++i) {
            printf("    ");
        }
        printf("%d ", h.get_id());
#endif
        if (isIntersect) {
            //return 0.5 * (h.getFaceNormal() + Vector3f(1.0, 1.0, 1.0));
            Vector3f finalColor = Vector3f::ZERO;
            //Vector3f point = h.getPoint();
            //Material* material = h.getMaterial();

            switch (h.getMaterial()->get_type()) {
            case Material::MATERIAL_TYPE::PHONG_MATERIAL:
            case Material::MATERIAL_TYPE::GLOSSY_MATERIAL:
            {
                if (NEE) {
                    Vector3f L_dir = Vector3f::ZERO, L_indir = Vector3f::ZERO;
                    //printf("%f\n", scene->get_light_area());
                    if (scene->get_light_area() > 0.f) {
                        Hit new_hit;
                        scene->getGroup()->sample_on_light(new_hit);

                        Vector3f wi = (new_hit.getPoint() - h.getPoint()).normalized();
                        float costheta = Vector3f::dot(h.getNormal(), wi);
                        //printf("???\n");
                        if (costheta >= 0.f) {
                            Hit test_hit;
                            castRay(Ray(h.getPoint(), wi), test_hit, depth + 1, false);
                            // printvec3(new_hit.getPoint());
                            // printvec3(test_hit.getPoint());
                            // printf("\n");
                            if ((new_hit.getPoint() - test_hit.getPoint()).length() < EPI && (new_hit.getPoint() - h.getPoint()).length() >= 0.05) {
#ifdef HIT_DATA
                                printf("L(%d,%f)\n", new_hit.get_id(), (new_hit.getPoint() - h.getPoint()).length());
#endif
                                // printvec3(new_hit.getMaterial()->getEmission());
                                // printf("\n");
                                // if ((new_hit.getPoint() - h.getPoint()).squaredLength() < 0.01) {
                                //     printf("!!!\n");
                                // }

                                L_dir = scene->get_light_area() * costheta * std::fabs(Vector3f::dot(new_hit.getNormal(), wi)) / (new_hit.getPoint() - h.getPoint()).squaredLength()
                                    * (new_hit.getMaterial()->getEmission() * h.getMaterial()->evalBSDF(wi, -r.getDirection(), h));
                            }
#ifdef HIT_DATA
                            else {
                                printf("Lmiss\n");
                            }
#endif
                        }
                    }

                    if (get_random_float() < RR) {
                        Vector3f wi = h.getMaterial()->sampleBSDF(-r.getDirection(), h.getNormal());
                        float costheta = Vector3f::dot(h.getNormal(), wi); // sample出来的wi能保证costheta>=0

                        Ray new_ray(h.getPoint(), wi);
                        Hit test_hit;
                        castRay(new_ray, test_hit, depth + 1, false);

                        if (test_hit.happened() && Vector3f::dot(h.getFaceNormal(), wi) >= 0.f) {
                            if (!test_hit.getMaterial()->hasEmission()) {
                                Hit new_hit;
                                L_indir = invRR * costheta * (castRay(new_ray, new_hit, depth + 1, true) * h.getMaterial()->evalBSDF(wi, -r.getDirection(), h)) / h.getMaterial()->pdf(wi, h.getNormal());
                            }
                        }
                        else {
                            L_indir = scene->getBackgroundColor();
                            //invRR * M_2PI * costheta * (scene->getBackgroundColor() * h.getMaterial()->evalBSDF(wi, -r.getDirection(), h));
                        }
                    }

                    return h.getMaterial()->getEmission() + L_dir + L_indir;
                }
                else {
                    //Vector3f L = ;
                    if (get_random_float() < RR) {
                        Vector3f wi = h.getMaterial()->sampleBSDF(-r.getDirection(), h.getNormal());
                        float costheta = Vector3f::dot(h.getNormal(), wi); // sample出来的wi能保证costheta>=0

                        //Ray new_ray;
                        Hit new_hit;
                        //castRay(new_ray, test_hit, depth + 1, false);

                        if (Vector3f::dot(h.getFaceNormal(), wi) >= 0.f) {
                            return h.getMaterial()->getEmission() +
                                invRR * costheta * (castRay(Ray(h.getPoint(), wi), new_hit, depth + 1, true) * h.getMaterial()->evalBSDF(wi, -r.getDirection(), h)) / h.getMaterial()->pdf(wi, h.getNormal());
                        }
                        // }
                        // else {
                        //     L = invRR * M_2PI * costheta * (scene->getBackgroundColor() * h.getMaterial()->evalBRDF(wi, -r.getDirection(), h.getNormal()));
                        // }
                    }

                    return h.getMaterial()->getEmission();
                }

                break;
            }
            case Material::MATERIAL_TYPE::REFLECTIVE:
            case Material::MATERIAL_TYPE::REFRACTIVE: { // 对于这两种材质，BSDF形如一个dirac δ函数，因此无需考虑NEE，直接进行反射/折射的光线计算即可
                if (get_random_float() < RR) {
                    Vector3f wi = h.getMaterial()->sampleBSDF(-r.getDirection(), h.getNormal());
                    float costheta = Vector3f::dot(h.getNormal(), wi); // costheta可能<0，因此最后需要将其绝对值代入计算公式

                    //Ray new_ray;
                    Hit new_hit;
                    return h.getMaterial()->getEmission() + invRR * std::fabs(costheta) * (castRay(Ray(h.getPoint(), wi), new_hit, depth + 1, true) * h.getMaterial()->evalBSDF(wi, -r.getDirection(), h)) / h.getMaterial()->pdf(wi, h.getNormal());
                }

                return h.getMaterial()->getEmission();
                break;
            }
            default: {
                printf("ERROR: Unsupported material type.\n");
                exit(1);
            }
            }
        }
        else {
            return scene->getBackgroundColor();
        }
    }
};

class PathTracer_GPU : public Renderer {
private:
    // int width, height;
    // std::string name;
    GLSL_Window* glsl_window;

    // bool NEE;
    // int spp;
    // float inv_spp, RR, invRR;

public:
    PathTracer_GPU(int _spp, int _MSAA, bool _usingAS, bool _NEE, float _RR, int width, int height, std::string name) :
        glsl_window(new GLSL_Window(width, height, _spp, _usingAS, _NEE, _RR, name)) {
        // MSAA = _MSAA;
        // invMSAA = 1.0 / MSAA;
        // inv_spp = 1.0 / spp;
        // calculate_MSAA_grid();
    }

    ~PathTracer_GPU() {
        if (glsl_window != nullptr) {
            delete glsl_window;
        }
    }

    void render(SceneParser& scene, Image& img) override {
        if (glsl_window == nullptr) {
            printf("ERROR: No OpenGL window.\n");
            exit(1);
        }

        //glsl_window->test();
        glsl_window->run(scene, img);
    }
    //virtual Vector3f castRay() {};
};

#endif