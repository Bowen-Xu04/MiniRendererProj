// 独立实现
#include "mesh.hpp"
//#include <tinyobjloader/tiny_obj_loader.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <cstdlib>
#include <utility>
#include <sstream>

// void Mesh::add_mesh(std::string path) {
//     //Mesh::mesh_path.
// }

// bool Mesh::check_existance(std::string path) {
//     return false;
// }

std::unordered_map<std::string, std::shared_ptr<MeshData>> Mesh::meshData_map;

bool Mesh::intersect(const Ray& r, Hit& h) {
    //printf("!!! hhh\n");
    if (meshData == nullptr) {
        printf("ERROR: meshData is null.\n");
        exit(1);
    }

    bool intersect = false;
    Hit temp_h;
    //printf("beg %d %d\n", h.get_id(), h.get_mesh_id());
    if (usingAS) {
        if (meshData->BLAS.empty()) {
            printf("ERROR: Invalid BLAS of mesh #%d.\n", mesh_id);
            exit(1);
        }

        intersect = meshData->BLAS.intersect(r, temp_h);
    }
    else {
        for (auto triangle : meshData->triangles) {
            // TriangleIndex& triIndex = t[triId];
            // Triangle triangle(v[triIndex[0]],
            //     v[triIndex[1]], v[triIndex[2]], material);
            // triangle.normal = n[triId];
            intersect |= triangle->intersect(r, temp_h);
        }
    }

    if (intersect == true && temp_h.getT() <= h.getT()) {
        //printf("true %f %f\n", temp_h.getT(), h.getT());
        h = temp_h;
        h.set_mesh_id(mesh_id);

        if (h.getMaterial() == nullptr) { // 这说明该mesh没有纹理贴图（即自带材质），因此要额外赋予mesh的材质
            h.set_material(material);
        }
        //printf("[%d]\n", h.getMaterial()->hasEmission());

        if (h.happened() && h.getMaterial() == nullptr) {
            printf("ERROR: No material for object #%d.\n", h.get_id());
            exit(1);
        }

        if (h.getMaterial()->has_texture()) {
            h.set_texCoords(calculate_interpolatedData<Vector2f>(h, meshData->texIndices, meshData->texCoords));
            //printf("%d <%f,%f>\n", h.get_mesh_id(), h.get_texCoords().y(), h.get_texCoords().x());
        }

        if (h.getMaterial()->has_normal_texture()) {
            h.set_normal(h.getMaterial()->normalTextureEnabled() ? h.getMaterial()->getNormal(h) : temp_h.getNormal());
            h.set_faceNormal(temp_h.getNormal());
        }
        else if (normal_interpolation) {
            h.set_normal(calculate_interpolatedData<Vector3f>(h, meshData->normalIndices, meshData->normals).normalized());
            h.set_faceNormal(temp_h.getNormal());
            // printf("!!! ");
            // printvec3(h.getNormal());
            // printvec3(h.getFaceNormal());
            // printf("\n");
        }
    }
    //printf("end %d %d\n", h.get_id(), h.get_mesh_id());
    return intersect;
}

Mesh::Mesh(const char* filename, Material* material, bool _usingAS, bool _usingGPU, bool normal_interp, bool dt, bool nt, Sampler2D::SAMPLER2D_TYPE _sampler2d_type) :
    Object3D(material), usingAS(_usingAS), usingGPU(_usingGPU), normal_interpolation(normal_interp), enable_diffuse_texture(dt), enable_normal_texture(nt) {
    type = OBJECT_TYPE::TRIANGULAR_MESH;
    mesh_id = triangle_mesh_cnt++;

    objFileName = (std::string)(filename);
    directory = objFileName.substr(0, objFileName.find_last_of('/') + 1);

    // Optional: Use tiny obj loader to replace this simple one.
    //using namespace tinyobj;

    // 如果先前已加载了该几何，且不使用GPU渲染
    if (meshData_map.find(filename) != meshData_map.end() && usingGPU == false) {
        // auto BVH_triangles = geometry_data[filename];
        // BLAS = BVH_triangles.first;
        // triangles = BVH_triangles.second;
        meshData = meshData_map[filename];
    }
    else {
        meshData = std::make_shared<MeshData>();

        //std::vector<Vector3f> _triangleIndices; // 存储mesh的所有三角形的所有顶点，每三个顶点组成一个三角形
        std::vector<MaterialData> _materials; // 存储mesh的所有材质
        std::vector<int> _material_ids; // 存储mesh的每个三角形的材质编号，无材质即为-1

        _materials.clear();
        _material_ids.clear();
        //v.clear(); t.clear(); n.clear();

        loadObj(filename, meshData->vertexIndices, meshData->vertices, meshData->normalIndices, meshData->normals, meshData->texIndices, meshData->texCoords, _materials, _material_ids);

        //if (!_materials.empty()) {
        for (auto& materialData : _materials) {
            materialData.enable_diffuse_texture = enable_diffuse_texture;
            materialData.enable_normal_texture = enable_normal_texture;
            //printf("[%d %d]\n", enable_diffuse_texture, enable_normal_texture);
            meshData->materials.push_back(new PhongMaterial(materialData, directory, _sampler2d_type)); // 所有通过mtl文件读入的材质都实例化为Phong材质
        }
        // }
        // else {
        //     meshData->materials.push_back(material);
        // }

        meshData->id_begin = Object3D::get_primitive_cnt();
        // 如果三角网格带有材质，则由.mtl文件指定的材质会覆盖三角形的material（但不会管mesh的material）
        // 事实上，在有自带材质的情况下，根据代码的实现逻辑，Mesh类的构造函数中传入的材质指针永远不会被调用（调用的都是triangle的材质）
        // 如果三角网格没有材质，将meshData中triangles的材质设为mesh本身的材质
        for (int i = 0;i < meshData->vertexIndices.size() / 3;i++) {
            //if (_material_ids[i]!=)
            // Triangle* triangle = ;

            // triangle->setVertexIndices(vertex_cnt + meshData->vertexIndices[3 * i], vertex_cnt + meshData->vertexIndices[3 * i + 1], vertex_cnt + meshData->vertexIndices[3 * i + 2]);

            meshData->triangles.push_back(new Triangle(meshData->vertices[meshData->vertexIndices[3 * i]],
                meshData->vertices[meshData->vertexIndices[3 * i + 1]],
                meshData->vertices[meshData->vertexIndices[3 * i + 2]],
                _material_ids[i] == -1 ? material : meshData->materials[_material_ids[i]]));

            //meshData->triangles
            //meshData->vertices[meshData->vertexIndices[]]))
//new Triangle(_triangleIndices[3 * i], _triangleIndices[3 * i + 1], _triangleIndices[3 * i + 2], _material_ids[i] == -1 ? nullptr : meshData->materials[_material_ids[i]]));
        }

        meshData->area = calculate_area();

        //vertex_cnt += meshData->vertices.size();

        if (!usingGPU) {
            meshData->create_blas();
            box = meshData->box = calculate_box();
            meshData_map[filename] = meshData;
        }

        // std::ifstream f;
        // f.open(filename);
        // if (!f.is_open()) {
        //     std::cout << "Cannot open " << filename << "\n";
        //     return;
        // }
        // std::string line;
        // std::string vTok("v");
        // std::string fTok("f");
        // std::string texTok("vt");
        // char bslash = '/', space = ' ';
        // std::string tok;
        // int texID;
        // while (true) {
        //     std::getline(f, line);
        //     if (f.eof()) {
        //         break;
        //     }
        //     if (line.size() < 3) {
        //         continue;
        //     }
        //     if (line.at(0) == '#') {
        //         continue;
        //     }
        //     std::stringstream ss(line);
        //     ss >> tok;
        //     if (tok == vTok) {
        //         Vector3f vec;
        //         ss >> vec[0] >> vec[1] >> vec[2];
        //         v.push_back(vec);
        //     }
        //     else if (tok == fTok) {
        //         if (line.find(bslash) != std::string::npos) {
        //             std::replace(line.begin(), line.end(), bslash, space);
        //             std::stringstream facess(line);
        //             TriangleIndex trig;
        //             facess >> tok;
        //             for (int ii = 0; ii < 3; ii++) {
        //                 facess >> trig[ii] >> texID;
        //                 trig[ii]--;
        //             }
        //             t.push_back(trig);
        //         }
        //         else {
        //             TriangleIndex trig;
        //             for (int ii = 0; ii < 3; ii++) {
        //                 ss >> trig[ii];
        //                 trig[ii]--;
        //             }
        //             t.push_back(trig);
        //         }
        //     }
        //     else if (tok == texTok) {
        //         Vector2f texcoord;
        //         ss >> texcoord[0];
        //         ss >> texcoord[1];
        //     }
        // }
        // computeNormal();
        // f.close();

        //printf("[%d]\n", triangles->size());
    }

    box = meshData->box;
    area = meshData->area;

    //meshData->print_info();
}

// void Mesh::computeNormal() {
//     n.resize(t.size());
//     for (int triId = 0; triId < (int)t.size(); ++triId) {
//         TriangleIndex& triIndex = t[triId];
//         Vector3f a = v[triIndex[1]] - v[triIndex[0]];
//         Vector3f b = v[triIndex[2]] - v[triIndex[0]];
//         b = Vector3f::cross(a, b);
//         n[triId] = b / b.length();
//     }
// }
// void Mesh::create_blas() {
//     assert(BLAS == nullptr && BLAS->empty() && triangles != nullptr);
//     //printf("Creating BLAS...\n");
//     // if (BLAS->get_root() != nullptr) {
//     //     BLAS->destruct(BLAS.get_root());
//     // }
//     //BLAS->get_root() = new Node(0, );
//     BLAS = std::make_shared<BVH>();
//     BLAS->build(triangles.get());
// }

float Mesh::calculate_area() {
    assert(meshData != nullptr);

    float _area = 0.f;
    for (auto triangle : meshData->triangles) {
        _area += triangle->get_area();
    }

    return _area;
}

AABB Mesh::calculate_box() {
    assert(meshData != nullptr);
    if (!usingGPU) {
        return meshData->BLAS.get_box();
    }
    else {
        Vector3f pMin = Vector3f(INF, INF, INF), pMax = Vector3f(-INF, -INF, -INF);

        for (auto triangle : meshData->triangles) {
            pMin.x() = std::min(pMin.x(), triangle->get_box().get_pMin().x());
            pMin.y() = std::min(pMin.x(), triangle->get_box().get_pMin().y());
            pMin.z() = std::min(pMin.x(), triangle->get_box().get_pMin().z());
            pMax.x() = std::max(pMin.x(), triangle->get_box().get_pMax().x());
            pMax.y() = std::max(pMin.x(), triangle->get_box().get_pMax().y());
            pMax.z() = std::max(pMin.x(), triangle->get_box().get_pMax().z());
        }

        return AABB(pMin, pMax);
    }
}

void Mesh::sample(Hit& h) {
    float target = get_random_float() * area, now = 0.f;
    for (auto object : meshData->triangles) {
        now += object->get_area();
        if (now >= target) {
            object->sample(h);
            break;
        }
    }
}

void Mesh::apply_transform(const Matrix4f& transform) {
    for (auto& vertex : meshData->vertices) {
        vertex = transformPoint(transform, vertex);
    }
    //printf("!!!\n");
    box = meshData->box = calculate_box();

    for (auto triangle : meshData->triangles) {
        triangle->apply_transform(transform);
    }
}

void Mesh::appendTriangleData(TriangleData& triangleData) {
    //printf("mesh [%d]\n", meshData->triangles.size());
    // for (int i = 0;i < meshData->vertexIndices.size() / 3;i++) {
    //     triangleData.vertexIndices.push_back(vertex_cnt + meshData->vertexIndices[3 * i]);
    //     triangleData.vertexIndices.push_back(vertex_cnt + meshData->vertexIndices[3 * i + 1]);
    //     triangleData.vertexIndices.push_back(vertex_cnt + meshData->vertexIndices[3 * i + 2]);
    //     triangleData.vertexIndices.push_back(material->get_id());
    // }

    for (auto& vertex : meshData->vertices) {
        triangleData.vertices.push_back(vertex.x());
        triangleData.vertices.push_back(vertex.y());
        triangleData.vertices.push_back(vertex.z());
        //triangleData.vertices.push_back(0.f);
    }

    //triangleData.triangles.insert(triangleData.triangles.end(), meshData->triangles.begin(), meshData->triangles.end());
    for (int i = 0;i < meshData->triangles.size();i++) {
        //printf("Triangle (%d %d %d)\n", vertex_cnt + meshData->vertexIndices[3 * i], vertex_cnt + meshData->vertexIndices[3 * i + 1], vertex_cnt + meshData->vertexIndices[3 * i + 2]);
        //meshData->triangles[i]->unique_triangleID = vertex_cnt / 3 + i;
        meshData->triangles[i]->setVertexIndices(vertex_cnt + meshData->vertexIndices[3 * i], vertex_cnt + meshData->vertexIndices[3 * i + 1], vertex_cnt + meshData->vertexIndices[3 * i + 2]);
        triangleData.triangles.push_back(meshData->triangles[i]);
    }

    vertex_cnt += meshData->vertices.size();
}

// void Mesh::print_info() override {
//     printf("#%d Mesh\n", id);
//     meshData->print_info();
// }
