// 独立实现
#ifndef AABB_H
#define AABB_H

#include <vecmath.h>
#include "ray.hpp"
#include "hit.hpp"

class Object3D;

class AABB {
private:
    //using Point3 = Vector3f;
    Vector3f pMin, pMax, center;
    //Object3D* object;
public:
    AABB() {}

    AABB(Vector3f _pMin, Vector3f _pMax) : pMin(_pMin), pMax(_pMax), center((_pMin + _pMax) * 0.5f) {
        //type = OBJECT_TYPE::BOUNDING_BOX;
    }

    ~AABB() {}

    bool intersect(const Ray& r) {
        //printf("^^^\n");
        float tx1 = (pMin.x() - r.getOrigin().x()) * r.getInv_Direction().x(), tx2 = (pMax.x() - r.getOrigin().x()) * r.getInv_Direction().x();
        float ty1 = (pMin.y() - r.getOrigin().y()) * r.getInv_Direction().y(), ty2 = (pMax.y() - r.getOrigin().y()) * r.getInv_Direction().y();
        float tz1 = (pMin.z() - r.getOrigin().z()) * r.getInv_Direction().z(), tz2 = (pMax.z() - r.getOrigin().z()) * r.getInv_Direction().z();
        float Tx1 = std::min(tx1, tx2), Tx2 = std::max(tx1, tx2);
        float Ty1 = std::min(ty1, ty2), Ty2 = std::max(ty1, ty2);
        float Tz1 = std::min(tz1, tz2), Tz2 = std::max(tz1, tz2);
        float tenter = std::max(Tx1, std::max(Ty1, Tz1));
        float texit = std::min(Tx2, std::min(Ty2, Tz2));

        if (tenter <= texit + EPI && texit > EPI) {
            return true;
        }
        return false;
    }

    // bool intersect_object(const Ray& r, Hit& h, float tmin) {
    //     assert(object != nullptr);
    // }

    Vector3f get_center() {
        return center;
    }

    Vector3f get_pMin() {
        return pMin;
    }

    Vector3f get_pMax() {
        return pMax;
    }

    // AABB get_box() {
    //     return *this;
    // }

    // void bind_object(Object3D* _object) {
    //     object = _object;
    // }

    friend class BVH;
};

#endif