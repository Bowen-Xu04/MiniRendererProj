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

public:
    Texture() {}

    virtual ~Texture() = 0;
};

class Texture2D : public Texture {
private:
    static std::unordered_map<std::string, std::shared_ptr<Texture2D>> texture2d_map;

    int width, height;
    Sampler2D* sampler;

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
                now += channels;
            }
        }

        delete[] data;
    }

    ~Texture2D() {
        if (sampler != nullptr) delete sampler;
        if (texture != nullptr) {
            for (int i = 0;i < height;i++) {
                if (texture[i] != nullptr) delete[] texture[i];
            }
            delete[] texture;
        }
    }

    Vector3f sample(const Vector2f& loc) const {
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