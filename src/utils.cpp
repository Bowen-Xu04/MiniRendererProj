// 多种原创性
#include "utils.hpp"

#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#endif
#include <tinyobjloader/tiny_obj_loader.h>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include <stb_image/stb_image.h>

// 完全拷贝：https://www.cnblogs.com/sdragonx/p/12183194.html
#if defined(_WIN32) && defined(USE_NVIDIA_GPU)
extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

Vector3f transformPoint(const Matrix4f& mat, const Vector3f& point) {
    return (mat * Vector4f(point, 1)).xyz();
}

// transform a 3D direction using a matrix, returning a direction
Vector3f transformDirection(const Matrix4f& mat, const Vector3f& dir) {
    return (mat * Vector4f(dir, 0)).xyz();
}

// 独立实现
void parseMaterial(MaterialData& dst, const tinyobj::material_t& src) {
    dst.name = src.name;

    for (int i = 0;i < 3;i++) {
        dst.ambient[i] = src.ambient[i]; // Ka
        dst.diffuse[i] = src.diffuse[i]; // Kd
        dst.specular[i] = src.specular[i]; // Ks
        dst.transmittance[i] = src.transmittance[i]; // Tf
        dst.emission[i] = src.emission[i]; // Ke
    }

    dst.shininess = src.shininess; // Ns
    dst.ior = src.ior; // Ni
    dst.dissolve = src.dissolve; // d
    dst.illum = src.illum; // illum

    dst.diffuse_texname = src.diffuse_texname; // map_Kd
    dst.normal_texname = src.normal_texname; // norm
}

// 参考已有代码：https://github.com/tinyobjloader/tinyobjloader
void loadObj(std::string filename, std::vector<int>& _vertexIndices, std::vector<Vector3f>& _vertices, std::vector<int>& _normalIndices, std::vector<Vector3f>& _normals, std::vector<int>& _texIndices, std::vector<Vector2f>& _texCoords, std::vector<MaterialData>& _materials, std::vector<int>& _material_ids) {
    // _indices    _triangles        _texCoords
    // 1 index <-> 1 vertexCoord <-> 1 texCoord
    // _triangles         _texCoords      meshData->triangles     _material_ids 
    // 3 vertexCoords <-> 3 texCoords <-> 1 triangle          <-> 1 material_id

    bool triangulate = true;

    std::string warn;
    std::string err;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    bool is_success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str(),
        filename.substr(0, filename.find_last_of('/') + 1).c_str(), triangulate);

    if (!warn.empty()) {
        std::cout << "WARNING: " << warn;
    }
    if (!err.empty()) {
        std::cout << "ERROR: " << err;
    }
    if (!is_success) {
        std::cout << "Failed to read object file: " << filename << "." << std::endl;
        exit(1);
    }

    std::cout << "Loading object file: " << filename << "..." << std::endl;

    // v = std::move(attrib.vertices);
    // n = std::move(attrib.normals);
    //assert(shapes.size() == 1);

    for (int k = 0; k < shapes.size(); ++k) { // 遍历所有shape
        int offset = 0;

        for (int i = 0; i < shapes[k].mesh.num_face_vertices.size(); ++i) { // 遍历shape的所有面
            //printf("face %d:\n", i);
            int fv = shapes[k].mesh.num_face_vertices[i];
            assert(fv == 3);

            for (int j = 0; j < fv; ++j) { // 遍历面的所有顶点    
                tinyobj::index_t idx = shapes[k].mesh.indices[offset + j];
                _vertexIndices.push_back(idx.vertex_index);
                _normalIndices.push_back(idx.normal_index);
                //printf("vert %d:\n[%d,%d] ", j, idx.vertex_index, idx.texcoord_index);
                _texIndices.push_back(idx.texcoord_index);

                // _triangleIndices.push_back(
                //     { attrib.vertices[3 * idx.vertex_index],attrib.vertices[3 * idx.vertex_index + 1],attrib.vertices[3 * idx.vertex_index + 2] });
                // printvec3(_triangleIndices[_triangleIndices.size() - 1]);
                // printf("\n");
                // if (idx.texcoord_index >= 0) { // >=0，若有指定material，且material有指定纹理
                //     _texCoords.push_back(
                //         { attrib.texcoords[2 * idx.texcoord_index],attrib.texcoords[2 * idx.texcoord_index + 1] });
                //     //printf("(%f,%f)\n", _texCoords[_texCoords.size() - 1].x(), _texCoords[_texCoords.size() - 1].y());
                // }
            }

            _material_ids.push_back(shapes[k].mesh.material_ids[i]);
            offset += fv;
        }
    }

    for (int i = 0;i < attrib.vertices.size() / 3;i++) {
        _vertices.push_back(Vector3f(attrib.vertices[3 * i], attrib.vertices[3 * i + 1], attrib.vertices[3 * i + 2]));
    }

    for (int i = 0;i < attrib.normals.size() / 3;i++) {
        _normals.push_back(Vector3f(attrib.normals[3 * i], attrib.normals[3 * i + 1], attrib.normals[3 * i + 2]));
    }

    for (int i = 0;i < attrib.texcoords.size() / 2;i++) {
        _texCoords.push_back(Vector2f(attrib.texcoords[2 * i], attrib.texcoords[2 * i + 1]));
        //printf("tex %d: (%f,%f)\n", i, _texCoords[_texCoords.size() - 1].x(), _texCoords[_texCoords.size() - 1].y());
    }

    _materials.resize(materials.size());
    for (int k = 0; k < materials.size(); ++k) {
        parseMaterial(_materials[k], materials[k]);
    }

    printf("Number of vertices: %ld\n", attrib.vertices.size() / 3);
    printf("Number of triangles: %ld\n", _vertexIndices.size() / 3);
    printf("Number of normals: %ld\n", attrib.normals.size() / 3);
    printf("Number of texCoords: %ld\n", attrib.texcoords.size() / 2);
    printf("Number of shapes: %ld\n", shapes.size());
    printf("Number of materials: %ld\n", materials.size());

}

// 参考已有代码：https://blog.csdn.net/u012278016/article/details/105784912
unsigned char* readImageFromFile(std::string filename, int& width, int& height, int& channels, const bool flip_y) {
    std::cout << "Loading texture2d file: " << filename << "..." << std::endl;

    std::string extension_name = filename.substr(filename.length() - 3);
    if (extension_name != "jpg" && extension_name != "png") {
        std::cout << "ERROR: Unsupported image format: " << extension_name << std::endl;
        exit(1);
    }

    stbi_set_flip_vertically_on_load(flip_y);
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
    //printf("[%d %d %d]\n", data[0], data[1], data[2]);
    if (data == nullptr) {
        std::cout << "ERROR: Failed to read image file " << filename << std::endl;
        exit(1);
    }
    if (channels != 3 && channels != 4) {
        printf("ERROR: Unsupported channel number.\n");
        exit(1);
    }

    printf("Width: %d\nHeight: %d\nChannels: %d\n", width, height, channels);

    return data;

    // texture = new Vector3f * [height];
    // int now = 0;

    // for (int i = 0;i < height;i++) {
    //     texture[i] = new Vector3f[width];
    //     for (int j = 0;j < width;j++) {
    //         texture[i][j] = Vector3f(((float)data[now] / 255), ((float)data[now + 1] / 255), ((float)data[now + 2] / 255));
    //         //printf("[%d][%d] ", i, j); printvec3(texture[i][j]); printf("\n");
    //         now += channels;
    //     }
    // }

    // stbi_image_free(data);
}

void clamp(Vector3f& a) {
    if (a.x() < 0.f) a.x() = 0.f;
    else if (a.x() > 1.f) a.x() = 1.f;
    if (a.y() < 0.f) a.y() = 0.f;
    else if (a.y() > 1.f) a.y() = 1.f;
    if (a.z() < 0.f) a.z() = 0.f;
    else if (a.z() > 1.f) a.z() = 1.f;
}

// 完全拷贝：GAMES101作业框架
float get_random_float() {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<float> dist(0.f, 1.f); // distribution in range [0, 1]

    return dist(rng);
}

// 以下代码均独立实现
int calculate_texture_size(int num_of_elements) {
    int _size = std::ceil(sqrt(num_of_elements * 1.0)), size = 1;

    while (size < _size) {
        size <<= 1;
    }

    return size;
}

void gamma_correction(Vector3f& color) {
    color.x() = std::pow(color.x(), inv_gamma);
    color.y() = std::pow(color.y(), inv_gamma);
    color.z() = std::pow(color.z(), inv_gamma);
}

void UpdateProgress(int percent) { // percent为[0,100]内的整数
    //printf("+++\n");
#ifndef HIT_DATA
    int barWidth = 100;
    printf("Progress: [");
    int pos = barWidth * percent / 100;
    for (int i = 1; i <= barWidth; ++i) {
        if (i < pos) putchar('=');
        else if (i == pos) putchar('>');
        else putchar(' ');
    }
    printf("] %d%%\r", percent);
#endif
};

void printvec3(const Vector3f& vec) {
    printf("(%f,%f,%f) ", vec.x(), vec.y(), vec.z());
}

// void throw_error(const char* info) {
//     printf("ERROR: %s\n", info);
//     exit(1);
// }

//void hhh() {}