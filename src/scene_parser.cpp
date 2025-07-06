// 独立实现
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

#include "scene_parser.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "material.hpp"
#include "object3d.hpp"
#include "group.hpp"
#include "mesh.hpp"
#include "sphere.hpp"
#include "plane.hpp"
#include "triangle.hpp"
#include "transform.hpp"
#include "curve.hpp"
#include "revsurface.hpp"
#include "texture.hpp"
#include "sampler.hpp"

#define DegreesToRadians(x) ((M_PI * x) / 180.0f)

// 目前仅支持静态场景，即场景初始化后不支持添加或删除物体
SceneParser::SceneParser(const char* filename, bool _usingAS, bool _usingGPU, Sampler2D::SAMPLER2D_TYPE _sampler2d_type) {

    // initialize some reasonable default values
    group = nullptr;
    camera = nullptr;
    background_color = Vector3f(0.5, 0.5, 0.5);
    num_lights = 0;
    lights = nullptr;
    num_materials = 0;
    materials = nullptr;
    current_material = nullptr;

    usingAS = _usingAS;
    usingGPU = _usingGPU;
    //printf("usingGPU = %d\n", usingGPU);
    sampler2d_type = _sampler2d_type;

    // parse the file
    assert(filename != nullptr);
    const char* ext = &filename[strlen(filename) - 4];

    if (strcmp(ext, ".txt") != 0) {
        printf("wrong file name extension\n");
        exit(0);
    }
    file = fopen(filename, "r");

    if (file == nullptr) {
        printf("cannot open scene file\n");
        exit(0);
    }
    parseFile();
    fclose(file);
    file = nullptr;

    total_light_area = group->calculate_light_area();

    if (num_lights == 0 && total_light_area == 0.0) {
        printf("WARNING: No lights specified.\n");
    }
}

SceneParser::~SceneParser() {

    delete group;
    delete camera;

    int i;
    for (i = 0; i < num_materials; i++) {
        delete materials[i];
    }
    delete[] materials;
    for (i = 0; i < num_lights; i++) {
        delete lights[i];
    }
    delete[] lights;
}

// ====================================================================
// ====================================================================

void SceneParser::parseFile() {
    //
    // at the top level, the scene can have a camera, 
    // background color and a group of objects
    // (we add lights and other things in future assignments)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    while (getToken(token)) {
        if (!strcmp(token, "PerspectiveCamera")) {
            parsePerspectiveCamera();
        }
        else if (!strcmp(token, "Background")) {
            parseBackground();
        }
        else if (!strcmp(token, "Lights")) {
            parseLights();
        }
        else if (!strcmp(token, "Materials")) {
            parseMaterials();
        }
        else if (!strcmp(token, "Group")) {
            group = parseGroup();
        }
        else {
            printf("Unknown token in parseFile: '%s'\n", token);
            exit(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser::parsePerspectiveCamera() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the camera parameters
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "center"));
    Vector3f center = readVector3f();
    getToken(token);
    assert(!strcmp(token, "direction"));
    Vector3f direction = readVector3f();
    getToken(token);
    assert(!strcmp(token, "up"));
    Vector3f up = readVector3f();
    getToken(token);
    assert(!strcmp(token, "angle"));
    float angle_degrees = readFloat();
    float angle_radians = DegreesToRadians(angle_degrees);
    getToken(token);
    assert(!strcmp(token, "width"));
    int width = readInt();
    getToken(token);
    assert(!strcmp(token, "height"));
    int height = readInt();
    getToken(token);
    assert(!strcmp(token, "}"));
    camera = new PerspectiveCamera(center, direction, up, width, height, angle_radians);
}

void SceneParser::parseBackground() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the background color
    getToken(token);
    assert(!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (!strcmp(token, "}")) {
            break;
        }
        else if (!strcmp(token, "color")) {
            background_color = readVector3f();
        }
        else {
            printf("Unknown token in parseBackground: '%s'\n", token);
            assert(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser::parseLights() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    // read in the number of objects
    getToken(token);
    assert(!strcmp(token, "numLights"));
    num_lights = readInt();
    lights = new Light * [num_lights];
    // read in the objects
    int count = 0;
    while (num_lights > count) {
        getToken(token);
        if (strcmp(token, "DirectionalLight") == 0) {
            if (!support_directional_light) {
                printf("WARNING: DIRECTIONAL LIGHT IS NOT SUPPORTED AND WILL BE NEGLECTED.\n");
                num_lights--;
                continue;
            }
            lights[count] = parseDirectionalLight();
        }
        else if (strcmp(token, "PointLight") == 0) {
            if (!support_point_light) {
                printf("WARNING: POINT LIGHT IS NOT SUPPORTED AND WILL BE NEGLECTED.\n");
                num_lights--;
                continue;
            }
            lights[count] = parsePointLight();
        }
        else {
            printf("Unknown token in parseLight: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token);
    assert(!strcmp(token, "}"));
}

Light* SceneParser::parseDirectionalLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "direction"));
    Vector3f direction = readVector3f();
    getToken(token);
    assert(!strcmp(token, "color"));
    Vector3f color = readVector3f();
    getToken(token);
    assert(!strcmp(token, "}"));
    return new DirectionalLight(direction, color);
}

Light* SceneParser::parsePointLight() {
    assert(support_point_light);
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "position"));
    Vector3f position = readVector3f();
    getToken(token);
    assert(!strcmp(token, "color"));
    Vector3f color = readVector3f();
    getToken(token);
    assert(!strcmp(token, "}"));
    return new PointLight(position, color);
}
// ====================================================================
// ====================================================================

void SceneParser::parseMaterials() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    // read in the number of objects
    getToken(token);
    assert(!strcmp(token, "numMaterials"));
    num_materials = readInt();
    materials = new Material * [num_materials];
    // read in the objects
    int count = 0;
    while (num_materials > count) {
        getToken(token);
        if ((!strcmp(token, "Material")) || (!strcmp(token, "PhongMaterial"))) { // 默认为Phong材质
            materials[count] = parseMaterial(Material::MATERIAL_TYPE::PHONG_MATERIAL);
        }
        else if (!strcmp(token, "GlossyMaterial")) {
            materials[count] = parseMaterial(Material::MATERIAL_TYPE::GLOSSY_MATERIAL);
        }
        else if (!strcmp(token, "ReflectiveMaterial")) {
            materials[count] = parseMaterial(Material::MATERIAL_TYPE::REFLECTIVE);
        }
        else if (!strcmp(token, "RefractiveMaterial")) {
            materials[count] = parseMaterial(Material::MATERIAL_TYPE::REFRACTIVE);
        }
        else {
            printf("Error: Unknown token in parseMaterial: '%s'\n", token);
            exit(1);
        }
        count++;
    }
    getToken(token);
    assert(!strcmp(token, "}"));
}


Material* SceneParser::parseMaterial(Material::MATERIAL_TYPE type) {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));

    Vector3f emission(0, 0, 0);

    switch (type) {
    case Material::MATERIAL_TYPE::PHONG_MATERIAL: {
        Vector3f diffuseColor(1, 1, 1), specularColor(0, 0, 0);
        float shininess = 0;

        while (true) {
            getToken(token);
            if (strcmp(token, "emission") == 0) {
                emission = readVector3f();
            }
            else if (strcmp(token, "diffuseColor") == 0) {
                diffuseColor = readVector3f();
            }
            else if (strcmp(token, "specularColor") == 0) {
                specularColor = readVector3f();
            }
            else if (strcmp(token, "shininess") == 0) {
                shininess = readFloat();
            }
            else {
                assert(!strcmp(token, "}"));
                break;
            }
        }

        return new PhongMaterial(emission, diffuseColor, specularColor, shininess);
        break;
    }
    case Material::MATERIAL_TYPE::GLOSSY_MATERIAL: {
        Vector3f albedo(1, 1, 1);
        float roughness = 0.0, metallic = 0.0;

        while (true) {
            getToken(token);
            if (strcmp(token, "emission") == 0) {
                emission = readVector3f();
            }
            else if (strcmp(token, "albedo") == 0) {
                albedo = readVector3f();
            }
            else if (strcmp(token, "roughness") == 0) {
                roughness = readFloat();
                if (roughness <= 0.0 || roughness >= 1.0) {
                    printf("ERROR: Roughness must be in (0,1).\n");
                    exit(1);
                }
            }
            else if (strcmp(token, "metallic") == 0) {
                metallic = readFloat();
                if (metallic < 0.0 || metallic > 1.0) {
                    printf("ERROR: Metallic must be in [0,1].\n");
                    exit(1);
                }
            }
            else {
                assert(!strcmp(token, "}"));
                break;
            }
        }

        return new GlossyMaterial(emission, albedo, roughness, metallic);
        break;
    }
    case Material::MATERIAL_TYPE::REFLECTIVE: {
        Vector3f transmittance(1, 1, 1);
        float transparent = 1.0;

        while (true) {
            getToken(token);
            if (strcmp(token, "emission") == 0) {
                emission = readVector3f();
            }
            else if (strcmp(token, "transmittance") == 0) {
                transmittance = readVector3f();
            }
            else if (strcmp(token, "transparent") == 0) {
                transparent = readFloat();
                assert(transparent >= 0.0 && transparent <= 1.0);
            }
            else {
                assert(!strcmp(token, "}"));
                break;
            }
        }

        return new ReflectiveMaterial(emission, transmittance, transparent);
        break;
    }
    case Material::MATERIAL_TYPE::REFRACTIVE: {
        Vector3f transmittance(1, 1, 1);
        float refractive_index = 1.0, transparent = 1.0;

        while (true) {
            getToken(token);
            if (strcmp(token, "emission") == 0) {
                emission = readVector3f();
            }
            else if (strcmp(token, "transmittance") == 0) {
                transmittance = readVector3f();
            }
            else if (strcmp(token, "refractive_index") == 0) {
                refractive_index = readFloat();
            }
            else if (strcmp(token, "transparent") == 0) {
                transparent = readFloat();
                assert(transparent >= 0.0 && transparent <= 1.0);
            }
            else {
                assert(!strcmp(token, "}"));
                break;
            }
        }

        return new RefractiveMaterial(emission, transmittance, refractive_index, transparent);
        break;
    }
    }

    return new PhongMaterial(Vector3f::ZERO, Vector3f(1.0, 1.0, 1.0), Vector3f::ZERO, 0.f); //default
    //auto* answer = 
    //return answer;
}

// ====================================================================
// ====================================================================

Object3D* SceneParser::parseObject(char token[MAX_PARSER_TOKEN_LENGTH]) {
    Object3D* answer = nullptr;
    //printf("%s\n", token);
    if (!strcmp(token, "Group")) {
        answer = (Object3D*)parseGroup();
    }
    else if (!strcmp(token, "Sphere")) {
        answer = (Object3D*)parseSphere();
    }
    else if (!strcmp(token, "Plane")) {
        answer = (Object3D*)parsePlane();
    }
    else if (!strcmp(token, "Triangle")) {
        answer = (Object3D*)parseTriangle();
    }
    else if (!strcmp(token, "TriangleMesh")) {
        answer = (Object3D*)parseTriangleMesh();
    }
    else if (!strcmp(token, "Transform")) {
        answer = (Object3D*)parseTransform();
    }
    else if (!strcmp(token, "BezierCurve")) {
        answer = (Object3D*)parseBezierCurve();
    }
    else if (!strcmp(token, "BsplineCurve")) {
        answer = (Object3D*)parseBsplineCurve();
    }
    else if (!strcmp(token, "RevSurface")) {
        answer = (Object3D*)parseRevSurface();
    }
    else {
        printf("Unknown token in parseObject: '%s'\n", token);
        exit(0);
    }
    return answer;
}

// ====================================================================
// ====================================================================

Group* SceneParser::parseGroup() {
    //
    // each group starts with an integer that specifies
    // the number of objects in the group
    //
    // the material index sets the material of all objects which follow,
    // until the next material index (scoping for the materials is very
    // simple, and essentially ignores any tree hierarchy)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));

    // read in the number of objects
    getToken(token);
    assert(!strcmp(token, "numObjects"));
    int num_objects = readInt();

    auto* answer = new Group(num_objects, usingAS);

    // read in the objects
    int count = 0;
    while (num_objects > count) {
        getToken(token);
        if (!strcmp(token, "MaterialIndex")) {
            // change the current material
            int index = readInt();
            assert(index >= 0 && index < getNumMaterials());
            current_material = getMaterial(index);
            //printf("<%d,%d,%d>\n", index, getNumMaterials(), current_material != nullptr);
        }
        else {
            Object3D* object = parseObject(token);
            assert(object != nullptr);
            answer->addObject(count, object);

            count++;
        }
    }

    getToken(token);
    assert(!strcmp(token, "}"));

    if (!usingGPU) {
        answer->create_tlas();
    }

    // return the group
    return answer;
}

// ====================================================================
// ====================================================================

Sphere* SceneParser::parseSphere() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "center"));
    Vector3f center = readVector3f();
    getToken(token);
    assert(!strcmp(token, "radius"));
    float radius = readFloat();
    getToken(token);
    assert(!strcmp(token, "}"));
    assert(current_material != nullptr);
    return new Sphere(center, radius, current_material);
}


Plane* SceneParser::parsePlane() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "normal"));
    Vector3f normal = readVector3f();
    getToken(token);
    assert(!strcmp(token, "offset"));
    float offset = readFloat();
    getToken(token);
    assert(!strcmp(token, "}"));
    assert(current_material != nullptr);
    return new Plane(normal, offset, current_material);
}


Triangle* SceneParser::parseTriangle() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "vertex0"));
    Vector3f v0 = readVector3f();
    getToken(token);
    assert(!strcmp(token, "vertex1"));
    Vector3f v1 = readVector3f();
    getToken(token);
    assert(!strcmp(token, "vertex2"));
    Vector3f v2 = readVector3f();
    getToken(token);
    assert(!strcmp(token, "}"));
    assert(current_material != nullptr);

    // Triangle* triangle = ;
    // triangle->setVertexIndices();

    return new Triangle(v0, v1, v2, current_material);
}

Mesh* SceneParser::parseTriangleMesh() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    char filename[MAX_PARSER_TOKEN_LENGTH];
    bool normal_interp = false, dt = true, nt = true;
    // get the filename
    getToken(token);
    assert(!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (!strcmp(token, "obj_file")) {
            getToken(filename);
            const char* ext = &filename[strlen(filename) - 4];
            assert(!strcmp(ext, ".obj"));
        }
        else if (!strcmp(token, "normalInterpolation")) {
            normal_interp = true;
        }
        else if (!strcmp(token, "disableDiffuseTexture")) {
            dt = false;
        }
        else if (!strcmp(token, "disableNormalTexture")) {
            nt = false;
        }
        else if (!strcmp(token, "}")) {
            break;
        }
        else {
            printf("Incorrect format for TriangleMesh!\n");
            exit(0);
        }
    }

    // assert(!);
    // getToken(token);
    // assert();
    Mesh* answer = new Mesh(filename, current_material, usingAS, usingGPU, normal_interp, dt, nt, sampler2d_type);

    return answer;
}

Curve* SceneParser::parseBezierCurve() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "controls"));
    vector<Vector3f> controls;
    while (true) {
        getToken(token);
        if (!strcmp(token, "[")) {
            controls.push_back(readVector3f());
            getToken(token);
            assert(!strcmp(token, "]"));
        }
        else if (!strcmp(token, "}")) {
            break;
        }
        else {
            printf("Incorrect format for BezierCurve!\n");
            exit(0);
        }
    }
    Curve* answer = new BezierCurve(controls);
    return answer;
}


Curve* SceneParser::parseBsplineCurve() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "controls"));
    vector<Vector3f> controls;
    while (true) {
        getToken(token);
        if (!strcmp(token, "[")) {
            controls.push_back(readVector3f());
            getToken(token);
            assert(!strcmp(token, "]"));
        }
        else if (!strcmp(token, "}")) {
            break;
        }
        else {
            printf("Incorrect format for BsplineCurve!\n");
            exit(0);
        }
    }
    Curve* answer = new BsplineCurve(controls);
    return answer;
}

RevSurface* SceneParser::parseRevSurface() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "profile"));
    Curve* profile;
    getToken(token);
    if (!strcmp(token, "BezierCurve")) {
        profile = parseBezierCurve();
    }
    else if (!strcmp(token, "BsplineCurve")) {
        profile = parseBsplineCurve();
    }
    else {
        printf("Unknown profile type in parseRevSurface: '%s'\n", token);
        exit(0);
    }
    getToken(token);
    assert(!strcmp(token, "}"));
    auto* answer = new RevSurface(profile, current_material);
    return answer;
}

Transform* SceneParser::parseTransform() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Matrix4f matrix = Matrix4f::identity();
    Object3D* object = nullptr;
    getToken(token);
    assert(!strcmp(token, "{"));
    // read in transformations: 
    // apply to the LEFT side of the current matrix (so the first
    // transform in the list is the last applied to the object)
    getToken(token);

    bool emission_is_allowed = true;
    // 如果emission_is_allowed为false说明该物体不支持emission   
    // 物体支持emission当且仅当物体仅经过UniformScale、Translate或Rotate（此时原uniform采样点可以直接经过同样的变换得到transform后物体上的uniform采样点）

    while (true) {
        if (!strcmp(token, "Scale")) {
            Vector3f s = readVector3f();
            matrix = matrix * Matrix4f::scaling(s[0], s[1], s[2]);
            emission_is_allowed &= ((s[0] == s[1]) && (s[1] == s[2]));
        }
        else if (!strcmp(token, "UniformScale")) {
            float s = readFloat();
            matrix = matrix * Matrix4f::uniformScaling(s);
        }
        else if (!strcmp(token, "Translate")) {
            matrix = matrix * Matrix4f::translation(readVector3f());
        }
        else if (!strcmp(token, "XRotate")) {
            matrix = matrix * Matrix4f::rotateX(DegreesToRadians(readFloat()));
        }
        else if (!strcmp(token, "YRotate")) {
            matrix = matrix * Matrix4f::rotateY(DegreesToRadians(readFloat()));
        }
        else if (!strcmp(token, "ZRotate")) {
            matrix = matrix * Matrix4f::rotateZ(DegreesToRadians(readFloat()));
        }
        else if (!strcmp(token, "Rotate")) {
            getToken(token);
            assert(!strcmp(token, "{"));
            Vector3f axis = readVector3f();
            float degrees = readFloat();
            float radians = DegreesToRadians(degrees);
            matrix = matrix * Matrix4f::rotation(axis, radians);
            getToken(token);
            assert(!strcmp(token, "}"));
        }
        else if (!strcmp(token, "Matrix4f")) {
            Matrix4f matrix2 = Matrix4f::identity();
            getToken(token);
            assert(!strcmp(token, "{"));
            for (int j = 0; j < 4; j++) {
                for (int i = 0; i < 4; i++) {
                    float v = readFloat();
                    matrix2(i, j) = v;
                }
            }
            getToken(token);
            assert(!strcmp(token, "}"));
            matrix = matrix2 * matrix;
            emission_is_allowed = false;
        }
        else {
            // otherwise this must be an object,
            // and there are no more transformations
            object = parseObject(token);
            emission_is_allowed &= ((object->get_type() == Object3D::OBJECT_TYPE::TRIANGLE) |
                (object->get_type() == Object3D::OBJECT_TYPE::TRIANGULAR_MESH) |
                (object->get_type() == Object3D::OBJECT_TYPE::SPHERE));
            //printf("[%d]\n", object->get_material() != nullptr);
            break;
        }
        getToken(token);
    }

    assert(object != nullptr);
    getToken(token);
    assert(!strcmp(token, "}"));

    if (emission_is_allowed == false && object->get_material()->hasEmission()) { // 如果不支持发光，且物体又有发光，则报错
        printf("ERROR: Emission is not allowed for objects to which such transformations are adapted.\n");
        exit(1);
    }

    return new Transform(matrix, object, emission_is_allowed);
}

// ====================================================================
// ====================================================================

int SceneParser::getToken(char token[MAX_PARSER_TOKEN_LENGTH]) {
    // for simplicity, tokens must be separated by whitespace
    assert(file != nullptr);
    int success = fscanf(file, "%s ", token);
    if (success == EOF) {
        token[0] = '\0';
        return 0;
    }
    return 1;
}


Vector3f SceneParser::readVector3f() {
    float x, y, z;
    int count = fscanf(file, "%f %f %f", &x, &y, &z);
    if (count != 3) {
        printf("Error trying to read 3 floats to make a Vector3f\n");
        assert(0);
    }
    return Vector3f(x, y, z);
}


float SceneParser::readFloat() {
    float answer;
    int count = fscanf(file, "%f", &answer);
    if (count != 1) {
        printf("Error trying to read 1 float\n");
        assert(0);
    }
    return answer;
}


int SceneParser::readInt() {
    int answer;
    int count = fscanf(file, "%d", &answer);
    if (count != 1) {
        printf("Error trying to read 1 int\n");
        assert(0);
    }
    return answer;
}