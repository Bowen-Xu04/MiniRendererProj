// 独立实现
#ifndef PLANE_H
#define PLANE_H

#include "utils.hpp"
#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// : Implement Plane representing an infinite plane
// function: ax+by+cz=d
// choose your representation , add more fields and fill in the functions

class Plane : public Object3D {
public:
    Vector3f normal;
    float d;

    Plane() = delete;

    Plane(const Vector3f& _normal, float _d, Material* m) : Object3D(m), normal(_normal.normalized()), d(_d) {
        id = primitive_cnt++;
        type = OBJECT_TYPE::PLANE;
        box = calculate_box();
    }

    ~Plane() override {
        primitive_cnt--;
    }

    bool intersect(const Ray& r, Hit& h) override {
        float nd = Vector3f::dot(normal, r.getDirection());
        if (fabs(nd) < EPI) return false;

        float t = (d - Vector3f::dot(normal, r.getOrigin())) / nd;
        if (t >= EPI) {
            if (t <= h.getT()) {
                h.set(id, t, material, r.pointAtParameter(t), normal);
            }
            return true;
        }
        return false;
    }

    AABB calculate_box() override {
        if (normal.y() == 0.f && normal.z() == 0.f) {
            return AABB({ d / normal.x(),-INF,-INF }, { d / normal.x(),INF,INF });
        }
        if (normal.x() == 0.f && normal.z() == 0.f) {
            return AABB({ -INF,d / normal.y(), -INF }, { INF,d / normal.y(),INF });
        }
        if (normal.x() == 0.f && normal.y() == 0.f) {
            return AABB({ -INF,-INF,d / normal.z() }, { INF,INF,d / normal.z() });
        }
        return AABB({ -INF,-INF,-INF }, { INF,INF,INF });
    }

protected:


};

#endif //PLANE_H


