// 独立实现
#ifndef SCENE_PARSER_H
#define SCENE_PARSER_H

#include <vecmath.h>
#include "object3d.hpp"
#include "material.hpp"
#include "sampler.hpp"

class Camera;
class Light;
class Material;
class Object3D;
class Group;
class Sphere;
class Plane;
class Triangle;
class Transform;
class Mesh;
class Curve;
class RevSurface;

#define MAX_PARSER_TOKEN_LENGTH 1024

class SceneParser {
private:
    FILE* file;
    Camera* camera;
    Vector3f background_color;

    int num_lights;
    Light** lights;
    int num_materials;
    Material** materials;
    Material* current_material;
    Group* group;

    Sampler2D::SAMPLER2D_TYPE sampler2d_type;

    bool support_point_light = true, support_directional_light = true;
    bool usingAS;
    bool usingGPU;

    float total_light_area;

    void parseFile();
    void parsePerspectiveCamera();
    void parseBackground();
    void parseLights();
    Light* parsePointLight();
    Light* parseDirectionalLight();
    void parseMaterials();
    Material* parseMaterial(Material::MATERIAL_TYPE);
    Object3D* parseObject(char token[MAX_PARSER_TOKEN_LENGTH]);
    Group* parseGroup();
    Sphere* parseSphere();
    Plane* parsePlane();
    Triangle* parseTriangle();
    Mesh* parseTriangleMesh();
    Transform* parseTransform();
    Curve* parseBezierCurve();
    Curve* parseBsplineCurve();
    RevSurface* parseRevSurface();

    int getToken(char token[MAX_PARSER_TOKEN_LENGTH]);

    Vector3f readVector3f();

    float readFloat();
    int readInt();

public:

    SceneParser() = delete;
    SceneParser(const char* filename, bool _usingAS, bool _usingGPU, Sampler2D::SAMPLER2D_TYPE _sampler2d_type);

    ~SceneParser();

    Camera* getCamera() const {
        return camera;
    }

    Vector3f getBackgroundColor() const {
        return background_color;
    }

    int getNumLights() const {
        return num_lights;
    }

    Light* getLight(int i) const {
        assert(i >= 0 && i < num_lights);
        return lights[i];
    }

    int getNumMaterials() const {
        return num_materials;
    }

    Material* getMaterial(int i) const {
        assert(i >= 0 && i < num_materials);
        return materials[i];
    }

    Group* getGroup() const {
        return group;
    }

    float get_light_area() {
        return total_light_area;
    }
};

#endif // SCENE_PARSER_H
