//
// Created by MnZn on 2022/7/13.
//

#ifndef MINI_ENGINE_RS_RENDER_HPP
#define MINI_ENGINE_RS_RENDER_HPP

#include "../interface/shader.hpp"
#include "../interface/render.hpp"
#include "../math/mat.hpp"
#include "../math/utils.hpp"
#include "../store/image.hpp"
#include "../data/camera.hpp"
#include "../store/model.hpp"
#include <memory>

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

 - 输入需要渲染的所有图元
 - 运行顶点着色器
   负责根据变换(模型+视图+透视)输出顶点坐标.
 - 图元装配
   负责将视锥体(可见空间)外的图元过滤,派发给不同光栅化函数.
 - 光栅化
   负责将图元(3D信息)转为片元.
 - 运行片元着色器
   负责输出每个像素的颜色.
 - 输出到缓存区
   深度检测,透明度,阴影等.
 */

class RsRender: public IRender {
private:
    BMPImage* canvas{};   // 当前渲染对象
    int       vh{}, vw{}; // 视口大小

    std::vector<number> depth; // z_buffer缓存

    Mat44 trans_mat{};     // 当前模型的变换矩阵
    Mat44 trans_mat_inv{}; // trans_mat.invert()的缓存
    Mat44 screen_mat{};    //
    Mat44 screen_mat_inv{};

    struct VertexData {
        Vec3  position;
        Vec2  texCoord;
        Color color{};
    };

public:
    RsRender() = default;

public:
    void drawAt(BMPImage& image) final {
        vh = camera.vh, vw = camera.vw;      // 视口大小
        canvas = &image, image.init(vh, vw); // 绑定画布
        camera.update();                     // 更新摄像机参数

        // 初始化深度缓存
        depth.assign(vh * vw, std::numeric_limits<number>::max());

        // 转观察空间
        auto view_mat = camera.getViewMat();
        // 转裁剪空间
        auto project_mat = camera.getProjectionMat();
        // 转屏幕空间
        screen_mat     = camera.getScreenMat();
        screen_mat_inv = screen_mat.invert();

        // 渲染每个model
        for (const auto& model : models) {
            model->transform.rotate.y() += pi / 60;
            // 局部转世界空间
            auto model_mat = model->transform.get_matrix();

            trans_mat     = Factory::merge(model_mat, view_mat, project_mat, screen_mat);
            trans_mat_inv = trans_mat.invert();

            // 渲染每个面
            for (auto& abc : model->triangles) {
                std::array<VertexData, 3> data;
                // 为每个顶点执行顶点着色器,输出裁剪空间的坐标
                for (int i = 0; i < 3; ++i) {
                    auto& drf = data[i];
                    drf       = {model->vertices[abc[i].pos], model->textures[abc[i].tex]};
                    model->shader->vertex(drf.position, drf.texCoord, trans_mat, drf.color);
                }
                drawTriangle(model->shader, data);
            }
        }
    }

private:
    // target为要渲染的模型, data[i]为三角形顶点信息: (position, texCoord, color)
    void drawTriangle(const std::shared_ptr<IShader>& shader, const std::array<VertexData, 3>& data) {
        // 检查是否所有点都在[-1,1]外
        bool all_out = true;
        for (auto& one : data) {
            auto tmp = screen_mat_inv * one.position;
            auto min = tmp.v_min(), max = tmp.v_max();
            all_out = all_out && (min < -1 || max > 1);
        }
        //if (all_out) return; // Todo 过滤

        // 缓存颜色
        std::array<Color, 3> colors{data[0].color, data[1].color, data[2].color};
        // 缓存颜色分量
        Vec3 red{colors[0].r, colors[1].r, colors[2].r};
        Vec3 green{colors[0].g, colors[1].g, colors[2].g};
        Vec3 blue{colors[0].b, colors[1].b, colors[2].b};
        // 缓存abc
        Vec3 a = data[0].position, b = data[1].position, c = data[2].position;
        Vec3 rawA = trans_mat_inv * a, rawB = trans_mat_inv * b, rawC = trans_mat_inv * c;
        // 获取abc平面的法向量
        Vec3 norm = (b - a).cross(c - a).normalize();
        // 缓存xy分量
        Vec2 a2 = a.as<2>(), b2 = b.as<2>(), c2 = c.as<2>();
        // 缓存深度信息,参与插值
        Vec3 z3 = make_vec(a.z(), b.z(), c.z());
        // 缓存纹理坐标
        Vec2 texes[] = {data[0].texCoord, data[1].texCoord, data[2].texCoord};
        Vec3 u3      = make_vec(texes[0].x(), texes[1].x(), texes[2].x());
        Vec3 v3      = make_vec(texes[0].y(), texes[1].y(), texes[2].y());
        // 获取i方向边界
        auto i_min = std::max((int) make_vec(a.x(), b.x(), c.x()).v_min(), 0);
        auto i_max = std::min((int) make_vec(a.x(), b.x(), c.x()).v_max(), vh - 1);

#pragma omp parallel for
        // Todo 阴影
        for (int i = i_min; i <= i_max; ++i) {
            // 获取紧致的左右边界
            auto [fl, fr] = getTriangleBound(a2, b2, c2, float(i));
            auto l = std::max(0, (int) fl), r = std::min(vw - 1, (int) fr);
            // Todo 反锯齿
            while (l <= r && !inTriangle(make_vec(i, l), a2, b2, c2)) ++l;
            while (l <= r && !inTriangle(make_vec(i, r), a2, b2, c2)) --r;
            // 填充[l,r]区间
            for (int j = l; j <= r; ++j) {
                // 遍历屏幕空间中的点
                Vec3 rawPoint = make_vec(i, j, 0);
                // 获取z轴信息
                rawPoint = intersect(a, norm, make_vec(0, 0, 1), rawPoint);

                // Todo 修复逆变换后缺面的bug
                // 变换到局部空间
                //rawPoint = trans_mat_inv * rawPoint;
                // 进行逆变换再求重心坐标
                //Vec3 gPos = getGravityPos(rawA, rawB, rawC, rawPoint);
                Vec3 gPos = getGravityPos(a2, b2, c2, rawPoint.as<2>());
                // 映射回屏幕空间
                //rawPoint = trans_mat * rawPoint;

                // 着色器的输入变量,根据重心坐标进行插值
                number dep = gPos * z3;                      // 深度
                Vec2   tex = make_vec(gPos * u3, gPos * v3); // 纹理坐标
                // 着色器的输出变量
                Color color{};         // 像素颜色
                bool  discard = false; // 是否弃用

                // 将uv坐标约束到[0,1]范围内
                for (int t = 0; t < 2; ++t) tex[t] = clamp<number>(0, tex[t], 1);
                // 执行片元着色器
                shader->fragment(rawPoint, tex, color, discard);
                if (discard) {
                    Vec3 vecColor = make_vec(gPos * red, gPos * green, gPos * blue);
                    color         = {vecColor.x(), vecColor.y(), vecColor.z()};
                }
                // 设置像素(并执行深度检测)
                setPixel(i, j, color, dep);
            }
        }
    }

    void setPixel(int i, int j, const Color& fill, number z) {
#ifndef NDEBUG
        if (i < 0 || i >= vh || j < 0 || j >= vw)
            throw std::out_of_range("RsRender::setPixel");
#endif
        auto& ref = depth[i * vw + j];
        if (ref > z) {
            canvas->setPixel(i, j, fill), ref = z;
        }
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
    static Vec3 getGravityPos(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& p) {
        decltype(auto) area = [](const Vec3& v1, const Vec3& v2, const Vec3& v3) {
            return (v1 - v3).cross(v2 - v3).length() / 2;
        };
        number s1 = area(b, c, p), s2 = area(a, b, p), s3 = area(a, b, p);
        number s = s1 + s2 + s3;
        return {s1 / s, s2 / s, s3 / s};
    }

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

    // 获取(r-p)n=0平面与直线r=tv+s
    // p为平面上一点,n为平面法线,v为直线方向向量,s为直线上一点
    static Vec3 intersect(
        const Vec3& facePoint, const Vec3& faceNorm,
        const Vec3& lineDir, const Vec3& linePoint) {
        return (((facePoint - linePoint) * faceNorm)
                / (lineDir * faceNorm))
                   * lineDir
               + linePoint;
    }
};

} // namespace mne

#endif //MINI_ENGINE_RS_RENDER_HPP
