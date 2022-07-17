//
// Created by MnZn on 2022/7/13.
//

#ifndef MINI_ENGINE_RENDER_HPP
#define MINI_ENGINE_RENDER_HPP

#include "../view/image.hpp"
#include "../math/mat.hpp"
#include "../math/utils.hpp"
#include "shader.hpp"
#include "model.hpp"
#include <memory>
#include <conio.h>

/*
 本模块将完成光栅化成像流程.
 输入 : {
     摄像机位置:Vec3,
     视线方向:Vec3,
     视线平面法线:Vec3,
     着色器接口:IShader {
        固定光源位置和颜色:{Vec3,Color}[],
        环境光源颜色:Color,
     },
     模型数据:{
        顶点坐标:Vec3[],
        三角形集合:int[][3]
     }[],
     视口大小: {
        宽度:int,
        高度:int
     }
 };
 输出 : 宽高与视口相同的一副bmp图片
 */

namespace mne {
/*
 局部坐标(刚读取出来的模型数据)
 + 作用与模型本身的一些变换(旋转,位移,缩放)
 世界空间(设定一个世界原点)
 + 所有坐标跟随摄像机变换到右手系中:eye_dir->+z , eye_norm->+y , eye_pos->原点
 观察空间(所有物体都放在了+z方向的视线前,此时进行着色)
 + 透视转正交投影+正交投影
 裁剪空间(所有坐标都位于[-1,1]^3中)
 + 进行深度检测
 屏幕空间(所有数据都输出到固定大小的BMPImage中)
 */

class Render {
    Vec3 eye_pos{};         // 视线位置
    Vec3 eye_dir{0, 0, 1};  // 视线方向
    Vec3 eye_norm{0, 1, 0}; // 视线平面法线

    // std::shared_ptr<IShader> shader{}; // Todo 着色器接口,可以在其中实现光照功能
    std::vector<std::shared_ptr<Model>> models{}; // 要渲染的模型数据

    std::vector<number> depth;      // z_buffer缓存
    int                 vh{}, vw{}; // 视口大小
    BMPImage*           canvas{};   // 当前渲染对象

    Mat44 proj_mat     = Factory::identity<4>(); // 投影矩阵 : 透视转正交,依赖视口函数
    Mat44 proj_mat_inv = Factory::identity<4>(); // 投影矩阵逆矩阵 : 正交转透视
    Mat44 trans_mat{};                           // 刚体变换矩阵,不改变形状(投影矩阵会改变形状)

public:
    Render() {
        std::string_view paths[][2] = {
            //            {
            //                R"(E:\vscode\MiniEngine\res\african_head\african_head.obj)",
            //                R"(E:\vscode\MiniEngine\res\african_head\african_head_diffuse.tga)",
            //            },
            //            {
            //                R"(E:\vscode\MiniEngine\.third_party\obj\floor.obj)",
            //                R"(E:\vscode\MiniEngine\.third_party\obj\floor_diffuse.tga)",
            //            }
            {
                R"(E:\vscode\MiniEngine\res\boggie\body.obj)",
                R"(E:\vscode\MiniEngine\res\boggie\body_diffuse.tga)",
            },
            {
                R"(E:\vscode\MiniEngine\res\boggie\eyes.obj)",
                R"(E:\vscode\MiniEngine\res\boggie\eyes_diffuse.tga)",
            },
            {
                R"(E:\vscode\MiniEngine\res\boggie\head.obj)",
                R"(E:\vscode\MiniEngine\res\boggie\head_diffuse.tga)",
            },
        };
        for (auto [obj, tga] : paths) {
            std::shared_ptr<Model> model = std::make_shared<Model>();
            model->loadFromDisk(obj);
            model->colorTexture.loadFromDisk(tga);
            models.push_back(model);

            model->transform.scale      = {300, 300, 300};
            model->transform.rotate.y() = pi;
            model->transform.rotate.z() = -pi / 2;
        }
    }

public:
    // Todo 修复运行一定时间后异常的闪退bug
    void drawAt(BMPImage& image) {
        canvas           = &image;
        std::tie(vh, vw) = image.size(); // 视口大小

        // Todo 键盘控制
        //if (_kbhit()) {
        //    auto& x  = model->transform.rotate.x();
        //    auto& y  = model->transform.rotate.y();
        //    int   ch = tolower(_getch());
        //    if (ch == 'w') x -= pi / 60;
        //    else if (ch == 's') x += pi / 60;
        //    else if (ch == 'a') y -= pi / 60;
        //    else if (ch == 'd') y += pi / 60;
        //}
        //eye_norm = Factory::rotateZ(pi / 60) * eye_norm;
        depth.assign(vh * vw, std::numeric_limits<number>::max());
        // 视图变换
        auto view_mat = getViewMat();
        // 转到视口中心
        auto to_center = Factory::translate(Vec3{float(vw) / 2, float(vh) / 2, 0});

        // 渲染每个model
        for (const auto& model : models) {
            model->transform.rotate.y() += pi / 60;
            // 模型变换
            auto model_mat = model->transform.get_matrix();
            // 正交投影 Todo 透视投影

            trans_mat = Factory::merge(model_mat, view_mat, to_center);

            // 渲染每个面
            for (auto [ta, tb, tc] : model->triangles) {
                drawTriangle(model, ta, tb, tc);
            }
        }
    }

private:
    // target为要渲染的模型, t_abc为将渲染三角形的三个顶点
    void drawTriangle(const std::shared_ptr<Model>& target,
                      const TriangleNode& ta, const TriangleNode& tb, const TriangleNode& tc) {
        // 获取顶点坐标
        Vec3 poses[] = {target->vertices[ta.pos], target->vertices[tb.pos], target->vertices[tc.pos]};
        // 变换到视图空间
        for (auto& p : poses) p = trans_mat * p;

        // 缓存abc
        Vec3 a = poses[0], b = poses[1], c = poses[2];
        // 缓存xy分量
        Vec2 a2 = a.as<2>(), b2 = b.as<2>(), c2 = c.as<2>();
        // 缓存深度信息,参与插值
        Vec3 z3 = make_vec(a.z(), b.z(), c.z());

        // 获取纹理坐标
        Vec2 texes[] = {target->textures[ta.tex], target->textures[tb.tex], target->textures[tc.tex]};
        // 缓存uv分量,参与插值
        Vec3 u3 = make_vec(texes[0].x(), texes[1].x(), texes[2].x());
        Vec3 v3 = make_vec(texes[0].y(), texes[1].y(), texes[2].y());

        // 获取i方向边界
        auto i_min = std::max((int) make_vec(a.x(), b.x(), c.x()).v_min(), 0);
        auto i_max = std::min((int) make_vec(a.x(), b.x(), c.x()).v_max(), vh - 1);

        // Todo 反锯齿
#pragma omp parallel for
        for (int i = i_min; i <= i_max; ++i) {
            // 获取紧致的左右边界
            auto [fl, fr] = getTriangleBound(a2, b2, c2, float(i));
            auto l = std::max(0, (int) fl), r = std::min(vw - 1, (int) fr);
            while (l <= r && !inTriangle(make_vec(i, l), a2, b2, c2)) ++l;
            while (l <= r && !inTriangle(make_vec(i, r), a2, b2, c2)) --r;
            // 填充[l,r]区间
            for (int j = l; j <= r; ++j) {
                Vec2 p = make_vec(i, j);
                // Todo (进行逆投影变换)3D空间再求重心坐标
                Vec3 k = getGravityPos(a2, b2, c2, p);
                // 根据重心坐标进行插值
                number dep = k * z3;                   // 深度
                Vec2   uv  = make_vec(k * u3, k * v3); // 纹理坐标
                setPixel(i, j, target->colorTexture.getPixel(uv), dep);
            }
        }
    }

    void setPixel(int i, int j, const Color& fill, number z) {
#ifndef NDEBUG
        if (i < 0 || i >= vh || j < 0 || j >= vw)
            throw std::out_of_range("Render::setPixel");
#endif
        auto& ref = depth[i * vw + j];
        if (ref > z) {
            canvas->setPixel(i, j, fill), ref = z;
        }
    }

    Mat44 getViewMat() const {
        // eye_dir->+z
        auto to_z = Factory::rotateToZ(eye_dir);
        // eye_norm->+y
        auto fact_norm = (to_z * eye_norm).as<2>();
        // 绕z轴变换到+y方向
        auto to_y = Factory::rotateZ(fact_norm.rotate({0, 1}));
        // eye_pos->原点
        return Factory::merge(
            Factory::translate(-eye_pos),
            to_z.as4(), to_y.as4());
    }

private:
    // p是否在abc构成的三角形中
    static bool inTriangle(const Vec2 p, const Vec2& a, const Vec2& b, const Vec2& c) {
        // 检查p和ab,bc,ca的叉积是否同号
        number mut[] = {
            (p - a).cdot(b - a),
            (p - b).cdot(c - b),
            (p - c).cdot(a - c)};
        return (mut[0] >= 0 && mut[1] >= 0 && mut[2] >= 0) || (mut[0] <= 0 && mut[1] <= 0 && mut[2] <= 0);
    }

    // 获取重心坐标
    static Vec3 getGravityPos(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& p) {
        decltype(auto) maker = [](const Vec2& v1, const Vec2& v2, const Vec2& v3) {
            return Mat33{
                {1, v1.x(), v1.y()},
                {1, v2.x(), v2.y()},
                {1, v3.x(), v3.y()},
            };
        };

        number s1 = maker(p, b, c).det();
        number s2 = maker(a, p, c).det();
        number s3 = maker(a, b, p).det();
        number s  = s1 + s2 + s3;
        return {s1 / s, s2 / s, s3 / s};
    }

    // 获取给定x的情况下的左右边界
    static std::pair<number, number> getTriangleBound(const Vec2& a, const Vec2& b, const Vec2& c, number x) {
        // 先按x轴升序排序
        std::array<Vec2, 3> ps{a, b, c};
        std::sort(ps.begin(), ps.end(), [](const Vec2& va, const Vec2& vb) { return va.x() < vb.x(); });
        Vec2 top = ps[0], mid = ps[1], bot = ps[2];

        // 一定和top->bot有交点
        std::pair<number, number> ret;
        ret.first = intersect(top, bot, x);
        // 上方为top->mid,下方为bot->mid
        ret.second = intersect(x < mid.x() || bot.x() == mid.x() ? top : bot, mid, x);
        if (ret.first > ret.second) std::swap(ret.first, ret.second);
        return ret;
    }

    // 获取直线sa->ea与x=x0的交点
    static number intersect(const Vec2& sa, const Vec2& ea, number x) {
        // sa + t * (ea - sa) = (x, ?)
        Vec2 dir = ea - sa;
        if (dir.x() == 0) return sa.y();
        number t = (x - sa.x()) / dir.x();
        return sa.y() + t * dir.y();
    }

    // 获取直线sa->ea,sb->ed的交点
    static Vec2 intersect(const Vec2& sa, const Vec2& ea, const Vec2& sb, const Vec2& eb) {
        // sa + t1 * v = sb + t2 * w
        // P=sa , v = (ea-sa).normalize()
        // Q=sb , w = (eb-sb).normalize()
        // u = P - Q
        // t1 = cross(w,u)/cross(v,w)
        // t2 = cross(v,u)/cross(v,w)
        Vec2   v = (ea - sa).normalize(), w = (eb - sb).normalize(), u = sa - sb;
        number t1 = w.cdot(u) / v.cdot(w);
        return sa + t1 * v;
    }
};

} // namespace mne

#endif //MINI_ENGINE_RENDER_HPP
