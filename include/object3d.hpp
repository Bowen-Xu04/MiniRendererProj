// 独立实现
#ifndef OBJECT3D_H
#define OBJECT3D_H

#include "AABB.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include "material.hpp"

// Base class for all 3d entities.
class Object3D {
protected:
    static int primitive_cnt; // 所有基元的数量，即三角形、圆形、平面和旋转曲面的数量之和。Triangle mesh不属于基元，故不将其数量计算在内
    static int triangle_mesh_cnt; // triangle mesh的数量

    static int vertex_cnt; // 所有三角形物体的顶点数量，同一个mesh的不同实例重复计算

    int id;

public:
    enum OBJECT_TYPE {
        TRIANGLE,
        TRIANGULAR_MESH,
        SPHERE,
        PLANE,
        REVSURFACE,
        CURVE,
        TRANSFORM
    };

    Object3D();

    virtual ~Object3D();

    explicit Object3D(Material* material);

    static int get_primitive_cnt() {
        return primitive_cnt;
    }

    static int get_vertex_cnt() {
        return vertex_cnt;
    }

    virtual int get_id() const;
    OBJECT_TYPE get_type() const;

    // Intersect Ray with this object. If hit, store information in hit structure.
    virtual bool intersect(const Ray& r, Hit& h) = 0;
    virtual AABB calculate_box();

    virtual float calculate_area();
    float get_area();

    virtual void sample(Hit& h);

    AABB get_box() const;
    Material* get_material() const;

    virtual void apply_transform(const Matrix4f& transform) {
        printf("ERROR: This object is neither a triangle nor a triangular mesh.\n");
        exit(1);
    }

    virtual void appendTriangleData(TriangleData& triangleData) { // 若该函数未重载，则一定不是三角形或三角形网格，报错
        printf("ERROR: This object is neither a triangle nor a triangular mesh.\n");
        exit(1);
    }

    virtual void setVertexIndices(int i0, int i1, int i2) {
        printf("ERROR: This object is not a triangle.\n");
        exit(1);
    }

    virtual void getVertexIndices(int& i0, int& i1, int& i2) {
        printf("ERROR: This object is not a triangle.\n");
        exit(1);
    }

    virtual void print_info() {};

protected:
    AABB box;
    Material* material;
    OBJECT_TYPE type;
    float area;
};

#endif

