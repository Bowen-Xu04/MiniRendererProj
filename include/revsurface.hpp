// 独立实现
// 参考：课程内容
#ifndef REVSURFACE_HPP
#define REVSURFACE_HPP

#include "object3d.hpp"
#include "curve.hpp"
#include <tuple>
#include <cmath>

class RevSurface : public Object3D {
private:
    Curve* pCurve;

public:
    RevSurface(Curve* pCurve, Material* material) : pCurve(pCurve), Object3D(material) {
        id = primitive_cnt++;
        type = OBJECT_TYPE::REVSURFACE;
        box = calculate_box();
    }

    ~RevSurface() override {
        if (pCurve != nullptr) {
            delete pCurve;
        }
        primitive_cnt--;
    }

    void calc_value_derivative(const Ray& r, float t, float& Ft, float& dFt, CurvePoint& cp) {
        float A = -r.getDirection().y() * r.getDirection().y();
        float B0 = r.getDirection().x() * r.getDirection().x() + r.getDirection().z() * r.getDirection().z();
        float B1 = 2 * (r.getDirection().y() * (r.getDirection().x() * r.getOrigin().x() + r.getDirection().z() * r.getOrigin().z()) -
            r.getOrigin().y() * B0);
        float B2 = r.getOrigin().y() * r.getOrigin().y() * B0 +
            r.getDirection().y() * r.getDirection().y() * (r.getOrigin().x() * r.getOrigin().x() + r.getOrigin().z() * r.getOrigin().z()) -
            2 * r.getDirection().y() * r.getOrigin().y() * (r.getDirection().x() * r.getOrigin().x() + r.getDirection().z() * r.getOrigin().z());

        cp = pCurve->calc_point_tangent(t); // 参数t是曲线参数t_curve

        Ft = A * (cp.V.x() * cp.V.x()) + B0 * (cp.V.y() * cp.V.y()) + B1 * cp.V.y() + B2;
        dFt = 2 * A * cp.V.x() * cp.T.x() + 2 * B0 * cp.V.y() * cp.T.y() + B1 * cp.T.y();
        //printf("(%f,%f,%f)\n", t, Ft, dFt);
    }

    bool intersect(const Ray& r, Hit& h) override {
        // (PA2 optional TODO): implement this for the ray-tracing routine using G-N iteration.
        // if (!box.intersect(r)) { // 如果与曲面的包围盒不交，则肯定与曲面不相交
        //     return false;
        // }

        // 通过求解交点满足的方程，转化为关于曲线参数t（不是光线参数t！）的一元方程
        // 使用Newton迭代求解，初值根据光线的方向选取为曲线参数的最小值或最大值
        // 局限性：只能处理曲线的y(t)单调递增的情况，且无法处理相机位于旋转面内的情况
        // S(t,θ)=(x(t)cosθ, y(t), x(t)sinθ)
        float t_ray; //光线参数t
        float t_curve = (r.getDirection().y() >= 0) ? pCurve->getTmin() : pCurve->getTmax(); //曲线参数t
        float Ft = 0.0, dFt = 0.0;
        CurvePoint cp;

        for (int step = 0;step < MAXSTEP;step++) {
            //printf("[%d]\n", step);
            calc_value_derivative(r, t_curve, Ft, dFt, cp);

            if (std::fabs(Ft) < EPI) {
                printf("[%d] <%f,%f,%f>\n", step, t_curve, Ft, dFt);
                t_ray = (cp.V.y() - r.getOrigin().y()) / r.getDirection().y();
                if (t_ray < EPI) {
                    continue;
                }
                float costheta = (t_ray * r.getDirection().x() + r.getOrigin().x()) / cp.V.x();
                float sintheta = (t_ray * r.getDirection().z() + r.getOrigin().z()) / cp.V.x();

                if (std::fabs(costheta) > 1.0 || std::fabs(sintheta) > 1.0) {
                    continue;
                }

                Vector3f partialT(cp.T.x() * costheta, cp.T.y(), cp.T.x() * sintheta);
                Vector3f partialTheta(-cp.V.x() * sintheta, 0.0, cp.V.x() * costheta);

                //if (t_ray >= EPI) {
                if (t_ray <= h.getT()) {
                    h.set(id, t_ray, material, r.pointAtParameter(t_ray), Vector3f::cross(partialT, partialTheta).normalized());
                }
                //printf("hit\n");
                return true;
                //std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                //}
                //return false;
            }

            t_curve = t_curve - Ft / dFt;

            if (t_curve<pCurve->getTmin() || t_curve>pCurve->getTmax()) {
                break;
            }
        }

        return false;
    }

    AABB calculate_box() override {
        float xmax = 0.0, ymin = INF, ymax = -INF; // 计算AABB
        for (const auto& cp : pCurve->getControls()) {
            xmax = std::max(xmax, std::fabs(cp.x()));
            ymin = std::min(ymin, cp.y());
            ymax = std::max(ymax, cp.y());
            // Check flatness.
            if (cp.z() != 0.0) {
                printf("Profile of revSurface must be flat on xy plane.\n");
                exit(1);
            }
        }
        return AABB({ -xmax, ymin, -xmax }, { xmax, ymax, xmax });
    }
};

#endif //REVSURFACE_HPP
