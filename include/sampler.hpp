// 独立实现
#ifndef SAMPLER_H
#define SAMPLER_H

#include "utils.hpp"

class Sampler2D { // 2D纹理的采样器
public:
    enum SAMPLER2D_TYPE {
        BILINEAR_SAMPLER,
        BICUBIC_SAMPLER
    };

    Sampler2D() {}
    ~Sampler2D() {}

    virtual Vector3f sample(Vector3f** texture, const int width, const int height, const Vector2f& loc) const = 0;
};

class BilinearSampler : public Sampler2D {
public:

    // 纹理贴图的左下角为坐标原点，坐标表示为loc=(u,v)，其中u是横坐标，v是纵坐标，u,v∈[0,1]
    Vector3f sample(Vector3f** texture, const int width, const int height, const Vector2f& loc) const override {
        float x_0 = loc.x() * width, y_0 = (1.f - loc.y()) * height; // x_0∈[0,width]，y_0∈[0,height]
        int x = std::ceil(x_0 - 0.5), y = std::ceil(y_0 - 0.5); // (x_0,y_0)所在网格中左上角的最近邻格点

        float dx = x_0 - x - 0.5, dy = y_0 - y - 0.5; // dx,dy∈[0,1)
        Vector3f color[2][2];

        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                int nowx = std::max(0, std::min(x + i, height - 1)), nowy = std::max(0, std::min(y + j, width - 1));
                // if (nowy < 0 || nowy >= height || nowx < 0 || nowx >= width) {
                //     printf("%d %d !!!\n", nowy, nowx);
                // }
                color[i][j] = texture[nowy][nowx]; // 注意：texture的存储格式为(纵坐标, 横坐标)
            }
        }
        //Vector3f col =         // printf("sample: ");
        // printvec3(col);
        // printf("\n");
        return (color[0][0] * (1 - dx) + color[0][1] * dx) * (1 - dy) + (color[1][0] * (1 - dx) + color[1][1] * dx) * dy;
    }
};

class BicubicSampler : public Sampler2D {
public:

    Vector3f sample(Vector3f** texture, const int width, const int height, const Vector2f& loc) const override {
        return Vector3f::ZERO;
    }
};

#endif