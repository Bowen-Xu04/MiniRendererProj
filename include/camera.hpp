// 独立实现
#ifndef CAMERA_H
#define CAMERA_H

#include "ray.hpp"
#include <vecmath.h>
#include <float.h>
#include <cmath>


class Camera {
public:
    Camera(const Vector3f& center, const Vector3f& direction, const Vector3f& up, int imgW, int imgH) {
        this->center = center;
        this->direction = direction.normalized();
        this->horizontal = Vector3f::cross(this->direction, up).normalized();
        this->up = Vector3f::cross(this->horizontal, this->direction);
        this->width = imgW;
        this->height = imgH;
        rotation_matrix = Matrix3f(this->horizontal, -this->up, this->direction);
    }

    // Generate rays for each screen-space coordinate
    virtual Ray generateRay(const Vector2f& point) = 0;
    virtual ~Camera() = default;

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    void move(const Vector3f& dir, const float dist) {
        center += dist * dir;
    }

    void rotate(const int axis, const float rad) {
        switch (axis) {
        case 0: // 绕x轴旋转
            direction = Matrix3f::rotateX(rad) * direction;
            up = Matrix3f::rotateX(rad) * up;
            break;
        case 1: // 绕y轴旋转
            direction = Matrix3f::rotateY(rad) * direction;
            up = Matrix3f::rotateY(rad) * up;
            break;
        case 2: // 绕z轴旋转
            direction = Matrix3f::rotateZ(rad) * direction;
            up = Matrix3f::rotateZ(rad) * up;
            break;
        default:
            printf("ERROR: Invalid axis.\n");
            exit(1);
        }
    }

    Matrix4f calculate_viewMatrix() const {
        return Matrix4f::identity();
    }

    virtual Matrix4f calculate_projectionMatrix() const { return Matrix4f::identity(); }

    Vector3f get_center() const { return center; }
    Vector3f get_direction() const { return direction; }
    Vector3f get_up() const { return up; }
    //Vector3f get_horizontal() const { return horizontal; }
    Matrix3f get_rotation_matrix() const { return rotation_matrix; }

    virtual Vector3f get_perspectiveData() const {
        printf("ERROR: This camera is not a perspective camera.\n");
        exit(1);
        return Vector3f::ZERO;
    }

protected:
    // Extrinsic parameters
    Vector3f center;
    Vector3f direction;
    Vector3f up;
    Vector3f horizontal;
    Matrix3f rotation_matrix;
    // Intrinsic parameters
    int width;
    int height;
};

// : Implement Perspective camera
// You can add new functions or variables whenever needed.
class PerspectiveCamera : public Camera {
private:
    float angle;
    float invfx, invfy;

public:
    PerspectiveCamera(const Vector3f& center, const Vector3f& direction,
        const Vector3f& up, int imgW, int imgH, float _angle) : Camera(center, direction, up, imgW, imgH),
        angle(_angle)
    {
        // angle is in radian.z
        invfy = 2.0 * tan(angle / 2.0) / height;
        //invfx=2*tan(angle/2)/width;
        invfx = invfy;
        //rotation_matrix = Matrix3f(this->horizontal, -this->up, this->direction);
    }

    Ray generateRay(const Vector2f& point) override {
        Vector3f dir = Vector3f((point.x() - width * 0.5) * invfx, (height * 0.5 - point.y()) * invfy, 1.f).normalized();
        //printvec3(dir); printf("\n");
        //printf("(%f,%f,1.0)\n",(point.x()-width*0.5)*invfx, (height*0.5-point.y())*invfy);
        return Ray(center, (rotation_matrix * dir).normalized());
    }

    Matrix4f calculate_projectionMatrix() const override {
        return Matrix4f::identity();
    }

    Vector3f get_perspectiveData() const {
        return Vector3f(angle, invfx, invfy);
    }
};

#endif //CAMERA_H
