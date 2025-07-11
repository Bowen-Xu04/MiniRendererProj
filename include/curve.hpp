// 独立实现
// 参考：课程内容
#ifndef CURVE_HPP
#define CURVE_HPP

#include "object3d.hpp"
#include <vecmath.h>
#include <vector>
#include <utility>

#include <algorithm>

//  (PA2): Implement Bernstein class to compute spline basis function.
//       You may refer to the python-script for implementation.

// The CurvePoint object stores information about a point on a curve
// after it has been tesselated: the vertex (V) and the tangent (T)
// It is the responsiblility of functions that create these objects to fill in all the data.
struct CurvePoint {
    Vector3f V; // Vertex
    Vector3f T; // Tangent  (unit)
};

class Curve : public Object3D {
protected:
    int tot_part;
    std::vector<Vector3f> controls;

public:
    explicit Curve(std::vector<Vector3f> points) : controls(std::move(points)) {
        type = OBJECT_TYPE::CURVE;
    }

    virtual float getTmin() = 0;
    virtual float getTmax() = 0;

    bool intersect(const Ray& r, Hit& h) override {
        // 在三维场景中，不考虑光线与二维曲线相交的情况，因此统一return false
        return false;
    }

    std::vector<Vector3f>& getControls() {
        return controls;
    }

    virtual CurvePoint calc_point_tangent(float t) = 0;

    virtual void discretize(int resolution, std::vector<CurvePoint>& data) = 0;

    Material* getMaterial() {
        return material;
    }
};

class BezierCurve : public Curve {
public:
    explicit BezierCurve(const std::vector<Vector3f>& points) : Curve(points) {
        if (points.size() < 4 || points.size() % 3 != 1) {
            printf("Number of control points of BezierCurve must be 3n+1!\n");
            exit(0);
        }
        tot_part = (points.size() - 1) / 3;
    }

    float getTmin() {
        return 0.0;
    }

    float getTmax() {
        return (float)tot_part;
    }

    Vector3f calc_point(int part, int k, int i, float t) {
        if (k == 0) {
            return controls[part * 3 + i];
        }
        return (1 - t) * calc_point(part, k - 1, i, t) + t * calc_point(part, k - 1, i + 1, t);
    }

    Vector3f calc_tangent(int part, int k, int i, float t) {
        if (k == 0) {
            return controls[part * 3 + i + 1] - controls[part * 3 + i];
        }
        return (1 - t) * calc_tangent(part, k - 1, i, t) + t * calc_tangent(part, k - 1, i + 1, t);
    }

    CurvePoint calc_point_tangent(float t) override {
        assert(t >= 0.0 && t <= tot_part);

        if (t == tot_part) return { calc_point(tot_part - 1,3,0,1.0),calc_tangent(tot_part - 1,2,0,1.0) };

        int part = (int)std::ceil(t);
        t = t - std::ceil(t);

        return { calc_point(part,3,0,t),calc_tangent(part,2,0,t) };
    }

    void discretize(int resolution, std::vector<CurvePoint>& data) override {
        data.clear();
        //  (PA2): fill in data vector
        for (int part = 0;part < tot_part;part++) {
            for (int i = 0;i < resolution;i++) {
                data.push_back({ calc_point(part,3,0,i * 1.0 / resolution),calc_tangent(part,2,0,i * 1.0 / resolution).normalized() });
                // 因为切向量最后都要被归一化，故不在calc_tangent的返回值前*n
            }
        }

        data.push_back({ calc_point(tot_part - 1,3,0,1.0),calc_tangent(tot_part - 1,2,0,1.0).normalized() });
    }

protected:

};

class BsplineCurve : public Curve {
public:
    BsplineCurve(const std::vector<Vector3f>& points) : Curve(points), k(3) {
        if (points.size() < 4) {
            printf("Number of control points of BspineCurve must be more than 4!\n");
            exit(0);
        }

        knots.clear();
        for (int i = 0;i <= points.size() + k;i++) { // knots.size()=n+k+2为节点的个数
            knots.push_back(i * 1.0 / (points.size() + k));
        }
        tot_part = knots.size() - 1; // tot_part=n+k+1为n+k+2个节点分成的段数
    }

    float getTmin() {
        return knots[k];
    }

    float getTmax() {
        return knots[controls.size()];
    }

    CurvePoint calc(int part, float t) {
        float** B = new float* [2 * k + 1]; // B[j][p]=Bspline_{part-k+j,p}(t)，其中part就是README中的i。p=0...k，j=0...2*k，part-k+j≤n+k
        float* T = new float[k + 1]; // T[j]=Bspline_{part-k+j,k}'(t)
        for (int j = 0;j <= 2 * k;j++) {
            B[j] = new float[k + 1];
        }

        int now = 0;
        for (int p = 0;p <= k;p++) {
            if (p == 0) {
                for (int j = 0;j <= 2 * k;j++) {
                    B[j][0] = (j == k ? 1.0 : 0.0);
                }
            }
            else {
                for (int j = 0;j <= 2 * k - p;j++) {
                    now = part - k + j;
                    B[j][p] = (t - knots[now]) / (knots[now + p] - knots[now]) * B[j][p - 1] + (knots[now + p + 1] - t) / (knots[now + p + 1] - knots[now + 1]) * B[j + 1][p - 1];
                }
            }
        }

        for (int j = 0;j <= k;j++) {
            now = part - k + j;
            T[j] = B[j][k - 1] / (knots[now + k] - knots[now]) - B[j + 1][k - 1] / (knots[now + k + 1] - knots[now + 1]);
            // 因为切向量最后都要被归一化，故不在tangent的前*k
        }

        Vector3f point, tangent;
        for (int i = 0;i <= k;i++) {
            point += B[i][k] * controls[part - k + i];
            tangent += T[i] * controls[part - k + i];
        }

        for (int j = 0;j <= k;j++) {
            delete[] B[j];
        }
        delete[] B;
        delete[] T;
        assert(tangent != Vector3f(0.0));
        return { point,tangent.normalized() };
    }

    CurvePoint calc_point_tangent(float t) override {
        assert(t >= knots[k] && t <= knots[controls.size()]);

        if (t == knots[controls.size()]) return calc(controls.size() - 1, knots[controls.size() - 1] + 1.0 / tot_part);

        int part = (int)std::ceil(t * tot_part);

        return calc(part, t);
    }

    void discretize(int resolution, std::vector<CurvePoint>& data) override {
        data.clear();
        //  (PA2): fill in data vector

        for (int part = k;part < controls.size();part++) {
            for (int i = 0;i < resolution;i++) {
                data.push_back(calc(part, knots[part] + i * 1.0 / (resolution * tot_part)));
            }
        }
        data.push_back(calc(controls.size() - 1, knots[controls.size() - 1] + 1.0 / tot_part));
    }

protected:

private:
    int k;
    std::vector<float> knots;
};

#endif // CURVE_HPP
