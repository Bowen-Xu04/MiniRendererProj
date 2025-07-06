// 独立实现
#ifndef BVH_H
#define BVH_H

#include <functional>
#include <cmath>
#include <algorithm>
//#include <memory>
#include <vecmath.h>

#include "object3d.hpp"
#include "AABB.hpp"

class BVH;

class Node {
private:
    Node* left;
    Node* right;
    AABB box;
    //std::vector<Object3D*>* objects;
    int l, r;

public:

    Node(int _l, int _r) :l(_l), r(_r), left(nullptr), right(nullptr) {}

    AABB get_box() {
        return box;
    }

    friend class BVH;
};

class BVH {
private:
    //using Point3 = Vector3f;

    std::function<bool(Object3D*, Object3D*)>boxcmp[3] = {
        [](Object3D* a,Object3D* b) {return a->get_box().get_center().x() < b->get_box().get_center().x();},
        [](Object3D* a,Object3D* b) {return a->get_box().get_center().y() < b->get_box().get_center().y();},
        [](Object3D* a,Object3D* b) {return a->get_box().get_center().z() < b->get_box().get_center().z();} };

    Node* root;
    std::vector<Object3D*>* objects; // TLAS: *AABB; BLAS: *triangle/*sphere/*plane/*RevSurface

    void destruct(Node* now) {
        if (now->left != nullptr) {
            destruct(now->left);
        }
        if (now->right != nullptr) {
            destruct(now->right);
        }
        delete now;
    }

    void build_recursive(Node* now, int y) {
        //now->objects = objects;
        int l = now->l, r = now->r;
        //printf("[%d,%d]\n", l, r);
        if (l == r) {
            now->box = (*objects)[l]->get_box();
            return;
        }
        if (r == l + 1) {
            now->left = new Node(l, l);
            build_recursive(now->left, (y + 1) % 3);
            now->right = new Node(r, r);
            build_recursive(now->right, (y + 1) % 3);
        }
        else {
            int mid = (l + r) >> 1;
            auto it = objects->begin();
            std::nth_element(it + l, it + mid, it + r + 1, boxcmp[y]);

            now->left = new Node(l, mid);
            build_recursive(now->left, (y + 1) % 3);
            now->right = new Node(mid + 1, r);
            build_recursive(now->right, (y + 1) % 3);
        }
        now->box = AABB(
            Vector3f{ std::min(now->left->box.pMin.x(),now->right->box.pMin.x()), std::min(now->left->box.pMin.y(),now->right->box.pMin.y()), std::min(now->left->box.pMin.z(),now->right->box.pMin.z()) },
            Vector3f{ std::max(now->left->box.pMax.x(),now->right->box.pMax.x()), std::max(now->left->box.pMax.y(),now->right->box.pMax.y()), std::max(now->left->box.pMax.z(),now->right->box.pMax.z()) }
        );
    }

    bool intersect_recursive(Node* node, const Ray& r, Hit& h) {
        //printf("[%d %d]\n", node->l, node->r);
        if (node == nullptr) {
            printf("ERROR: Invalid BVH node.\n");
            exit(1);
        }

        if (!node->box.intersect(r)) {
            //printf("ddd\n");
            return false;
        }
        // printf("-- [%d %d]\n", node->l, node->r);
        // if (node->l < 0 || node->r < 0) printf("!!!!!\n");

        if (node->l == node->r) {
            // if ((*objects)[node->l] == nullptr) {
            //     printf("!!!!!!!\n");
            // }
            //bool k = (*objects)[node->l]->intersect(r, h);
            //printf("<<<%d>>>\n", k);
            return (*objects)[node->l]->intersect(r, h);
        }
        //printf("...2\n");
        bool left_hit = intersect_recursive(node->left, r, h), right_hit = intersect_recursive(node->right, r, h);
        //printf("[%d %d] (%d,%d)\n", node->l, node->r, h.get_id(), h.get_mesh_id());
        return left_hit || right_hit;
    }

    void generateBVHData_recursive(Node* node, int& cnt, std::vector<int>& bvh, std::vector<float>& boxes) {
        int idx = cnt;
        bvh.push_back(-1);
        bvh.push_back(-1);
        bvh.push_back(node->l);
        bvh.push_back(node->r);
        boxes.push_back(node->get_box().get_pMin().x());
        boxes.push_back(node->get_box().get_pMin().y());
        boxes.push_back(node->get_box().get_pMin().z());
        boxes.push_back(node->get_box().get_pMax().x());
        boxes.push_back(node->get_box().get_pMax().y());
        boxes.push_back(node->get_box().get_pMax().z());

        if (node->l == node->r) {
            return;
        }

        bvh[4 * idx] = ++cnt;
        generateBVHData_recursive(node->left, cnt, bvh, boxes);

        bvh[4 * idx + 1] = ++cnt;
        generateBVHData_recursive(node->right, cnt, bvh, boxes);
    }

    friend class Object3D;

public:
    BVH() :root(nullptr), objects(nullptr) {}
    BVH(std::vector<Object3D*>* _objects, Node* _root) : objects(_objects), root(_root) {}

    ~BVH() {
        if (root != nullptr) {
            destruct(root);
        }
    }

    AABB get_box() {
        return root->get_box();
    }

    bool empty() {
        return root == nullptr;
    }

    void build(std::vector<Object3D*>* _objects) {
        assert(_objects != nullptr);
        objects = _objects;
        //printf("[[%d]]\n", objects->size());
        root = new Node(0, objects->size() - 1);
        build_recursive(root, 0);
    }

    bool intersect(const Ray& r, Hit& h) {
        return intersect_recursive(root, r, h);
    }

    void generateBVHData(int& cnt, std::vector<int>& bvh, std::vector<float>& boxes) {
        generateBVHData_recursive(root, cnt, bvh, boxes);
    }
};

#endif