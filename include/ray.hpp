// 独立实现
#ifndef RAY_H
#define RAY_H

#include <iostream>
#include <Vector3f.h>
#include "utils.hpp"

// Ray class mostly copied from Peter Shirley and Keith Morley
class Ray {
public:

    Ray() = delete;
    Ray(const Vector3f& orig, const Vector3f& dir) {
        origin = orig;
        direction = dir;
        inv_direction = Vector3f(1.f / dir.x(), 1.f / dir.y(), 1.f / dir.z());
    }

    Ray(const Ray& r) {
        origin = r.origin;
        direction = r.direction;
    }

    const Vector3f& getOrigin() const {
        return origin;
    }

    const Vector3f& getDirection() const {
        return direction;
    }

    const Vector3f& getInv_Direction() const {
        return inv_direction;
    }

    Vector3f pointAtParameter(float t) const {
        return origin + direction * t;
    }

private:

    Vector3f origin;
    Vector3f direction;
    Vector3f inv_direction;
};

inline std::ostream& operator<<(std::ostream& os, const Ray& r) {
    os << "Ray <" << r.getOrigin() << ", " << r.getDirection() << ">";
    return os;
}

#endif // RAY_H
