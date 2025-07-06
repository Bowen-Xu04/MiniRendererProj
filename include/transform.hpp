// 独立实现
#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <vecmath.h>
#include <cmath>
#include "object3d.hpp"

class Transform : public Object3D {
public:
    Transform() { type = OBJECT_TYPE::TRANSFORM; }

    Transform(const Matrix4f& m, Object3D* obj, bool _emission_is_allowed) : o(obj), emission_is_allowed(_emission_is_allowed) {
        type = OBJECT_TYPE::TRANSFORM;
        material = o->get_material();
        transform = m;
        inv_transform = m.inverse();
        box = calculate_box();

        float temp = std::cbrt(transform.determinant());
        ampli_coeff = temp * temp;
        area = calculate_area();
    }

    ~Transform() {}

    int get_id() const override {
        if (o == nullptr) {
            printf("ERROR: No transformed object.\n");
            exit(1);
        }
        return o->get_id();
    }

    virtual bool intersect(const Ray& r, Hit& h) {
        Vector3f trSource = transformPoint(inv_transform, r.getOrigin());
        Vector3f trDirection = transformDirection(inv_transform, r.getDirection());
        Ray tr(trSource, trDirection);
        Hit temp_h;

        bool inter = o->intersect(tr, temp_h);
        if (inter && temp_h.getT() <= h.getT()) {
            h.set(temp_h.get_id(), temp_h.getT(), temp_h.getMaterial(), r.pointAtParameter(temp_h.getT()), transformDirection(transform, temp_h.getNormal()).normalized());
            h.set_mesh_id(temp_h.get_mesh_id());
            h.set_barycentricCoords(temp_h.get_barycentricCoords());
            h.set_texCoords(temp_h.get_texCoords());
            h.set_faceNormal(transformDirection(transform, temp_h.getFaceNormal()).normalized());
        }
        return inter;
    }

    AABB calculate_box() override {
        AABB box0 = o->get_box();
        Vector3f box0_p[] = {
            box0.get_pMin(),
            {box0.get_pMin().x(),box0.get_pMin().y(),box0.get_pMax().z()},
            {box0.get_pMin().x(),box0.get_pMax().y(),box0.get_pMin().z()},
            {box0.get_pMin().x(),box0.get_pMax().y(),box0.get_pMax().z()},
            {box0.get_pMax().x(),box0.get_pMin().y(),box0.get_pMin().z()},
            {box0.get_pMax().x(),box0.get_pMin().y(),box0.get_pMax().z()},
            {box0.get_pMax().x(),box0.get_pMax().y(),box0.get_pMin().z()},
            box0.get_pMax()
        };

        Vector3f box0_p1[8];
        for (int i = 0;i < 8;i++) {
            Vector4f temp = transform * Vector4f(box0_p[i], 1.f);
            assert(temp.w() != 0);
            box0_p1[i] = temp.xyz() / temp.w();
        }

        // Vector3f p1 = (transform * Vector4f(box0.get_pMin(), 1.f)).xyz();
        // Vector3f p2 = (transform * Vector4f(box0.get_pMax(), 1.f)).xyz();
        //printvec3(box0.get_pMin()); printvec3(box0.get_pMax()); printf("\n");
        //printvec3(p1); printvec3(p2); printf("\n");

        Vector3f pMin(INF, INF, INF), pMax(-INF, -INF, -INF);

        for (int i = 0;i < 8;i++) {
            pMin.x() = std::min(pMin.x(), box0_p1[i].x());
            pMin.y() = std::min(pMin.y(), box0_p1[i].y());
            pMin.z() = std::min(pMin.z(), box0_p1[i].z());
            pMax.x() = std::max(pMax.x(), box0_p1[i].x());
            pMax.y() = std::max(pMax.y(), box0_p1[i].y());
            pMax.z() = std::max(pMax.z(), box0_p1[i].z());
        }

        //printvec3(pMin); printvec3(pMax); printf("\n");

        return AABB(pMin, pMax);
    }

    float calculate_area() {
        assert(o != nullptr);
        return ampli_coeff * o->get_area();
    }

    void sample(Hit& h) override {
        Hit temp_h;
        o->sample(temp_h);
        // if (!material->hasEmission()) {
        //     printf("!!!\n");
        // }
        h.set(o->get_id(), MAXT, material, transformPoint(transform, temp_h.getPoint()), transformDirection(transform, temp_h.getNormal()).normalized());
        //printf("SAMP "); printvec3(temp_h.getPoint()); printvec3(h.getPoint()); printf("\n");
        h.set_mesh_id(temp_h.get_mesh_id());
        h.set_barycentricCoords(temp_h.get_barycentricCoords());
        h.set_texCoords(temp_h.get_texCoords());
    }

    void appendTriangleData(TriangleData& triangleData) override {
        //printf("transform\n");
        o->apply_transform(transform);
        //box = calculate_box();
        //TriangleData temp;
        o->appendTriangleData(triangleData);

        // printf("===========\n");
        // temp.printInfo();
        // printf("===========\n");

        //triangleData.vertexIndices.insert(triangleData.vertexIndices.end(), temp.vertexIndices.begin(), temp.vertexIndices.end());

        // for (int i = 0;i < temp.vertices.size() / 3;i++) {
        //     Vector3f new_location = transformPoint(transform, Vector3f(temp.vertices[3 * i], temp.vertices[3 * i + 1], temp.vertices[3 * i + 2]));
        //     triangleData.vertices.push_back(new_location.x());
        //     triangleData.vertices.push_back(new_location.y());
        //     triangleData.vertices.push_back(new_location.z());
        // }

        // for (auto triangle : temp.triangles) {
        //     //triangle->
        //     triangleData.triangles.push_back(triangle);
        // }
    }

protected:
    Object3D* o; //un-transformed object
    Matrix4f transform, inv_transform; // 变形矩阵及其逆
    bool emission_is_allowed;
    float ampli_coeff; //表面积的放大系数
};

#endif //TRANSFORM_H
