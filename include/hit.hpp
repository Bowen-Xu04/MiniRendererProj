// 独立实现
#ifndef HIT_H
#define HIT_H

#include <vecmath.h>
#include "ray.hpp"

class Material;

class Hit {
public:

    // constructors
    Hit() {
        id = -1;
        mesh_id = -1;
        t = MAXT;
        material = nullptr;
    }

    Hit(float _t, Material* m, const Vector3f& n) {
        id = -1;
        mesh_id = -1;
        t = _t;
        material = m;
        normal = n;
        faceNormal = n;
    }

    Hit(const Hit& h) {
        //printf("???\n");
        id = h.id;
        mesh_id = h.mesh_id;
        t = h.t;
        material = h.material;
        point = h.point;
        normal = h.normal;
        barycentricCoords = h.barycentricCoords;
        texCoords = h.texCoords;
    }

    // destructor
    ~Hit() = default;

    bool happened() const {
        return (id != -1);
    }

    int get_id() const {
        return id;
    }

    int get_mesh_id() const {
        return mesh_id;
    }

    // int get_mesh_id_begin() const {
    //     return mesh_id_begin;
    // }

    float getT() const {
        return t;
    }

    Material* getMaterial() const {
        return material;
    }

    const Vector3f& getPoint() const {
        return point;
    }

    const Vector3f& getNormal() const {
        return normal;
    }

    const Vector3f& getFaceNormal() const {
        return faceNormal;
    }

    Vector3f get_barycentricCoords() const {
        return barycentricCoords;
    }

    Vector2f get_texCoords() const {
        return texCoords;
    }

    void set(int _id, float _t, Material* m, const Vector3f& _point, const Vector3f& n) {
        //printf("set: %d\n", _id);
        id = _id;
        t = _t;
        material = m;
        point = _point;
        normal = n;
        faceNormal = n;
    }

    void set_mesh_id(int _mesh_id) {
        //printf("set_mesh_id: %d\n", _mesh_id);
        mesh_id = _mesh_id;
        //mesh_id_begin = _mesh_id_begin;
    }

    void set_barycentricCoords(const Vector3f& coords) {
        barycentricCoords = coords;
    }

    void set_texCoords(const Vector2f& coords) {
        texCoords = coords;
    }

    void set_material(Material* _material) {
        material = _material;
    }

    void set_normal(const Vector3f& _normal) {
        normal = _normal;
    }

    void set_faceNormal(const Vector3f& _faceNormal) { // 仅在mesh使用法向插值或法向贴图时调用
        faceNormal = _faceNormal;
    }

private:
    float t;
    Material* material;
    Vector3f point, normal, faceNormal;
    Vector3f barycentricCoords; // 与三角形的交点的重心坐标
    Vector2f texCoords;

    int id; // object id
    int mesh_id; // triangle-mesh id
    //int mesh_id_begin; // triangle-mesh的三角形的id_begin

};

inline std::ostream& operator<<(std::ostream& os, const Hit& h) {
    os << "Hit <" << h.get_id() << ", " << h.getT() << ", " << h.getNormal() << ">";
    return os;
}

#endif // HIT_H
