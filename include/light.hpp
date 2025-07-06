#ifndef LIGHT_H
#define LIGHT_H

#include <Vector3f.h>
#include "object3d.hpp"

class Light {
public:
    enum LIGHT_TYPE {
        DIRECTIONAL_LIGHT,
        POINT_LIGHT
    };

    Light() = default;

    virtual ~Light() = default;

    virtual void getIllumination(const Vector3f& p, Vector3f& dir, Vector3f& col, float& t) const = 0;

    LIGHT_TYPE get_type() const { return type; }
    //virtual Vector3f getDirection(const Vector3f& p) const = 0;

protected:
    LIGHT_TYPE type;
};


class DirectionalLight : public Light {
public:
    DirectionalLight() = delete;

    DirectionalLight(const Vector3f& d, const Vector3f& c) {
        type = LIGHT_TYPE::DIRECTIONAL_LIGHT;
        direction = d.normalized();
        color = c;
    }

    ~DirectionalLight() override = default;

    ///@param p unsed in this function
    ///@param distanceToLight not well defined because it's not a point light
    void getIllumination(const Vector3f& p, Vector3f& dir, Vector3f& col, float& t) const override {
        // the direction to the light is the opposite of the
        // direction of the directional light source
        dir = -direction;
        col = color;
        t = MAXT;
    }

    // Vector3f getDirection(const Vector3f& p) const override {
    //     return 
    // }

private:

    Vector3f direction;
    Vector3f color;

};

class PointLight : public Light {
public:
    PointLight() = delete;

    PointLight(const Vector3f& p, const Vector3f& c) {
        type = LIGHT_TYPE::POINT_LIGHT;
        position = p;
        color = c;
    }

    ~PointLight() override = default;

    void getIllumination(const Vector3f& p, Vector3f& dir, Vector3f& col, float& t) const override {
        // the direction to the light is the opposite of the
        // direction of the directional light source
        dir = (position - p);
        t = dir.length();
        dir = dir / dir.length();
        col = color;
    }

    // Vector3f getDirection(const Vector3f& p, Vector3f& dir) const override {

    // }

private:

    Vector3f position;
    Vector3f color;

};

#endif // LIGHT_H
