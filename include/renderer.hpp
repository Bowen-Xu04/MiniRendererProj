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

        framebuffer.resize(camera->getWidth() * camera->getHeight());

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

            gamma_correction(framebuffer[index]);
            img.SetPixel(i, j, framebuffer[index]);

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
        bool isIntersect = baseGroup->intersect(r, h);

        if (!calc_light) {
            return Vector3f::ZERO;
        }

        if (isIntersect) {
            Vector3f finalColor = Vector3f::ZERO;
            Vector3f point = h.getPoint();
            Material* material = h.getMaterial();

            switch (material->get_type()) {
            case Material::MATERIAL_TYPE::PHONG_MATERIAL: {
                for (int li = 0;li < scene->getNumLights();++li) {
                    Light* light = scene->getLight(li);
                    Vector3f L, lightColor;
                    float t;
                    light->getIllumination(point, L, lightColor, t);

                    Hit new_hit;
                    bool new_intersect = baseGroup->intersect(Ray(point, L), new_hit);

                    if ((!new_intersect) || (new_intersect && new_hit.getT() >= t - EPI)) { // 处理阴影
                        finalColor += lightColor * material->evalBSDF_Whitted(L, -r.getDirection(), h);
                    }
                }
                finalColor += material->getEmission();

                break;
            }
            case Material::MATERIAL_TYPE::REFLECTIVE:
            case Material::MATERIAL_TYPE::REFRACTIVE: {
                Hit new_hit;
                Vector3f wi = material->sampleBSDF(-r.getDirection(), h.getNormal());
                Ray new_ray(h.getPoint(), wi);

                if (depth < MAX_LIGHT_BOUNCE) {
                    // 此时一定没有纹理贴图
                    finalColor = castRay(new_ray, new_hit, depth + 1, true) * material->evalBSDF_Whitted(wi, -r.getDirection(), h);
                }
                finalColor += material->getEmission();

                break;
            }
            default: {
                printf("ERROR: Unsupported material type.\n");
                exit(1);
            }
            }

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
                        ID_cnt[hit.get_id()] = std::make_pair(1, k);
                    }
                }

                for (const std::pair<int, std::pair<int, int>>& kv : ID_cnt) {
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

                framebuffer[index] *= invMSAA;
            }

            framebuffer[index] *= inv_spp;
#ifndef HIT_DATA
            gamma_correction(framebuffer[index]);
#endif
            img.SetPixel(i, j, framebuffer[index]);
#ifdef HIT_DATA
            printvec3(framebuffer[index]);
            //++m;
            printf("\n================================\n");
#endif

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
        // 每次castRay：
        // 1. 从当前光线出发，计算与场景的第一个交点
        // 2. 在交点处启用NEE，先对光源进行采样，之后，如果RR判定成功，则计算下一根光线，递归调用castRay
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
            Vector3f finalColor = Vector3f::ZERO;

            switch (h.getMaterial()->get_type()) {
            case Material::MATERIAL_TYPE::PHONG_MATERIAL:
            case Material::MATERIAL_TYPE::GLOSSY_MATERIAL: {
                if (NEE) {
                    Vector3f L_dir = Vector3f::ZERO, L_indir = Vector3f::ZERO;

                    if (scene->get_light_area() > 0.f) {
                        Hit new_hit;
                        scene->getGroup()->sample_on_light(new_hit);

                        Vector3f wi = (new_hit.getPoint() - h.getPoint()).normalized();
                        float costheta = Vector3f::dot(h.getNormal(), wi);

                        if (costheta >= 0.f) {
                            Hit test_hit;
                            castRay(Ray(h.getPoint(), wi), test_hit, depth + 1, false);

                            if ((new_hit.getPoint() - test_hit.getPoint()).length() < EPI && (new_hit.getPoint() - h.getPoint()).length() >= 0.05) {
#ifdef HIT_DATA
                                printf("L(%d,%f)\n", new_hit.get_id(), (new_hit.getPoint() - h.getPoint()).length());
#endif
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
                        }
                    }

                    return h.getMaterial()->getEmission() + L_dir + L_indir;
                }
                else {
                    if (get_random_float() < RR) {
                        Vector3f wi = h.getMaterial()->sampleBSDF(-r.getDirection(), h.getNormal());
                        float costheta = Vector3f::dot(h.getNormal(), wi); // sample出来的wi能保证costheta>=0
                        Hit new_hit;

                        if (Vector3f::dot(h.getFaceNormal(), wi) >= 0.f) {
                            return h.getMaterial()->getEmission() +
                                invRR * costheta * (castRay(Ray(h.getPoint(), wi), new_hit, depth + 1, true) * h.getMaterial()->evalBSDF(wi, -r.getDirection(), h)) / h.getMaterial()->pdf(wi, h.getNormal());
                        }
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
    GLSL_Window* glsl_window;

public:
    PathTracer_GPU(int _spp, int _MSAA, bool _usingAS, bool _NEE, float _RR, int width, int height, std::string name) :
        glsl_window(new GLSL_Window(width, height, _spp, _usingAS, _NEE, _RR, name)) {
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

        glsl_window->run(scene, img);
    }
};

#endif