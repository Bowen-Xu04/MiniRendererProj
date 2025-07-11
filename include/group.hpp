// 独立实现
#ifndef GROUP_H
#define GROUP_H


#include "object3d.hpp"
#include "BVH.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include <iostream>
#include <vector>

// : Implement Group - add data structure to store a list of Object*
class Group : public Object3D {
private:
    int num_objects;
    std::vector<Object3D*> objects;
    std::vector<Object3D*> emissive_objects;
    BVH TLAS;
    float total_light_area; // 发光物体的总面积（不包括点光源和方向光）
    bool usingAS;

public:
    Group() = delete;

    Group(int _num_objects, bool _usingAS) :
        num_objects(_num_objects), total_light_area(0.f), usingAS(_usingAS) {
        objects.resize(num_objects);
        emissive_objects.clear();
    }

    ~Group() override {
        for (int i = 0;i < num_objects;++i) {
            delete objects[i];
        }
    }

    bool intersect(const Ray& r, Hit& h) override {
        if (usingAS) {
            if (TLAS.empty()) {
                printf("ERROR: Invalid TLAS.\n");
                exit(1);
            }

            return TLAS.intersect(r, h);
        }
        else {
            bool result = false;
            for (int i = 0;i < num_objects;++i) {
                result |= objects[i]->intersect(r, h);
            }

            return result;
        }


    }

    void addObject(int index, Object3D* obj) {
        assert(index >= 0 && index < num_objects);
        objects[index] = obj;

        if (obj->get_material()->hasEmission()) {
            emissive_objects.push_back(obj);
        }
    }

    void create_tlas() {
        printf("Creating TLAS...\n");
        assert(TLAS.empty() && (!objects.empty()));
        TLAS.build(&objects);
    }

    BVH& get_tlas() {
        return TLAS;
    }

    int getGroupSize() {
        return num_objects;
    }

    float calculate_light_area() {
        total_light_area = 0.f;
        for (auto object : emissive_objects) {
            total_light_area += object->get_area();
        }
        printf("Total light area: %f\n", total_light_area);

        return total_light_area;
    }

    void sample_on_light(Hit& h) {
        float target = get_random_float() * total_light_area, now = 0.f;
        for (auto object : emissive_objects) {
            now += object->get_area();
            if (now >= target) {
                object->sample(h);
                break;
            }
        }
    }

    void generateTriangleData(TriangleData& triangleData) {
        for (int i = 0;i < num_objects;i++) {
            objects[i]->appendTriangleData(triangleData);
        }
    }
};

#endif

