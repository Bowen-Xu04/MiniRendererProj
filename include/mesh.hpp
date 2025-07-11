// 独立实现
#ifndef MESH_H
#define MESH_H

#include <iostream>
#include <vector>
#include <set>
#include <cstring>
#include <memory>
#include "object3d.hpp"
#include "triangle.hpp"
#include "BVH.hpp"
#include "Vector2f.h"
#include "Vector3f.h"

struct MeshData {
    int id_begin;

    std::vector<int> vertexIndices; // 顶点的index
    std::vector<Vector3f> vertices; // 所有顶点的坐标
    std::vector<Object3D*> triangles;

    std::vector<int> normalIndices; // 法向量的index
    std::vector<Vector3f> normals; // 所有顶点的法向量

    std::vector<int> texIndices; // 纹理坐标的index
    std::vector<Vector2f> texCoords; // 所有顶点的纹理坐标（diffuse纹理和normal纹理共用）
    std::vector<Material*> materials;
    BVH BLAS;

    AABB box;
    float area;

    MeshData() {};

    MeshData(std::vector<int>&& _vertexIndices, std::vector<Vector3f>&& _vertices, std::vector<Object3D*>&& _triangles, std::vector<int>&& _texIndices, std::vector<Vector2f>&& _texCoords, std::vector<Material*>&& _materials) :
        vertexIndices(std::move(_vertexIndices)), vertices(std::move(_vertices)), triangles(std::move(_triangles)), texIndices(std::move(_texIndices)), texCoords(std::move(_texCoords)), materials(std::move(_materials)) {
        assert(!triangles.empty());
        id_begin = triangles[0]->get_id();
        create_blas();
    }

    void create_blas() { BLAS.build(&triangles); }

    void print_info() {
        printf("[%ld %ld %ld %ld]\n", texIndices.size(), triangles.size(), texCoords.size(), materials.size());

        for (int i = 0;i < triangles.size();i++) {
            printf("triangle %d:\n", i);
            triangles[i]->print_info();
            if (!texCoords.empty()) {

                int texIndex[3] = {
                    texIndices[3 * i],
                    texIndices[3 * i + 1],
                    texIndices[3 * i + 2] };

                Vector2f _texCoords[3] = { texCoords[texIndex[0]], texCoords[texIndex[1]], texCoords[texIndex[2]] };

                for (int j = 0;j < 3;j++) {
                    printf("[%d](%f,%f) ", texIndex[j], _texCoords[j].x(), _texCoords[j].y());
                }

                printf("\n");
            }
        }
    }
};

class Mesh : public Object3D {
private:
    static std::unordered_map<std::string, std::shared_ptr<MeshData>> meshData_map;

    std::string directory, objFileName;
    int mesh_id;

    std::shared_ptr<MeshData> meshData;

    bool usingAS;
    bool usingGPU;
    bool normal_interpolation;
    bool enable_diffuse_texture;
    bool enable_normal_texture;

public:
    Mesh(const char* filename, Material* m, bool _usingAS, bool _usingGPU, bool normal_interp, bool dt, bool nt, Sampler2D::SAMPLER2D_TYPE _sampler2d_type);
    ~Mesh() { Object3D::triangle_mesh_cnt--; }

    int get_id() const override {
        return mesh_id;
    }

    float calculate_area() override;
    bool intersect(const Ray& r, Hit& h) override;
    AABB calculate_box() override;
    void sample(Hit& h) override;
    void apply_transform(const Matrix4f& transform) override;
    void appendTriangleData(TriangleData& triangleData) override;

    template <class T>
    T calculate_interpolatedData(const Hit& h, const std::vector<int>& indices, const std::vector<T>& data) {
        if (h.get_id() < meshData->id_begin) {
            printf("ERROR: Invalid texIndices index.\n");
            exit(1);
        }

        int myIndices[3] = { indices[3 * (h.get_id() - meshData->id_begin)], indices[3 * (h.get_id() - meshData->id_begin) + 1], indices[3 * (h.get_id() - meshData->id_begin) + 2] };

        if (myIndices[0] < 0 || myIndices[1] < 0 || myIndices[2] < 0 || myIndices[0] >= data.size() || myIndices[1] >= data.size() || myIndices[2] >= data.size()) {
            printf("ERROR: Invalid texIndex index.\n");
            exit(1);
        }

        T myData[3] = { data[myIndices[0]], data[myIndices[1]], data[myIndices[2]] };

        return h.get_barycentricCoords().x() * myData[0] + h.get_barycentricCoords().y() * myData[1] + h.get_barycentricCoords().z() * myData[2];
    }

    void print_info() override {
        printf("#%d Mesh\n", id);
        meshData->print_info();
    }

};

#endif
