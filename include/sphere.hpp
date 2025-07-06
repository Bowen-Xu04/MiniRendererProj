// 独立实现，除了sample函数参考了GAMES101作业框架
#ifndef SPHERE_H
#define SPHERE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>


class Sphere : public Object3D {
public:
    Vector3f center;
    float radius;

    Sphere() : center(Vector3f(0.0)), radius(1.0) { // unit ball at the center
        id = primitive_cnt++;
        type = OBJECT_TYPE::SPHERE;
        //objects.push_back(this);
        box = calculate_box();
        area = calculate_area();
    }

    Sphere(const Vector3f& _center, float _radius, Material* material) : Object3D(material), center(_center), radius(_radius) {
        id = primitive_cnt++;
        type = OBJECT_TYPE::SPHERE;
        //objects.push_back(this);
        box = calculate_box();
        area = calculate_area();
    }

    ~Sphere() override {
        primitive_cnt--;
        //objects.erase(objects.begin() + id);
    }

    float calculate_area() {
        return 4 * M_PI * radius * radius;
    }

    bool intersect(const Ray& r, Hit& h) override {
        float b = 2 * Vector3f::dot(r.getDirection(), r.getOrigin() - center);
        float c = (r.getOrigin() - center).squaredLength() - radius * radius;

        float delta = b * b - 4 * r.getDirection().squaredLength() * c;
        if (delta < 0) return false;

        float sqrdelta = sqrt(delta);
        float t1 = (-b - sqrdelta) / (2 * r.getDirection().squaredLength()), t2 = (-b + sqrdelta) / (2 * r.getDirection().squaredLength());
        if (t2 < EPI) return false;

        float t = (t1 < EPI ? t2 : t1);
        if (t <= h.getT()) {
            h.set(id, t, material, r.pointAtParameter(t), (r.pointAtParameter(t) - center).normalized());
        }

        return true;
    }

    AABB calculate_box() override {
        return AABB({ center.x() - radius,center.y() - radius ,center.z() - radius },
            { center.x() + radius,center.y() + radius ,center.z() + radius });
    }

    // 参考已有代码：GAMES101作业框架
    void sample(Hit& h) override {
        float x_1 = get_random_float(), x_2 = get_random_float();
        float z = 1.0f - 2.0f * x_1;
        float r = std::sqrt(1.0f - z * z), phi = 2 * M_PI * x_2;
        Vector3f point(r * std::cos(phi), r * std::sin(phi), z);

        h.set(id, MAXT, material, point, (point - center).normalized());
    }

};


#endif
