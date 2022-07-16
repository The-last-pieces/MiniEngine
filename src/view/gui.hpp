//
// Created by MnZn on 2022/7/13.
//

#ifndef MINI_ENGINE_GUI_HPP
#define MINI_ENGINE_GUI_HPP

#include "image.hpp"
#include "../math/mat.hpp"
#include "../engine/render.hpp"
#include "GLFW/glfw3.h"
#include <string>
#include <ctime>
#include <queue>
#include <windows.h>

#undef min
#undef max

namespace mne {

// 自动控制绘图模式作用域
struct ModeGuard {
    // 控制类型:   GL_TRIANGLES,GL_LINES,GL_POINTS
    // 分别表示:   三角形,       线段,    像素
    explicit ModeGuard(GLenum mode) { glBegin(mode); }

    ~ModeGuard() { glEnd(); }
};

// 平滑统计量
template<size_t N>
requires(N > 0) class Average {
private:
    std::queue<double> q;

    double sum{};

public:
    operator double() const { return sum / q.size(); }

    Average& operator+=(double val) { return expand(val), *this; }

private:
    void expand(double cur) {
        sum += cur, q.push(cur);
        if (q.size() > N) sum -= q.front(), q.pop();
    }
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
        // 帧率相关
        clock_t         stop, start;
        Average<60 * 3> ave_fps;
        // glfw 事件循环
        while (!glfwWindowShouldClose(window)) {
            start = clock();
            stop  = start + CLOCKS_PER_SEC / fps;

            clearWith(bg_color);     // 清除颜色缓存
            update();                // 更新ui
            glfwSwapBuffers(window); // 实现双缓冲

            if (clock() < stop) Sleep(stop - clock());
            stop = clock();

            ave_fps += stop - start;
            double fact_fps = CLOCKS_PER_SEC / ave_fps;
            glfwSetWindowTitle(window, (title.data() + (" FPS: " + std::to_string(fact_fps))).data());

            glfwPollEvents(); // 处理IO事件(键盘,鼠标...)
        }
        glfwDestroyWindow(window);
        window = nullptr;
    }

protected:
    const int fps = 60;

    BMPImage                image;
    std::shared_ptr<Render> render = std::make_shared<Render>();

    void update() {
        image.init(width, height);
        render->drawAt(image);
        drawBMP(image);
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