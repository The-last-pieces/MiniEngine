//
// Created by MnZn on 2022/7/13.
//

#ifndef MINI_ENGINE_MODEL_HPP
#define MINI_ENGINE_MODEL_HPP

#include "data/transform.hpp"
#include "interface/shader.hpp"
#include "interface/texture.hpp"
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

namespace mne {

// 三角形的节点信息
struct TriangleNode {
    int pos; // 位置信息,在vertices中的下标
    int tex; // 纹理信息,在textures中的下标
    // Todo 其他信息
};

class Model {
public:
    std::vector<Vec3> vertices{}; // 顶点空间集合
    std::vector<Vec2> textures{}; // 纹理坐标集合

    std::vector<std::array<TriangleNode, 3>> triangles{}; // 三角形集合(储存在vertices中的下标)

    Transform transform; // 模型自身的变换

    std::shared_ptr<ITexture> colorTexture; // 颜色纹理信息

    std::shared_ptr<IShader> shader; // 模型的着色器

public:
    Model() = default;

    // 根据模型路径载入信息
    Model(const std::string& model_path) { loadFromDisk(model_path); }

public:
    void loadFromDisk(const std::string& model_path) {
        std::ifstream in(model_path.data());
        if (!in) return;
        vertices.clear(), triangles.clear(), transform = {};
        std::string line, type; // 单行内容,内容类型
        Vec3        v3;
        char        ignore;
        while (std::getline(in, line)) {
            std::istringstream iss(line);
            iss >> type;
            if (type == "v") {
                // 几何顶点列表,格式为"x y z( w)?" , w默认为1.0
                iss >> v3, vertices.push_back(v3);
            } else if (type == "vt") { // Todo
                // 纹理坐标列表,格式为"u v( w)?" , u,v,w均属于[0,1] , w默认为0.0
                iss >> v3.x() >> v3.y(), textures.push_back(v3.as<2>());
            } else if (type == "vn") { // Todo
                // 顶点法线列表,格式为"x y z" , 法线不规定是单位向量.
            } else if (type == "vp") { // Todo
                // 参数空间顶点,格式为"u( v( w)?)?"
            } else if (type == "f") {
                // 多边形面元素 , 暂时只支持三角形面 , 索引从1开始
                // f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 ...
                int v, t, n;

                std::array<TriangleNode, 3> triangle{};
                for (int i = 0; i < 3; ++i) {
                    iss >> v >> ignore >> t >> ignore >> n;
                    triangle[i] = {v - 1, t - 1};
                }
                triangles.push_back(triangle);
            }
        }
        in.close();
        printf("vertex : %d , face : %d \n", vertex_count(), face_count());
    }

    //    void saveToDisk(std::string_view save_path) {
    //        std::ofstream out(save_path.data());
    //        if (!out) return;
    //        // 顶点坐标
    //        for (auto& vertex : vertices) out << "v " << vertex << '\n';
    //        // 表面坐标
    //        for (auto& abc : triangles) {
    //            out << "f ";
    //            for (auto v : abc) out << v + 1 << "/0/0 ";
    //            out << '\n';
    //        }
    //        out.close();
    //    }

public:
    int vertex_count() const { return (int) vertices.size(); }
    int face_count() const { return (int) triangles.size(); }
};

} // namespace mne

#endif //MINI_ENGINE_MODEL_HPP
