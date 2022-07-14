//
// Created by MnZn on 2022/7/13.
//

#ifndef MINI_ENGINE_GUI_HPP
#define MINI_ENGINE_GUI_HPP

#include "GLFW/glfw3.h"
#include <string>
#include <ctime>
#include "windows.h"
#include "image.hpp"
#include "../math/mat.hpp"

namespace mne {

// 自动控制绘图模式作用域
struct ModeGuard {
    // 控制类型:   GL_TRIANGLES,GL_LINES,GL_POINTS
    // 分别表示:   三角形,       线段,    像素
    explicit ModeGuard(GLenum mode) { glBegin(mode); }

    ~ModeGuard() { glEnd(); }
};

class MainWindow {
protected:
    GLFWwindow* window = nullptr;

private:
    // 标题
    std::string_view title;

    // 宽高
    int width, height;
    // 背景颜色
    Color bg_color = {0, 0, 0};

public:
    MainWindow(std::string_view title, int width, int height):
        title(title), width(width), height(height) {
        glfwInit(); // 初始化
    }

    ~MainWindow() {
        glfwTerminate(); // 终止并释放GLFW资源
    }

    // 阻塞直到窗口关闭
    void show() {
        // glfw 创建窗口
        window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
        if (window == nullptr) return;
        // glfw 创建运行上下文
        glfwMakeContextCurrent(window);
        // glfw 事件循环
        clock_t next = clock(); // 控制帧率
        while (!glfwWindowShouldClose(window)) {
            clearWith(bg_color); // 清除颜色缓存
            update();            // 更新ui

            if (clock() > next) Sleep(clock() - next);
            glfwSwapBuffers(window); // 实现双缓冲
            next = clock() + 1000 / fps;

            glfwPollEvents(); // 处理IO事件(键盘,鼠标...)
        }
        glfwDestroyWindow(window);
        window = nullptr;
    }

protected:
    const float size  = 0.25;
    const int   fps   = 60;
    const float sqrt3 = sqrtf(3);

    Mat33 rot = Factory::rotateZ(3.14159 / 60 / 60);
    Vec3  r   = Vec3{0, sqrt3 * 2 / 3, 0} * size;
    Vec3  g   = Vec3{-1, -sqrt3 / 3, 0} * size;
    Vec3  b   = Vec3{1, -sqrt3 / 3, 0} * size;

    void update() {
        // 绘制绕原点旋转的OpenGL经典图案
        r = rot * r;
        g = rot * g;
        b = rot * b;
        
        ModeGuard guard(GL_TRIANGLES);
        glColor3f(1.0, 0.0, 0.0); // Red
        glVertex3fv(r.data);

        glColor3f(0.0, 1.0, 0.0); // Green
        glVertex3fv(g.data);

        glColor3f(0.0, 0.0, 1.0); // Blue
        glVertex3fv(b.data);
    }

protected:
    // 封装常用的OpenGL操作
    static void clearWith(const Color& color) {
        // 设置背景色
        glClearColor(color.r, color.g, color.b, 0.0);
        // 以设置的背景色填充颜色缓冲区
        glClear(GL_COLOR_BUFFER_BIT);
    }

    // 绘制BMP图
    static void drawBMP(const BMPImage& image) {
        ModeGuard guard(GL_POINTS);
        auto [w, h] = image.size();
        auto fw = number(w), fh = number(h);
        for (int i = 0; i < w; i++) {
            auto y = number(i) / fh * 2 - 1;
            for (int j = 0; j < h; j++) {
                auto [r, g, b] = image.getPixel(i, j);
                auto x         = number(j) / fw * 2 - 1;
                glColor3f(r, g, b);
                glVertex2f(x, y);
            }
        }
    }
};
} // namespace mne

#endif