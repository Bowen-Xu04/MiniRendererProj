// 独立实现，除了sample函数参考了GAMES101作业框架
#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>
using namespace std;

// : implement this class and add more fields as necessary,
class Triangle : public Object3D {
public:
    Triangle() = delete;

    // a b c are three vertex positions of the triangle
    Triangle(const Vector3f& a, const Vector3f& b, const Vector3f& c, Material* m) : Object3D(m) {
        id = primitive_cnt++;
        type = OBJECT_TYPE::TRIANGLE;

        vertices[0] = a;
        vertices[1] = b;
        vertices[2] = c;

        e1 = b - a;
        e2 = c - a;
        normal = Vector3f::cross(e1, e2).normalized(); // 三角形的法向量由右手定则（四指沿三个顶点的顺序弯曲）决定

        box = calculate_box();
        area = calculate_area();
    }

    ~Triangle() override {
        primitive_cnt--;
    }

    float calculate_area() {
        return 0.5f * Vector3f::cross(e1, e2).length();
    }

    bool intersect(const Ray& ray, Hit& h) override {
        Vector3f s = ray.getOrigin() - vertices[0];
        Vector3f s1 = Vector3f::cross(ray.getDirection(), e2), s2 = Vector3f::cross(s, e1);

        float s1e1 = Vector3f::dot(s1, e1);
        if (fabs(s1e1) < EPI) {
            return false;
        }

        float invs1e1 = 1.f / s1e1;
        float t = Vector3f::dot(s2, e2) * invs1e1;
        float b1 = Vector3f::dot(s1, s) * invs1e1, b2 = Vector3f::dot(s2, ray.getDirection()) * invs1e1;

        if (t >= EPI && b1 >= 0 && b2 >= 0 && b1 + b2 <= 1) {
            if (t <= h.getT()) {
                h.set(id, t, material, ray.pointAtParameter(t), Vector3f::dot(ray.getDirection(), normal) <= 0 ? normal : -normal);
                h.set_barycentricCoords(Vector3f(1.f - b1 - b2, b1, b2));
            }
            return true;
        }

        return false;
    }

    AABB calculate_box() override {
        Vector3f pmin(std::min(vertices[0].x(), min(vertices[1].x(), vertices[2].x())),
            std::min(vertices[0].y(), min(vertices[1].y(), vertices[2].y())),
            std::min(vertices[0].z(), min(vertices[1].z(), vertices[2].z())));
        Vector3f pmax(std::max(vertices[0].x(), max(vertices[1].x(), vertices[2].x())),
            std::max(vertices[0].y(), max(vertices[1].y(), vertices[2].y())),
            std::max(vertices[0].z(), max(vertices[1].z(), vertices[2].z())));

        return AABB(pmin, pmax);
    }

    // 参考已有代码：GAMES101作业框架
    void sample(Hit& h) override {
        float x = std::sqrt(get_random_float()), y = get_random_float();
        Vector3f point = vertices[0] * (1.0f - x) + vertices[1] * (x * (1.0f - y)) + vertices[2] * (x * y);

        h.set(id, MAXT, material, point, normal);
    }

    void apply_transform(const Matrix4f& transform) override {
        vertices[0] = transformPoint(transform, vertices[0]);
        vertices[1] = transformPoint(transform, vertices[1]);
        vertices[2] = transformPoint(transform, vertices[2]);
        box = calculate_box();
    }

    void appendTriangleData(TriangleData& triangleData) override {
        vertexIndices[0] = vertex_cnt++;
        vertexIndices[1] = vertex_cnt++;
        vertexIndices[2] = vertex_cnt++;

        for (int i = 0;i < 3;i++) {
            triangleData.vertices.push_back(vertices[i].x());
            triangleData.vertices.push_back(vertices[i].y());
            triangleData.vertices.push_back(vertices[i].z());
        }

        triangleData.triangles.push_back(this);
    }

    void setVertexIndices(int i0, int i1, int i2) override {
        vertexIndices[0] = i0;
        vertexIndices[1] = i1;
        vertexIndices[2] = i2;
    }

    void getVertexIndices(int& i0, int& i1, int& i2) override {
        i0 = vertexIndices[0];
        i1 = vertexIndices[1];
        i2 = vertexIndices[2];
    }

    void print_info() override {
        printf("#%d Triangle\n", id);
        printvec3(vertices[0]);
        printvec3(vertices[1]);
        printvec3(vertices[2]);
        printf("\n");
    }

protected:
    Vector3f normal;
    Vector3f vertices[3];
    Vector3f e1, e2;

    int vertexIndices[3];
};

#endif //TRIANGLE_H
