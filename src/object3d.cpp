// 独立实现
#include "object3d.hpp"

int Object3D::primitive_cnt = 0, Object3D::triangle_mesh_cnt = 0, Object3D::vertex_cnt = 0;

Object3D::Object3D() : material(nullptr) {}

Object3D::~Object3D() {
    primitive_cnt--;
}

Object3D::Object3D(Material* material) {
    this->material = material;
}

int Object3D::get_id() const { return id; }
Object3D::OBJECT_TYPE Object3D::get_type() const { return type; }

// Intersect Ray with this object. If hit, store information in hit structure.
AABB Object3D::calculate_box() { return AABB(); }

float Object3D::calculate_area() { return 0.f; }
float Object3D::get_area() { return area; }

void Object3D::sample(Hit& h) {
    printf("ERROR: Emission is not supported.\n");
    exit(1); // 若该函数未重写覆盖，则必定不支持发光
};

AABB Object3D::get_box() const { return box; }

Material* Object3D::get_material() const {
    assert(material != nullptr);
    return material;
}