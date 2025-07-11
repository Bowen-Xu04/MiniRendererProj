// 独立实现
#ifndef UTILS_H
#define UTILS_H

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <process.h>
#include <cmdline/cmdline.h>
#else
#include <unistd.h>
#include <getopt.h>
#endif

#include <iostream>
#include <cmath>
#include <vector>
#include <cstring>
#include <string>
#include <random>
#include <cassert>
#include <assert.h>
#include <thread>
#include <vecmath.h>
#include <omp.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
//#include <tqdm/tqdm.h>

// #ifndef TINYOBJLOADER_IMPLEMENTATION
// #define TINYOBJLOADER_IMPLEMENTATION 
// #include "../deps/tinyobjloader/tiny_obj_loader.h"
// #endif

#define OMP_ACCELERATION
// #define USE_NVIDIA_GPU
//#define NVIDIA_TEST
//#define HIT_DATA

#define EPI 1e-4
#define INF 1e38
#define MAXT 1e38
#define MAXSTEP 100
#define MAX_MATERIAL_CNT 32
#define inv_gamma 0.6

#define FPS_CALC_INTERNAL 100

#undef min
#undef max

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_2PI
#define M_2PI 6.28318530717958647692
#endif

#ifndef M_1_PI
#define M_1_PI 0.318309886183790671538
#endif

#ifndef M_1_2PI
#define M_1_2PI 0.159154943091895335768
#endif

const char str_base[] = "./mesh/";
const std::string window_name = "RTProject";

const int MAX_THREAD_NUM = omp_get_max_threads();
const int CHECK_PROGRESS_THREAD_INDEX = MAX_THREAD_NUM / 2;

class Object3D;

struct TriangleData {
    std::vector<int> vertexIndices;
    std::vector<int> emissive_vertexIndices;
    std::vector<float> vertices;
    std::vector<int> texIndices;
    std::vector<float> texCoords;

    std::vector<Object3D*> triangles;
    std::vector<int> bvh;
    std::vector<float> boxes;

    TriangleData() {
        vertexIndices.clear();
        emissive_vertexIndices.clear();
        vertices.clear();
        texIndices.clear();
        texCoords.clear();
        triangles.clear();
        bvh.clear();
        boxes.clear();
    }

    void printInfo() {
        printf("Total vertex indices: %d\n", vertexIndices.size());
        printf("Total emissive vertex indices: %d\n", emissive_vertexIndices.size());
        printf("Total vertex coords: %d\n", vertices.size());
        printf("Total triangles: %d\n", triangles.size());
        printf("Total texture indices: %d\n", texIndices.size());
        printf("Total texture coords: %d\n", texCoords.size());
    }
};

struct MaterialData {
    std::string name;

    Vector3f ambient;
    Vector3f diffuse;
    Vector3f specular;
    Vector3f transmittance;
    Vector3f emission;
    float shininess;
    float ior;       // index of refraction
    float dissolve;  // 1 == opaque; 0 == fully transparent
    float transparent; // = 1.0 - dissolve
    int illum;

    //PBR Extension
    Vector3f albedo;
    Vector3f F0;
    float roughness;
    float metallic;

    std::string diffuse_texname;
    std::string normal_texname;

    bool enable_diffuse_texture;
    bool enable_normal_texture;

    MaterialData() :
        ambient(Vector3f::ZERO), diffuse(Vector3f::ZERO), specular(Vector3f::ZERO), transmittance(Vector3f::ZERO), emission(Vector3f::ZERO),
        shininess(0.f), ior(0.f), transparent(1.f), dissolve(0.f), illum(0), enable_diffuse_texture(true), enable_normal_texture(true),
        albedo(Vector3f::ZERO), F0(Vector3f(0.04)), roughness(0.5), metallic(0.f) {
    }
};

// transforms a 3D point using a matrix, returning a 3D point
Vector3f transformPoint(const Matrix4f& mat, const Vector3f& point);

// transform a 3D direction using a matrix, returning a direction
Vector3f transformDirection(const Matrix4f& mat, const Vector3f& dir);

void loadObj(std::string filename, std::vector<int>& _vertexIndices, std::vector<Vector3f>& _vertices, std::vector<int>& _normalIndices, std::vector<Vector3f>& _normals, std::vector<int>& _texIndices, std::vector<Vector2f>& _texCoords, std::vector<MaterialData>& _materials, std::vector<int>& _material_ids);

unsigned char* readImageFromFile(std::string filename, int& width, int& height, int& channels, const bool flip_y);

void clamp(Vector3f& a);

float get_random_float();

int calculate_texture_size(int size);

void gamma_correction(Vector3f& color);

void UpdateProgress(int percent);

void printvec3(const Vector3f& vec);

#endif