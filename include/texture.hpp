// 独立实现
#ifndef TEXTURE_H
#define TEXTURE_H

#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "sampler.hpp"

class Texture {
public:
    enum TEXTURE_TYPE {
        TEXTURE1D,
        TEXTURE2D,
        TEXTURE3D
    };

protected:
    TEXTURE_TYPE type;

    //int total_length; // =Texture的大小*3
    //int size[3]; // 1D：长；2D：宽、高；3D：长、宽、高
    //char* buffer;

public:

    Texture() {}

    virtual ~Texture() = 0;

    // void getBuffer(std::ifstream& file) {
    //     //std::ifstream file(filename, std::ifstream::in | std::ios::binary);
    //     // if (!file) {
    //     //     std::cout << "ERROR: Cannot read file " << filename << std::endl;
    //     //     exit(1);
    //     // }
    //     file.seekg(0, file.end);
    //     total_length = file.tellg();
    //     file.seekg(0, file.beg);
    //     buffer = new char[total_length];
    //     file.read(buffer, total_length);
    //     //file.close();
    // }
    //virtual void readFromFile(const std::string& filename) = 0;
};

class Texture2D : public Texture {
private:
    static std::unordered_map<std::string, std::shared_ptr<Texture2D>> texture2d_map;

    int width, height;
    Sampler2D* sampler;

    //char** buffer2D;
    Vector3f** texture;

    friend class PhongMaterial;

public:

    Texture2D() = delete;

    Texture2D(Sampler2D::SAMPLER2D_TYPE sampler_type, const std::string& filename) {
        type = TEXTURE_TYPE::TEXTURE2D;

        switch (sampler_type) {
        case Sampler2D::SAMPLER2D_TYPE::BILINEAR_SAMPLER:
            sampler = new BilinearSampler();
            break;

        case Sampler2D::SAMPLER2D_TYPE::BICUBIC_SAMPLER:
            sampler = new BicubicSampler();
            break;

        default:
            printf("ERROR: Unknown sampler type.\n");
            exit(1);
        }

        int channels = 0;
        unsigned char* data = readImageFromFile(filename, width, height, channels, false);

        texture = new Vector3f * [height];
        int now = 0;

        for (int i = 0;i < height;i++) {
            texture[i] = new Vector3f[width];
            for (int j = 0;j < width;j++) {
                texture[i][j] = Vector3f(((float)data[now] / 255), ((float)data[now + 1] / 255), ((float)data[now + 2] / 255));
                //printf("[%d][%d] ", i, j); printvec3(texture[i][j]); printf("\n");
                now += channels;
            }
        }

        delete[] data;
    }

    ~Texture2D() {
        if (sampler != nullptr) delete sampler;
        //if (buffer != nullptr) delete buffer;
        if (texture != nullptr) {
            for (int i = 0;i < height;i++) {
                if (texture[i] != nullptr) delete[] texture[i];
            }
            delete[] texture;
        }
    }

    //void readFromFile(const std::string& filename) override {
        // std::ifstream file(filename, std::ifstream::in | std::ios::binary);
        // if (!file) {
        //     std::cout << "ERROR: Cannot read file " << filename << std::endl;
        //     exit(1);
        // }
        // getBuffer(file);
        // std::string extension_name = filename.substr(filename.length() - 3);
        // char s1[2] = { 0,0 }, s2[2] = { 0,0 };
        // if (extension_name == "jpg") {
        //     file.seekg(164);
        //     file.read(s1, 2);
        //     file.read(s2, 2);
        //     width = (unsigned int)(s1[1]) << 8 | (unsigned int)(s1[0]);
        //     height = (unsigned int)(s2[1]) << 8 | (unsigned int)(s2[0]);
        // }
        // else if (extension_name == "png") {
        //     file.seekg(17);
        //     file.read(s1, 2);
        //     file.seekg(2, std::ios::cur);
        //     file.read(s2, 2);
        //     width = (unsigned int)(s1[1]) << 8 | (unsigned int)(s1[0]);
        //     height = (unsigned int)(s2[1]) << 8 | (unsigned int)(s2[0]);
        // }
        // else {
        //     std::cout << "ERROR: Unsupported image format: " << extension_name << std::endl;
        //     exit(1);
        // }
        // printf("<%d,%d>\n", height, width);
        // texture = new Vector3f * [height];
        // for (int i = 0;i < height;i++) {
        //     texture[i] = new Vector3f[width];
        //     for (int j = 0;j < width;j++) {
        //         texture[i][j] = Vector3f(((float)buffer[i * width + j]) / 255, ((float)buffer[i * width + j + 1]) / 255, ((float)buffer[i * width + j + 2]) / 255);
        //         printf("[%d,%d] ", i, j);
        //         printvec3(texture[i][j]);
        //         printf("\n");
        //     }
        // }
    //}

    Vector3f sample(const Vector2f& loc) const {
        //printf("(%f,%f)\n", loc.y(), loc.x());
        if (texture == nullptr) {
            printf("ERROR: No texture.\n");
            exit(1);
        }
        if (sampler == nullptr) {
            printf("ERROR: No sampler.\n");
            exit(1);
        }

        return sampler->sample(texture, width, height, loc);
    }

};

#endif