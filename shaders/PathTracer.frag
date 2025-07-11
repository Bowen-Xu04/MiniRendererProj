// 独立实现，除了get_random_float函数参考了https://zhuanlan.zhihu.com/p/390862782
// 参考：https://learnopengl-cn.github.io/、https://zhuanlan.zhihu.com/p/51387524、https://blog.csdn.net/wyq1153/article/details/126191318
#version 450 core
#define STACK_SIZE 128
#define EPI 1e-4
#define MAXT 1e38
#define MAX_MATERIAL_CNT 32

#define M_PI 3.14159265358979323846
#define M_2PI 6.28318530717958647692
#define M_1_PI 0.318309886183790671538

layout(location = 0) out vec3 pixelColor;

struct Camera {
    vec3 center;

    int width;
    int height;

    float invfx, invfy;
    mat3 rotation_matrix;
};

struct Ray {
    vec3 origin;
    vec3 direction;
    vec3 inv_direction;
};

struct Hit {
    float t;
    int id;

    int material;
    vec3 point;
    vec3 normal;
    vec3 barycentricCoords;

    bool happened;
};

struct Material {
    int type;

    vec3 diffuse;
    vec3 emission;
};

uniform int USE_NVIDIA_GPU;

uniform int frameCnt;

uniform sampler2D lastFrameColorBuffer;

uniform float RR;
uniform float invRR;
uniform int usingAS;
uniform int NEE;

uniform vec3 background_color;

uniform float total_light_area;

uniform int primitiveCnt;
uniform int emissive_primitiveCnt;

uniform Camera camera;
uniform Material materials[MAX_MATERIAL_CNT];

uniform isamplerBuffer vertexIndices; // ivec4类型的sampler，每个位置前三个分量存储三角形的三个顶点的编号，最后一个分量存储材质编号
uniform isamplerBuffer emissive_vertexIndices; // ivec4类型的sampler，每个位置前三个分量存储三角形光源的三个顶点的编号，最后一个分量存储三角形编号
uniform samplerBuffer vertices; // vec3类型的sampler，每个位置存储一个顶点的空间坐标

uniform isamplerBuffer BVH; // ivec4类型的sampler，存储格式为(左子节点编号, 右子节点编号, 包含的三角形的最小编号, 包含的三角形的最大编号)；若为叶子节点，则前两个分量均为-1
uniform samplerBuffer AABB; // vec3类型的sampler，第2*i和(2*i+1)个元素分别表示BVH中第i个节点的包围盒的pMin和pMax

// 参考已有代码：https://zhuanlan.zhihu.com/p/390862782
float get_random_float(int randCnt) {
    return fract(sin(frameCnt * 15.637 + gl_FragCoord.x * 76.243 + gl_FragCoord.y * 37.168 + randCnt * 83.511) * 14375.5964);
}

vec3 sampleHemisphere(vec3 N, inout int randCnt) {
    float x_1 = get_random_float(++randCnt), x_2 = get_random_float(++randCnt);
    float z = abs(1.0 - 2.0 * x_1);
    float r = sqrt(1.0 - z * z), phi = 2 * M_PI * x_2;
    vec3 localRay = vec3(r * cos(phi), r * sin(phi), z);
    vec3 B, C;
    if(abs(N.x) > abs(N.y)) {
        float invLen = 1.0 / sqrt(N.x * N.x + N.z * N.z);
        C = vec3(N.z * invLen, 0.0, -N.x * invLen);
    } else {
        float invLen = 1.0 / sqrt(N.y * N.y + N.z * N.z);
        C = vec3(0.0, N.z * invLen, -N.y * invLen);
    }
    B = cross(C, N);

    return localRay.x * B + localRay.y * C + localRay.z * N;
}

vec3 sample_triangle(mat3 vertices, inout int randCnt) {
    float x = sqrt(get_random_float(++randCnt)), y = get_random_float(++randCnt);

    return vertices[0] * (1.0 - x) + vertices[1] * (x * (1.0 - y)) + vertices[2] * (x * y);
}

Hit sample_on_light(inout int randCnt) {
    float target = get_random_float(++randCnt) * total_light_area, now = 0.0;
    for(int i = 0; i < emissive_primitiveCnt; ++i) {
        ivec3 indices = texelFetch(emissive_vertexIndices, i).xyz;
        mat3 vertices = mat3(texelFetch(vertices, indices.x).xyz, texelFetch(vertices, indices.y).xyz, texelFetch(vertices, indices.z).xyz);

        float area = 0.5 * length(cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));
        now += area;

        if(now >= target) {
            Hit h;
            h.id = texelFetch(emissive_vertexIndices, i).w;
            h.t = MAXT;
            h.material = texelFetch(emissive_vertexIndices, i).w;
            h.point = sample_triangle(vertices, randCnt);
            h.normal = normalize(cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));

            return h;
        }
    }
}

vec3 calculate_inv_direction(Ray r) {
    return vec3(1.0 / r.direction.x, 1.0 / r.direction.y, 1.0 / r.direction.z);
}

Ray generateRay(vec2 screenCoords) {
    vec3 dir = vec3((screenCoords.x - camera.width * 0.5) * camera.invfx, (camera.height * 0.5 - screenCoords.y) * camera.invfy, 1.0);

    Ray newRay;
    newRay.origin = camera.center;
    newRay.direction = normalize(camera.rotation_matrix * dir);
    newRay.inv_direction = calculate_inv_direction(newRay);

    return newRay;
}

vec3 pointAtParameter(Ray r, float t) {
    return r.origin + t * r.direction;
}

vec3 evalBSDF(vec3 wo, Hit h) {
    return dot(h.normal, wo) > 0.0 ? materials[h.material].diffuse * M_1_PI : vec3(0, 0, 0);
}

bool intersectAABB(Ray r, int AABBIndex) {
    vec3 pMin = texelFetch(AABB, AABBIndex * 2).xyz, pMax = texelFetch(AABB, AABBIndex * 2 + 1).xyz;
    float tx1 = (pMin.x - r.origin.x) * r.inv_direction.x, tx2 = (pMax.x - r.origin.x) * r.inv_direction.x;
    float ty1 = (pMin.y - r.origin.y) * r.inv_direction.y, ty2 = (pMax.y - r.origin.y) * r.inv_direction.y;
    float tz1 = (pMin.z - r.origin.z) * r.inv_direction.z, tz2 = (pMax.z - r.origin.z) * r.inv_direction.z;
    float Tx1 = min(tx1, tx2), Tx2 = max(tx1, tx2);
    float Ty1 = min(ty1, ty2), Ty2 = max(ty1, ty2);
    float Tz1 = min(tz1, tz2), Tz2 = max(tz1, tz2);
    float tenter = max(Tx1, max(Ty1, Tz1));
    float texit = min(Tx2, min(Ty2, Tz2));

    if(tenter <= texit + EPI && texit > EPI) {
        return true;
    }
    return false;
}

void intersectTriangle(Ray r, inout Hit h, int triangleIndex) {
    ivec3 indices = texelFetch(vertexIndices, triangleIndex).xyz;
    vec3 v0 = texelFetch(vertices, indices.x).xyz, v1 = texelFetch(vertices, indices.y).xyz, v2 = texelFetch(vertices, indices.z).xyz;
    vec3 e1 = v1 - v0, e2 = v2 - v0;
    vec3 s = r.origin - v0;
    vec3 s1 = cross(r.direction, e2), s2 = cross(s, e1);
    float s1e1 = dot(s1, e1);

    if(abs(s1e1) < EPI) {
        return;
    }

    float invs1e1 = 1.0 / s1e1;
    float t = dot(s2, e2) * invs1e1;
    float b1 = dot(s1, s) * invs1e1, b2 = dot(s2, r.direction) * invs1e1;
    if(t >= EPI && b1 >= 0 && b2 >= 0 && b1 + b2 <= 1) {
        if(t <= h.t) {
            h.happened = true;
            h.id = triangleIndex;
            h.t = t;
            h.material = texelFetch(vertexIndices, triangleIndex).w;
            h.point = pointAtParameter(r, t);
            h.normal = normalize(cross(e1, e2));

            if(dot(r.direction, h.normal) > 0) {
                h.normal = -h.normal;
            }
            h.barycentricCoords = vec3(1.0 - b1 - b2, b1, b2);
        }
    }
}

Hit intersect(Ray r) {
    Hit h;
    h.t = MAXT;

    if(usingAS == 1) {
        int node[32];
        int top = 0;
        node[top++] = 0;

        while(top > 0) {
            int now = node[--top];

            if(intersectAABB(r, now) == false) {
                continue;
            }

            ivec4 nodeData = texelFetch(BVH, now).xyzw;
            if(nodeData.z == nodeData.w) {
                intersectTriangle(r, h, nodeData.z);
                continue;
            }

            node[top++] = nodeData.x;
            node[top++] = nodeData.y;
        }

    } else {
        for(int i = 0; i < primitiveCnt; ++i) {
            intersectTriangle(r, h, i);
        }
    }

    return h;
}

vec3 castRay(Ray r) {
    int randCnt = 0;
    vec3 emission[STACK_SIZE];
    vec3 multiplier[STACK_SIZE];

    Ray nowray = r;

    int depth = 0;
    vec3 color = vec3(0, 0, 0);

    while(true) {
        if(depth >= STACK_SIZE) {
            --depth;
            break;
        }

        Hit nowhit = intersect(nowray);

        if(nowhit.happened) {
            if(materials[nowhit.material].type == 0) { // diffuse
                if(NEE == 1) {
                    emission[depth] = materials[nowhit.material].emission;

                    if(total_light_area > 0.0) {
                        Hit newhit = sample_on_light(randCnt);
                        vec3 wi = normalize(newhit.point - nowhit.point);
                        float costheta = dot(nowhit.normal, wi);

                        if(costheta >= 0.0) {
                            Ray testray;
                            testray.origin = nowhit.point;
                            testray.direction = wi;
                            testray.inv_direction = calculate_inv_direction(testray);

                            Hit testhit = intersect(testray);

                            if(distance(newhit.point, testhit.point) < EPI && distance(newhit.point, nowhit.point) >= 0.05) {
                                emission[depth] += total_light_area * costheta * abs(dot(newhit.normal, wi)) / pow(distance(newhit.point, nowhit.point), 2) * (materials[newhit.material].emission * evalBSDF(-nowray.direction, nowhit));
                            }
                        }
                    }

                    if(get_random_float(++randCnt) < RR) {
                        vec3 wi = sampleHemisphere(nowhit.normal, randCnt);
                        float costheta = dot(nowhit.normal, wi); // sample出来的wi能保证costheta>=0

                        Ray newray;
                        newray.origin = nowhit.point;
                        newray.direction = wi;
                        newray.inv_direction = calculate_inv_direction(newray);

                        Hit testhit = intersect(newray);

                        if(testhit.happened && dot(nowhit.normal, wi) >= 0.0) {
                            if(materials[testhit.material].emission == vec3(0.0)) {
                                multiplier[depth] = invRR * M_2PI * costheta * evalBSDF(-nowray.direction, nowhit);

                                nowray = newray;
                                ++depth;                   
                            } else {
                                break;
                            }
                        } else {
                            emission[depth] += background_color;
                            break;
                        }

                    } else {
                        break;
                    }

                } else {
                    emission[depth] = materials[nowhit.material].emission;

                    if(get_random_float(++randCnt) < RR) {
                        vec3 wi = sampleHemisphere(nowhit.normal, randCnt);
                        float costheta = dot(nowhit.normal, wi);

                        multiplier[depth] = invRR * M_2PI * costheta * evalBSDF(-nowray.direction, nowhit);

                        nowray.origin = nowhit.point;
                        nowray.direction = wi;
                        nowray.inv_direction = calculate_inv_direction(nowray);
                        ++depth;

                    } else {
                        break;
                    }
                }
            }
        } else {
            emission[depth] = background_color;
            break;
        }
    }

    color = emission[depth];

    for(int i = depth - 1; i >= 0; --i) {
        color = emission[i] + multiplier[i] * color;
    }

    return color;
}

vec3 gamma_correction(vec3 color) {
    return vec3(pow(color.x, 0.6), pow(color.y, 0.6), pow(color.z, 0.6));
}

vec3 inv_gamma_correction(vec3 color) {
    return vec3(pow(color.x, 1.666667), pow(color.y, 1.666667), pow(color.z, 1.666667));
}

void main() {
    Ray ray = generateRay(gl_FragCoord.xy); // x为横坐标，y为纵坐标，左下角为原点
    Hit hit;
    hit.t = MAXT;

    vec3 thisFrameColor = castRay(ray);

    if(USE_NVIDIA_GPU == 1) {
        // pixelColor = vec3(0.2, 0.3, 0.4);
        // return;
        //ivec2 size = textureSize(lastFrameColorTBO);
        // if(camera.width == 512 && camera.height == 512) {
        // if(texelFetch(lastFrameColorTBO, int(gl_FragCoord.y * camera.width + gl_FragCoord.x)).xyz == ivec3(0, 0, 0)) {
        //     pixelColor = vec3(0.6, 0.3, 0.4);
        // } else {
        //     pixelColor = vec3(1, 0, 0);
        // }
        // return;
        //pixelColor = vec3(textureSize(lastFrameColorTBO) * 1.0);
        //pixelColor = gamma_correction((inv_gamma_correction(texelFetch(lastFrameColorTBO, int(gl_FragCoord.y) * camera.width + int(gl_FragCoord.x)).xyz * 1.0 / 255) * frameCnt + thisFrameColor) / (frameCnt + 1));
        pixelColor = gamma_correction((inv_gamma_correction(texelFetch(lastFrameColorBuffer, ivec2(gl_FragCoord.xy), 0).xyz) * frameCnt + thisFrameColor) / (frameCnt + 1));
        //lastFrameColorBuffer

    } else {
        pixelColor = (texelFetch(lastFrameColorBuffer, ivec2(gl_FragCoord.xy), 0).xyz * frameCnt + thisFrameColor) / (frameCnt + 1);
    }

}